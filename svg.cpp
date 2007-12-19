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
        Private(const QString& imagePath)
            : renderer(0),
              contentType(Svg::SingleImage)
        {
            if (QDir::isAbsolutePath(imagePath)) {
                path = imagePath;
                themed = false;

                if (!QFile::exists(path)) {
                    kDebug() << "Plasma::Svg: file '" << path << "' does not exist!";
                }
            } else {
                themePath = imagePath;
                themed = true;
            }
        }

        ~Private()
        {
            eraseRenderer();
        }

        void removeFromCache()
        {
            if ( id.isEmpty() ) {
                return;
            }

            QPixmapCache::remove( id );
            id.clear();
        }

        void findInCache(QPixmap& p, const QString& elementId)
        {
            createRenderer();
            id = QString::fromLatin1("%3_%2_%1")
                                    .arg(size.width())
                                    .arg(size.height())
                                    .arg(path);
            if (!elementId.isEmpty()) {
                id.append(elementId);
            }
            //kDebug() << "id is " << id;

            if (QPixmapCache::find(id, p)) {
                //kDebug() << "found cached version of " << id;
                return;
            } else {
                //kDebug() << "didn't find cached version of " << id << ", so re-rendering";
            }

            // we have to re-render this puppy
            QSize s;
            if (elementId.isEmpty() || contentType == Svg::ImageSet) {
                s = size.toSize();
            } else {
                s = elementSize(elementId);
            }
            //kDebug() << "size for " << elementId << " is " << s;

            p = QPixmap(s);
            p.fill(Qt::transparent);
            QPainter renderPainter(&p);

            if (elementId.isEmpty()) {
                renderer->render(&renderPainter);
            } else {
                renderer->render(&renderPainter, elementId);
            }
            renderPainter.end();
            bool inserted = QPixmapCache::insert( id, p );
            if (!inserted)
                kDebug() << "pixmap cache is too small for inserting" << id << "of size" << s;
        }

        void createRenderer()
        {
            if (renderer) {
                return;
            }

            if (themed && path.isNull()) {
                path = Plasma::Theme::self()->image(themePath);
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

        QRect elementRect(const QString& elementId)
        {
            createRenderer();
            QRectF elementRect = renderer->boundsOnElement(elementId);
            QSizeF naturalSize = renderer->defaultSize();
            qreal dx = size.width() / naturalSize.width();
            qreal dy = size.height() / naturalSize.height();

            return QRect(elementRect.x() * dx, elementRect.y() * dy,
                         elementRect.width() * dx, elementRect.height() * dy);
        }
    
        QMatrix matrixForElement(const QString& elementId)
        {
            createRenderer();
            return renderer->matrixForElement(elementId);
        }

        static QHash<QString, SharedSvgRenderer::Ptr> renderers;
        SharedSvgRenderer::Ptr renderer;
        QString themePath;
        QString path;
        QString id;
        QSizeF size;
        bool themed;
        Svg::ContentType contentType;
};

QHash<QString, SharedSvgRenderer::Ptr> Svg::Private:: renderers;

Svg::Svg(const QString& imagePath, QObject* parent)
    : QObject(parent),
      d(new Private(imagePath))
{
    if (d->themed) {
        connect(Plasma::Theme::self(), SIGNAL(changed()), this, SLOT(themeChanged()));
    }
}

Svg::~Svg()
{
    delete d;
}

void Svg::paint(QPainter* painter, const QPointF& point, const QString& elementID)
{
    QPixmap pix;
    d->findInCache(pix, elementID);
    //kDebug() << "pix size is " << pix.size();
    painter->drawPixmap(QRectF(point, pix.size()), pix, QRectF(QPointF(0,0), pix.size()));
}

void Svg::paint(QPainter* painter, int x, int y, const QString& elementID)
{
    paint(painter, QPointF(x, y), elementID);
}

void Svg::paint(QPainter* painter, const QRectF& rect, const QString& elementID)
{
    QPixmap pix;
    d->findInCache(pix, elementID);
    //kDebug() << "pix size is " << pix.size();
    painter->drawPixmap(rect, pix, QRectF(QPointF(0,0), pix.size()));
}

void Svg::resize( int width, int height )
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

QRect Svg::elementRect(const QString& elementId) const
{
    return d->elementRect(elementId);
}

bool Svg::elementExists(const QString& elementId) const
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

QMatrix Svg::matrixForElement(const QString& elementId) const
{
    d->createRenderer();
    return d->renderer->matrixForElement(elementId);
}

bool Svg::isValid() const
{
    d->createRenderer();
    return d->renderer->isValid();
}

QSize Svg::size() const
{
    return d->size.toSize();
}

void Svg::setContentType(ContentType contentType)
{
    d->contentType = contentType;
}

Svg::ContentType Svg::contentType()
{
    return d->contentType;
}

void Svg::setFile(const QString &svgFilePath)
{
   d->themePath = svgFilePath;
   d->eraseRenderer();
}

QString Svg::file() const
{
   return d->themePath;
}

void Svg::themeChanged()
{
    d->removeFromCache();
    d->path.clear();
    //delete d->renderer; we're a KSharedPtr
    d->renderer = 0;
    emit repaintNeeded();
}

} // Plasma namespace

#include "svg.moc"

