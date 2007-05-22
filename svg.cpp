/*
 *   Copyright (C) 2006 Aaron Seigo <aseigo@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License version 2 as
 *   published by the Free Software Foundation
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

#include <QMatrix>
#include <QPainter>
#include <QPixmapCache>

#include <KDebug>
#include <KSvgRenderer>
#include "theme.h"

namespace Plasma
{

class Svg::Private
{
    public:
        Private( const QString& image )
            : renderer( 0 ),
              themePath( image )
        {
        }

        ~Private()
        {
            delete renderer;
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
            if ( path.isNull() ) {
                path = Plasma::Theme::self()->image( themePath );

                if ( path.isNull() ) {
                    // bad theme path
                    return;
                }
            }

            id = QString::fromLatin1("%3_%2_%1")
                                    .arg( size.width() )
                                    .arg( size.height() )
                                    .arg( themePath );
            if (!elementId.isEmpty()) {
                id.append(elementId);
            }

            if ( QPixmapCache::find( id, p ) ) {
                //kDebug() << "found cached version of " << id << endl;
                return;
            } else {
                //kDebug() << "didn't find cached version of " << id << ", so re-rendering" << endl;
            }

            // we have to re-render this puppy
            if (!renderer) {
                //TODO: connect the renderer's repaintNeeded to the Plasma::Svg signal
                //      take into consideration for cache, e.g. don't cache if svg is animated
                renderer = new KSvgRenderer(path);
            }

            QSize s;
            if ( elementId.isEmpty() ) {
                s = size.toSize();
            } else {
                s = renderer->boundsOnElement(elementId).size().toSize();
            }
            //kDebug() << "size for " << elementId << " is " << s << endl;

            p = QPixmap(s);
            p.fill(Qt::transparent);
            QPainter renderPainter( &p );

            if ( elementId.isEmpty() ) {
                renderer->render( &renderPainter );
            } else {
                renderer->render( &renderPainter, elementId );
            }
            renderPainter.end();
            QPixmapCache::insert( id, p );
        }

        //TODO: share renderers between Svg objects with identical themePath
        KSvgRenderer* renderer;
        QString themePath;
        QString path;
        QString id;
        QSizeF size;
};

Svg::Svg( const QString& imagePath, QObject* parent )
    : QObject( parent ),
      d( new Private( imagePath ) )
{
    connect(Plasma::Theme::self(), SIGNAL(changed()), this, SLOT(themeChanged()));
}

Svg::~Svg()
{

}

void Svg::paint(QPainter* painter, const QPointF& point, const QString& elementID)
{
    QPixmap pix;
    d->findInCache(pix, elementID);
    painter->drawPixmap(point.toPoint(), pix);
}

void Svg::paint(QPainter* painter, int x, int y, const QString& elementID)
{
    paint(painter, QPointF( x, y ), elementID);
}

void Svg::paint(QPainter* painter, const QRectF& rect, const QString& elementID)
{
    QPixmap pix;
    d->findInCache(pix, elementID);
    painter->drawPixmap(rect, pix, rect);
}

void Svg::resize( int width, int height )
{
    resize( QSize( width, height ) );
}

void Svg::resize( const QSizeF& size )
{
    d->size = size;
}

void Svg::resize()
{
    if (!d->renderer) {
        d->renderer = new KSvgRenderer(Plasma::Theme::self()->image(d->themePath));
    }
    d->size = d->renderer->defaultSize();
}

QSize Svg::elementSize(const QString& elementId) const
{
    if (!d->renderer) {
        d->renderer = new KSvgRenderer(Plasma::Theme::self()->image(d->themePath));
    }

    QSizeF elementSize = d->renderer->boundsOnElement(elementId).size();
    QSizeF naturalSize = d->renderer->defaultSize();
    qreal dx = d->size.width() / naturalSize.width();
    qreal dy = d->size.height() / naturalSize.height();
    elementSize.scale( elementSize.width() * dx, elementSize.height() * dy, Qt::IgnoreAspectRatio );

    return elementSize.toSize();
}

QSize Svg::size() const
{
    return d->size.toSize();
}

void Svg::themeChanged()
{
    d->removeFromCache();
    d->path.clear();
    delete d->renderer;
    d->renderer = 0;
    emit repaintNeeded();
}

} // Plasma namespace

#include "svg.moc"

