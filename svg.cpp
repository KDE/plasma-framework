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

#include <QMatrix>
#include <QPainter>
#include <QPixmapCache>
#include <QSvgRenderer>

#include <KDebug>

#include "svg.h"
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

        void findInCache( QPainter* painter, QPixmap& p )
        {
            if ( path.isNull() ) {
                path = Plasma::Theme::self()->image( themePath );

                if ( path.isNull() ) {
                    // bad theme path
                    return;
                }
            }

            QMatrix matrix = painter->worldMatrix();

            //TODO: if the id changes, should we remove it or just let QPixmapCache do that for us?
            id = QString::fromLatin1( "%7_%1_%2_%3_%4_%5_%6" )
                                      .arg( size.width() )
                                      .arg( size.height() )
                                      .arg( matrix.m11() )
                                      .arg( matrix.m12() )
                                      .arg( matrix.m21() )
                                      .arg( matrix.m22() )
                                      .arg( themePath );

            if ( QPixmapCache::find( id, p ) ) {
                kDebug() << "found cached version of " << id << endl;
                return;
            } else {
                kDebug() << "didn't find cached version of " << id << ", so re-rendering" << endl;
            }

            // we have to re-render this puppy
            if ( ! renderer ) {
                //TODO: connect the renderer's repaintNeeded to the Plasma::Svg signal
                //      take into consideration for cache, e.g. don't cache if svg is animated
                renderer = new QSvgRenderer( path );
            }

            p.resize( size );
            p.fill(Qt::transparent);
            QPainter renderPainter( &p );
            renderPainter.setWorldMatrix( matrix );
            renderer->render( &renderPainter, p.rect() );
            renderPainter.end();
            QPixmapCache::insert( id, p );
        }

        //TODO: share renderers between Svg objects with identical themePath
        QSvgRenderer* renderer;
        QString themePath;
        QString path;
        QString id;
        QSize size;
};

Svg::Svg( const QString& imagePath, QObject* parent )
    : QObject( parent ),
      d( new Private( imagePath ) )
{
    connect( Plasma::Theme::self(), SIGNAL(changed()), this, SLOT(themeChanged()) );
}

Svg::~Svg()
{

}

void Svg::paint( QPainter* painter, const QPoint& point )
{
    QPixmap pix;
    d->findInCache( painter, pix );

    QMatrix matrix = painter->worldMatrix();
    painter->setWorldMatrix( QMatrix() );
    painter->drawPixmap( point, pix );
    painter->setWorldMatrix( matrix );
}

void Svg::paint( QPainter* painter, int x, int y )
{
    paint( painter, QPoint( x, y ) );
}

void Svg::paint( QPainter* painter, const QRect& rect )
{
    QPixmap pix;
    d->findInCache( painter, pix );

    QMatrix matrix = painter->worldMatrix();
    painter->setWorldMatrix( QMatrix() );
    painter->drawPixmap( rect, pix );
    painter->setWorldMatrix( matrix );
}

void Svg::resize( int width, int height )
{
    resize( QSize( width, height ) );
}

void Svg::resize( const QSize& size )
{
    d->size = size;
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

