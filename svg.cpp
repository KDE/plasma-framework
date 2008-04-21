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

#include <QDir>
#include <QMatrix>
#include <QPainter>
#include <QPixmapCache>
#include <QSharedData>

#include <KDebug>
#include <KSharedPtr>
#include <KSvgRenderer>
#include <KColorScheme>
#include <KIconEffect>
#include <KGlobalSettings>

#include "theme.h"

namespace Plasma
{

class SharedSvgRenderer : public KSvgRenderer, public QSharedData
{
    public:
        typedef KSharedPtr<SharedSvgRenderer> Ptr;

        SharedSvgRenderer(QObject *parent = 0)
            : KSvgRenderer(parent)
        {}

        SharedSvgRenderer(const QString &filename, QObject *parent = 0)
            : KSvgRenderer(filename, parent)
        {}

        SharedSvgRenderer(const QByteArray &contents, QObject *parent = 0)
            : KSvgRenderer(contents, parent)
        {}

        ~SharedSvgRenderer()
        {
            //kDebug() << "leaving this world for a better one.";
        }
};

class Svg::Private
{
    public:
        Private(const QString& imagePath, Svg *svg)
            : q(svg),
              renderer(0),
              multipleImages(false),
              themed(false)
        {
            setImagePath(imagePath, q);
        }

        ~Private()
        {
            eraseRenderer();
        }

        void setImagePath(const QString &imagePath, Svg *q)
        {
            if (themed) {
                disconnect(Plasma::Theme::self(), SIGNAL(changed()), q, SLOT(themeChanged()));
                disconnect(KGlobalSettings::self(), SIGNAL(kdisplayPaletteChanged()), q, SLOT(colorsChanged()));
            }

            themed = !QDir::isAbsolutePath(imagePath);
            path = themePath = QString();

            if (themed) {
                themePath = imagePath;
                connect(Plasma::Theme::self(), SIGNAL(changed()), q, SLOT(themeChanged()));

                // check if svg wants colorscheme applied
                createRenderer();
                applyColors = renderer->elementExists("hint-apply-color-scheme");
                if (applyColors && !Theme::self()->colorScheme()) {
                    connect(KGlobalSettings::self(), SIGNAL(kdisplayPaletteChanged()), q, SLOT(colorsChanged()));
                }

            } else {
                path = imagePath;

                if (!QFile::exists(path)) {
                    kDebug() << "Plasma::Svg: file '" << path << "' does not exist!";
                }
            }
        }

        void removeFromCache() {
            if (ids.isEmpty()) {
                return;
            }

            foreach (const QString &id, ids) {
                QPixmapCache::remove(id);
            }

            ids.clear();
        }

        void findInCache(QPixmap& p, const QString& elementId, const QPainter *itemPainter, const QSizeF &s = QSizeF())
        {
            createRenderer();

            QSize size;
            if (elementId.isEmpty() || multipleImages) {
                size = s.toSize();
            } else {
                size = elementSize(elementId);
            }

            if (!size.isValid()) {
                p = QPixmap();
                return;
            }

            QString id = QString::fromLatin1("%3_%2_%1_").arg(size.width())
                                                         .arg(size.height())
                                                         .arg(path);

            if (!elementId.isEmpty()) {
                id.append(elementId);
            }
            //kDebug() << "id is " << id;

            if (!ids.contains(id)) {
                ids.append(id);
            }

            if (QPixmapCache::find(id, p)) {
                //kDebug() << "found cached version of " << id;
                return;
            } else {
                //kDebug() << "didn't find cached version of " << id << ", so re-rendering";
            }

            //kDebug() << "size for " << elementId << " is " << s;
            // we have to re-render this puppy

            p = QPixmap(size);

            p.fill(Qt::transparent);
            QPainter renderPainter(&p);

            if (elementId.isEmpty()) {
                renderer->render(&renderPainter);
            } else {
                renderer->render(&renderPainter, elementId);
            }

            renderPainter.end();

            // Apply current color scheme if the svg asks for it
            if (applyColors) {
                QImage itmp = p.toImage();
                KIconEffect::colorize(itmp, Theme::self()->backgroundColor(), 1.0);
                p = p.fromImage(itmp);
            }

            if (!QPixmapCache::insert(id, p)) {
                //kDebug() << "pixmap cache is too small for inserting" << id << "of size" << s;
            }
        }

        void createRenderer()
        {
            if (renderer) {
                return;
            }

            if (themed && path.isEmpty()) {
                path = Plasma::Theme::self()->imagePath(themePath);
            }

            QHash<QString, SharedSvgRenderer::Ptr>::const_iterator it = renderers.find(path);

            if (it != renderers.end()) {
                //kDebug() << "gots us an existing one!";
                renderer = it.value();
            } else {
                renderer = new SharedSvgRenderer(path);
                renderers[path] = renderer;
            }

            size = renderer->defaultSize();
        }

        void eraseRenderer()
        {
            if (renderer.count() == 2) {
                // this and the cache reference it; and boy is this not thread safe ;)
                renderers.erase(renderers.find(themePath));
            }

            renderer = 0;
        }

        QSize elementSize(const QString& elementId)
        {
            createRenderer();
            QSizeF elementSize = renderer->boundsOnElement(elementId).size();
            QSizeF naturalSize = renderer->defaultSize();
            qreal dx = size.width() / naturalSize.width();
            qreal dy = size.height() / naturalSize.height();
            elementSize.scale(elementSize.width() * dx, elementSize.height() * dy, Qt::IgnoreAspectRatio);

            return elementSize.toSize();
        }

        QRectF elementRect(const QString& elementId)
        {
            createRenderer();
            QRectF elementRect = renderer->boundsOnElement(elementId);
            QSizeF naturalSize = renderer->defaultSize();
            qreal dx = size.width() / naturalSize.width();
            qreal dy = size.height() / naturalSize.height();

            return QRectF(elementRect.x() * dx, elementRect.y() * dy,
                         elementRect.width() * dx, elementRect.height() * dy);
        }

        QMatrix matrixForElement(const QString& elementId)
        {
            createRenderer();
            return renderer->matrixForElement(elementId);
        }

        void themeChanged()
        {
            if (!themed) {
                return;
            }

            QString newPath = Theme::self()->imagePath(themePath);

            if (path == newPath) {
                return;
            }

            removeFromCache();
            path = newPath;
            //delete d->renderer; we're a KSharedPtr
            eraseRenderer();
            emit q->repaintNeeded();
        }

        void colorsChanged()
        {
            if (!applyColors) {
                return;
            }

            removeFromCache();
            eraseRenderer();
            emit q->repaintNeeded();
        }

        Svg *q;
        static QHash<QString, SharedSvgRenderer::Ptr> renderers;
        SharedSvgRenderer::Ptr renderer;
        QString themePath;
        QString path;
        QList<QString> ids;
        QSizeF size;
        bool multipleImages;
        bool themed;
        bool applyColors;
};

QHash<QString, SharedSvgRenderer::Ptr> Svg::Private::renderers;

Svg::Svg(const QString& imagePath, QObject* parent)
    : QObject(parent),
      d(new Private(imagePath, this))
{
}

Svg::~Svg()
{
    delete d;
}

void Svg::paint(QPainter* painter, const QPointF& point, const QString& elementID)
{
    QPixmap pix;
    d->findInCache(pix, elementID, painter);

    if (pix.isNull()) {
        return;
    }

    painter->drawPixmap(QRectF(point, pix.size()), pix, QRectF(QPointF(0,0), pix.size()));
}

void Svg::paint(QPainter* painter, int x, int y, const QString& elementID)
{
    paint(painter, QPointF(x, y), elementID);
}

void Svg::paint(QPainter* painter, const QRectF& rect, const QString& elementID)
{
    QPixmap pix;
    d->findInCache(pix, elementID, painter, rect.size());
    painter->drawPixmap(rect, pix, QRectF(QPointF(0,0), pix.size()));
}

QSize Svg::size() const
{
    return d->size.toSize();
}

void Svg::resize( qreal width, qreal height )
{
    resize( QSize( width, height ) );
}

void Svg::resize( const QSizeF& size )
{
    d->createRenderer();
    d->size = size;
}

void Svg::resize()
{
    d->createRenderer();
    d->size = d->renderer->defaultSize();
}

QSize Svg::elementSize(const QString& elementId) const
{
    return d->elementSize(elementId);
}

QRectF Svg::elementRect(const QString& elementId) const
{
    return d->elementRect(elementId);
}

bool Svg::hasElement(const QString& elementId) const
{
    d->createRenderer();
    return d->renderer->elementExists(elementId);
}

QString Svg::elementAtPoint(const QPoint &point) const
{
    d->createRenderer();
    QSizeF naturalSize = d->renderer->defaultSize();
    qreal dx = d->size.width() / naturalSize.width();
    qreal dy = d->size.height() / naturalSize.height();
    //kDebug() << point << "is really" << QPoint(point.x() *dx, naturalSize.height() - point.y() * dy);

    return QString(); // d->renderer->elementAtPoint(QPoint(point.x() *dx, naturalSize.height() - point.y() * dy));
}

bool Svg::isValid() const
{
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
   d->setImagePath(svgFilePath, this);
   d->eraseRenderer();
}

QString Svg::imagePath() const
{
   return d->themed ? d->themePath : d->path;
}

} // Plasma namespace

#include "svg.moc"

