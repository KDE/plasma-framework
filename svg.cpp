/*
 *   Copyright 2006-2007 Aaron Seigo <aseigo@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "svg.h"

#include <cmath>

#include <QDir>
#include <QMatrix>
#include <QPainter>
#include <QSharedData>
#include <QSvgRenderer>

#include <kcolorscheme.h>
#include <kconfiggroup.h>
#include <kdebug.h>
#include <kiconeffect.h>
#include <kglobalsettings.h>
#include <ksharedptr.h>

#include "applet.h"
#include "package.h"
#include "theme.h"

namespace Plasma
{

class SharedSvgRenderer : public QSvgRenderer, public QSharedData
{
    public:
        typedef KSharedPtr<SharedSvgRenderer> Ptr;

        SharedSvgRenderer(QObject *parent = 0)
            : QSvgRenderer(parent)
        {}

        SharedSvgRenderer(const QString &filename, QObject *parent = 0)
            : QSvgRenderer(filename, parent)
        {}

        SharedSvgRenderer(const QByteArray &contents, QObject *parent = 0)
            : QSvgRenderer(contents, parent)
        {}

        ~SharedSvgRenderer()
        {
            //kDebug() << "leaving this world for a better one.";
        }
};

class SvgPrivate
{
    public:
        SvgPrivate(Svg *svg)
            : q(svg),
              renderer(0),
              lastModified(0),
              multipleImages(false),
              themed(false),
              applyColors(false),
              cacheRendering(true),
              themeFailed(false)
        {
        }

        ~SvgPrivate()
        {
            eraseRenderer();
        }

        //This function is meant for the rects cache
        QString cacheId(const QString &elementId)
        {
            if (size.isValid() && size != naturalSize) {
                return QString("%3_%2_%1").arg(int(size.height()))
                                        .arg(int(size.width()))
                                        .arg(elementId);
            } else {
                return QString("%2_%1").arg("Natural")
                                        .arg(elementId);
            }
        }

        //This function is meant for the pixmap cache
        QString cachePath(const QString &path, const QSize &size)
        {
             return QString("%3_%2_%1_").arg(int(size.height()))
                                        .arg(int(size.width()))
                                        .arg(path);
        }

        bool setImagePath(const QString &imagePath)
        {
            bool isThemed = !QDir::isAbsolutePath(imagePath);

            // lets check to see if we're already set to this file
            if (isThemed == themed &&
                ((themed && themePath == imagePath) ||
                 (!themed && path == imagePath))) {
                return false;
            }

            // if we don't have any path right now and are going to set one,
            // then lets not schedule a repaint because we are just initializing!
            bool updateNeeded = true; //!path.isEmpty() || !themePath.isEmpty();

            if (themed) {
                QObject::disconnect(actualTheme(), SIGNAL(themeChanged()),
                                    q, SLOT(themeChanged()));
            }

            themed = isThemed;
            path.clear();
            themePath.clear();
            localRectCache.clear();

            if (themed) {
                themePath = imagePath;
                themeFailed = false;
                QObject::connect(actualTheme(), SIGNAL(themeChanged()), q, SLOT(themeChanged()));
            } else if (QFile::exists(imagePath)) {
                path = imagePath;
            } else {
                kDebug() << "file '" << path << "' does not exist!";
            }

            // check if svg wants colorscheme applied
            QObject::disconnect(KGlobalSettings::self(), SIGNAL(kdisplayPaletteChanged()),
                                q, SLOT(colorsChanged()));

            checkApplyColorHint();
            if (applyColors && !actualTheme()->colorScheme()) {
                QObject::connect(KGlobalSettings::self(), SIGNAL(kdisplayPaletteChanged()),
                                 q, SLOT(colorsChanged()));
            }

            // also images with absolute path needs to have a natural size initialized,
            // even if looks a bit weird using Theme to store non-themed stuff
            if (themed || QFile::exists(imagePath)) {
                QRectF rect;
                bool found = actualTheme()->findInRectsCache(path, "_Natural", rect);

                if (!found) {
                    createRenderer();
                    naturalSize = renderer->defaultSize();
                    //kDebug() << "natural size for" << path << "from renderer is" << naturalSize;
                    actualTheme()->insertIntoRectsCache(path, "_Natural", QRectF(QPointF(0,0), naturalSize));
                } else {
                    naturalSize = rect.size();
                    //kDebug() << "natural size for" << path << "from cache is" << naturalSize;
                }
            }

            if (!themed) {
                QFile f(imagePath);
                QFileInfo info(f);
                lastModified = info.lastModified().toTime_t();
            }

            return updateNeeded;
        }

        Theme *actualTheme()
        {
            if (!theme) {
                theme = Plasma::Theme::defaultTheme();
            }

            return theme.data();
        }

        QPixmap findInCache(const QString &elementId, const QSizeF &s = QSizeF())
        {
            QSize size;
            QString actualElementId(QString("%1-%2-%3").arg(qRound(s.width())).arg( qRound(s.height())).arg(elementId));

            if (elementId.isEmpty() || !q->hasElement(actualElementId)) {
                actualElementId = elementId;
            }

            if (elementId.isEmpty() || (multipleImages && s.isValid())) {
                size = s.toSize();
            } else {
                size = elementRect(actualElementId).size().toSize();
            }

            if (size.isEmpty()) {
                return QPixmap();
            }

            QString id = cachePath(path, size);

            if (!actualElementId.isEmpty()) {
                id.append(actualElementId);
            }

            //kDebug() << "id is " << id;

            QPixmap p;
            if (cacheRendering) {
                if (actualTheme()->findInCache(id, p, lastModified)) {
                    //kDebug() << "found cached version of " << id << p.size();
                    return p;
                }
            }

            //kDebug() << "didn't find cached version of " << id << ", so re-rendering";

            //kDebug() << "size for " << actualElementId << " is " << s;
            // we have to re-render this puppy

            createRenderer();

            QRectF finalRect = makeUniform(renderer->boundsOnElement(actualElementId), QRect(QPoint(0,0), size));


            //don't alter the pixmap size or it won't connect animre different parts of framesvg
            //but makeUniform should never change the size so much to make necessary to gain or remove a whole pixel
            p = QPixmap(size);

            p.fill(Qt::transparent);
            QPainter renderPainter(&p);

            if (actualElementId.isEmpty()) {
                renderer->render(&renderPainter, finalRect);
            } else {
                renderer->render(&renderPainter, actualElementId, finalRect);
            }

            renderPainter.end();

            // Apply current color scheme if the svg asks for it
            if (applyColors) {
                QImage itmp = p.toImage();
                KIconEffect::colorize(itmp, actualTheme()->color(Theme::BackgroundColor), 1.0);
                p = p.fromImage(itmp);
            }

            if (cacheRendering) {
                actualTheme()->insertIntoCache(id, p, QString::number((qint64)q, 16)+actualElementId);
            }

            return p;
        }

        void createRenderer()
        {
            if (renderer) {
                return;
            }

            //kDebug() << kBacktrace();
            if (themed && path.isEmpty() && !themeFailed) {
                Applet *applet = qobject_cast<Applet*>(q->parent());
                if (applet && applet->package()) {
                    path = applet->package()->filePath("images", themePath + ".svg");

                    if (path.isEmpty()) {
                        path = applet->package()->filePath("images", themePath + ".svgz");
                    }
                }

                if (path.isEmpty()) {
                    path = actualTheme()->imagePath(themePath);
                    themeFailed = path.isEmpty();
                    if (themeFailed) {
                        kWarning() << "No image path found for" << themePath;
                    }
                }
            }

            //kDebug() << "********************************";
            //kDebug() << "FAIL! **************************";
            //kDebug() << path << "**";

            QHash<QString, SharedSvgRenderer::Ptr>::const_iterator it = s_renderers.constFind(path);

            if (it != s_renderers.constEnd()) {
                //kDebug() << "gots us an existing one!";
                renderer = it.value();
            } else {
                if (path.isEmpty()) {
                    renderer = new SharedSvgRenderer();
                } else {
                    renderer = new SharedSvgRenderer(path);
                }

                s_renderers[path] = renderer;
            }

            if (size == QSizeF()) {
                size = renderer->defaultSize();
            }
        }

        void eraseRenderer()
        {
            if (renderer && renderer.count() == 2) {
                // this and the cache reference it
                s_renderers.erase(s_renderers.find(path));

                if (theme) {
                    theme.data()->releaseRectsCache(path);
                }
            }

            renderer = 0;
            localRectCache.clear();
        }

        QRectF elementRect(const QString &elementId)
        {
            if (themed && path.isEmpty()) {
                if (themeFailed) {
                    return QRectF();
                }

                path = actualTheme()->imagePath(themePath);
                themeFailed = path.isEmpty();

                if (themeFailed) {
                    return QRectF();
                }
            }

            QString id = cacheId(elementId);
            if (localRectCache.contains(id)) {
                return localRectCache.value(id);
            }

            QRectF rect;
            bool found = actualTheme()->findInRectsCache(path, id, rect);

            if (found) {
                localRectCache.insert(id, rect);
                return rect;
            }

            return findAndCacheElementRect(elementId);
        }

        QRectF findAndCacheElementRect(const QString &elementId)
        {
            createRenderer();
            QRectF elementRect = renderer->elementExists(elementId) ?
                                 renderer->boundsOnElement(elementId) : QRectF();
            naturalSize = renderer->defaultSize();
            //kDebug() << "natural size for" << path << "is" << naturalSize;
            qreal dx = size.width() / naturalSize.width();
            qreal dy = size.height() / naturalSize.height();

            elementRect = QRectF(elementRect.x() * dx, elementRect.y() * dy,
                                 elementRect.width() * dx, elementRect.height() * dy);

            actualTheme()->insertIntoRectsCache(path, cacheId(elementId), elementRect);
            return elementRect;
        }

        QMatrix matrixForElement(const QString &elementId)
        {
            createRenderer();
            return renderer->matrixForElement(elementId);
        }

        void checkApplyColorHint()
        {
            applyColors = elementRect("hint-apply-color-scheme").isValid();
        }

        //Folowing two are utility functions to snap rendered elements to the pixel grid
        //to and from are always 0 <= val <= 1
        qreal closestDistance(qreal to, qreal from)
        {
                qreal a = to - from;
                if (qFuzzyCompare(to, from))
                    return 0;
                else if ( to > from ) {
                    qreal b = to - from - 1;
                    return (qAbs(a) > qAbs(b)) ?  b : a;
                } else {
                    qreal b = 1 + to - from;
                    return (qAbs(a) > qAbs(b)) ? b : a;
                }
        }

        QRectF makeUniform(const QRectF &orig, const QRectF &dst)
        {
            if (qFuzzyIsNull(orig.x()) || qFuzzyIsNull(orig.y())) {
                return dst;
            }

            QRectF res(dst);
            qreal div_w = dst.width() / orig.width();
            qreal div_h = dst.height() / orig.height();

            qreal div_x = dst.x() / orig.x();
            qreal div_y = dst.y() / orig.y();

            //horizontal snap
            if (!qFuzzyIsNull(div_x) && !qFuzzyCompare(div_w, div_x)) {
                qreal rem_orig = orig.x() - (floor(orig.x()));
                qreal rem_dst = dst.x() - (floor(dst.x()));
                qreal offset = closestDistance(rem_dst, rem_orig);
                res.translate(offset + offset*div_w, 0);
                res.setWidth(res.width() + offset);
            }
            //vertical snap
            if (!qFuzzyIsNull(div_y) && !qFuzzyCompare(div_h, div_y)) {
                qreal rem_orig = orig.y() - (floor(orig.y()));
                qreal rem_dst = dst.y() - (floor(dst.y()));
                qreal offset = closestDistance(rem_dst, rem_orig);
                res.translate(0, offset + offset*div_h);
                res.setHeight(res.height() + offset);
            }

            //kDebug()<<"Aligning Rects, origin:"<<orig<<"destination:"<<dst<<"result:"<<res;
            return res;
        }

        //Slots
        void themeChanged()
        {
            // check if new theme svg wants colorscheme applied
            bool wasApplyColors = applyColors;
            checkApplyColorHint();
            if (applyColors && actualTheme()->colorScheme()) {
                if (!wasApplyColors) {
                    QObject::connect(KGlobalSettings::self(), SIGNAL(kdisplayPaletteChanged()),
                                     q, SLOT(colorsChanged()));
                }
            } else {
                QObject::disconnect(KGlobalSettings::self(), SIGNAL(kdisplayPaletteChanged()),
                                    q, SLOT(colorsChanged()));
            }

            if (!themed) {
                return;
            }

            QString currentPath = themePath;
            themePath.clear();
            eraseRenderer();
            setImagePath(currentPath);

            //kDebug() << themePath << ">>>>>>>>>>>>>>>>>> theme changed";
            emit q->repaintNeeded();
        }

        void colorsChanged()
        {
            if (!applyColors) {
                return;
            }

            eraseRenderer();
            //kDebug() << "repaint needed from colorsChanged";
            emit q->repaintNeeded();
        }

        static QHash<QString, SharedSvgRenderer::Ptr> s_renderers;

        Svg *q;
        QWeakPointer<Theme> theme;
        QHash<QString, QRectF> localRectCache;
        SharedSvgRenderer::Ptr renderer;
        QString themePath;
        QString path;
        QSizeF size;
        QSizeF naturalSize;
        unsigned int lastModified;
        bool multipleImages : 1;
        bool themed : 1;
        bool applyColors : 1;
        bool cacheRendering : 1;
        bool themeFailed : 1;
};

QHash<QString, SharedSvgRenderer::Ptr> SvgPrivate::s_renderers;

Svg::Svg(QObject *parent)
    : QObject(parent),
      d(new SvgPrivate(this))
{
}

Svg::~Svg()
{
    delete d;
}

QPixmap Svg::pixmap(const QString &elementID)
{
    if (elementID.isNull() || d->multipleImages) {
        return d->findInCache(elementID, size());
    } else {
        return d->findInCache(elementID);
    }
}

void Svg::paint(QPainter *painter, const QPointF &point, const QString &elementID)
{
    QPixmap pix(elementID.isNull() ? d->findInCache(elementID, size()) :
                                     d->findInCache(elementID));

    if (pix.isNull()) {
        return;
    }

    painter->drawPixmap(QRectF(point, pix.size()), pix, QRectF(QPointF(0, 0), pix.size()));
}

void Svg::paint(QPainter *painter, int x, int y, const QString &elementID)
{
    paint(painter, QPointF(x, y), elementID);
}

void Svg::paint(QPainter *painter, const QRectF &rect, const QString &elementID)
{
    QPixmap pix(d->findInCache(elementID, rect.size()));
    painter->drawPixmap(QRectF(rect.topLeft(), pix.size()), pix, QRectF(QPointF(0, 0), pix.size()));
}

void Svg::paint(QPainter *painter, int x, int y, int width, int height, const QString &elementID)
{
    QPixmap pix(d->findInCache(elementID, QSizeF(width, height)));
    painter->drawPixmap(x, y, pix, 0, 0, pix.size().width(), pix.size().height());
}

QSize Svg::size() const
{
    if (d->size.isEmpty()) {
        d->size = d->naturalSize;
    }

    return d->size.toSize();
}

void Svg::resize(qreal width, qreal height)
{
    resize(QSize(width, height));
}

void Svg::resize(const QSizeF &size)
{
    if (qFuzzyCompare(size.width(), d->size.width()) &&
        qFuzzyCompare(size.height(), d->size.height())) {
        return;
    }

    d->size = size;
    d->localRectCache.clear();
}

void Svg::resize()
{
    if (qFuzzyCompare(d->naturalSize.width(), d->size.width()) &&
        qFuzzyCompare(d->naturalSize.height(), d->size.height())) {
        return;
    }

    d->size = d->naturalSize;
    d->localRectCache.clear();
}

QSize Svg::elementSize(const QString &elementId) const
{
    return d->elementRect(elementId).size().toSize();
}

QRectF Svg::elementRect(const QString &elementId) const
{
    return d->elementRect(elementId);
}

bool Svg::hasElement(const QString &elementId) const
{
    if (d->path.isNull() && d->themePath.isNull()) {
        return false;
    }

    return d->elementRect(elementId).isValid();
}

QString Svg::elementAtPoint(const QPoint &point) const
{
    Q_UNUSED(point)

    return QString();
/*
FIXME: implement when Qt can support us!
    d->createRenderer();
    QSizeF naturalSize = d->renderer->defaultSize();
    qreal dx = d->size.width() / naturalSize.width();
    qreal dy = d->size.height() / naturalSize.height();
    //kDebug() << point << "is really"
    //         << QPoint(point.x() *dx, naturalSize.height() - point.y() * dy);

    return QString(); // d->renderer->elementAtPoint(QPoint(point.x() *dx, naturalSize.height() - point.y() * dy));
    */
}

bool Svg::isValid() const
{
    if (d->path.isNull() && d->themePath.isNull()) {
        return false;
    }

    d->createRenderer();
    return d->renderer->isValid();
}

void Svg::setContainsMultipleImages(bool multiple)
{
    d->multipleImages = multiple;
}

bool Svg::containsMultipleImages() const
{
    return d->multipleImages;
}

void Svg::setImagePath(const QString &svgFilePath)
{
    d->eraseRenderer();
    d->setImagePath(svgFilePath);
    //kDebug() << "repaintNeeded";
    emit repaintNeeded();
}

QString Svg::imagePath() const
{
   return d->themed ? d->themePath : d->path;
}

void Svg::setUsingRenderingCache(bool useCache)
{
    d->cacheRendering = useCache;
}

bool Svg::isUsingRenderingCache() const
{
    return d->cacheRendering;
}

void Svg::setTheme(Plasma::Theme *theme)
{
    if (d->theme) {
        disconnect(d->theme.data(), 0, this, 0);
    }

    d->theme = theme;
    if (!imagePath().isEmpty()) {
        QString path = imagePath();
        d->themePath.clear();
        setImagePath(path);
    }
}

Theme *Svg::theme() const
{
    return d->theme ? d->theme.data() : Theme::defaultTheme();
}

} // Plasma namespace

#include "svg.moc"

