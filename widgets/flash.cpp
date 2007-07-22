/*
 *   Copyright (C) 2007 by André Duffeck <andre@duffeck.de>
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
 *   51 Franklin Stre
 *   et, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "flash.h"


#include <QtCore/QString>
#include <QtCore/QTimeLine>
#include <QtCore/QTimer>
#include <QtGui/QPainter>
#include <QtGui/QPixmap>
#include <QtGui/QColor>

#include <KDebug>

#include <plasma/phase.h>

using namespace Plasma;

class Flash::Private
{
    public:
        enum FlashType { Text, Pixmap };

        Private() { }
        ~Private() { }
        int height;
        int width;

        QString text;
        QColor color;
        QFont font;
        QPixmap pixmap;
        int duration;
        int defaultDuration;
        FlashType type;

        Plasma::Phase::AnimId animId;
        QPixmap renderedPixmap;
};


Flash::Flash(QGraphicsItem *parent)
    : QObject(),
      QGraphicsItem(parent),
      d(new Private)
{
    d->duration = 2000;
    d->type = Private::Text;
    d->color = Qt::black;
    d->height = 40;
    d->width = 100 ;
    d->animId = 0;
}

Flash::~Flash()
{
    delete d;
}

QRectF Flash::boundingRect() const
{
    return QRectF(0,0,d->width,d->height);
}

int Flash::height() const
{
    return d->height;
}

int Flash::width() const
{
    return d->width;
}

void Flash::setHeight(int h)
{
    prepareGeometryChange ();
    d->height = h;
    update();
}

void Flash::setWidth(int w)
{
    prepareGeometryChange ();
    d->width = w;
    update();
}

void Flash::setDuration( int duration )
{
    d->defaultDuration = duration;
}

QSize Flash::size() const
{
    return QSize(d->width,d->height);
}

void Flash::setSize(const QSize &s)
{
    prepareGeometryChange ();
    d->width = s.width();
    d->height = s.height();
    update();
}

void Flash::setColor( const QColor &color )
{
    d->color = color;
}

void Flash::setFont( const QFont &font )
{
    d->font = font;
}

void Flash::flash( const QString &text, int duration)
{
    d->type = Private::Text;
    d->duration = duration > 0 ? duration : d->defaultDuration;
    d->text = text;
    QTimer::singleShot( 0, this, SLOT(fadeIn()) );
}

void Flash::flash( const QPixmap &pixmap, int duration )
{
    d->type = Private::Pixmap;
    d->duration = duration > 0 ? duration : d->defaultDuration;
    d->pixmap = pixmap;
    QTimer::singleShot( 0, this, SLOT(fadeIn()) );
}

void Flash::fadeIn()
{
    d->renderedPixmap = renderPixmap();
    d->animId = Plasma::Phase::self()->animateElement(this, Plasma::Phase::ElementAppear);
    Plasma::Phase::self()->setAnimationPixmap( d->animId, d->renderedPixmap );
    QTimer::singleShot( d->duration, this, SLOT(fadeOut()) );
}

void Flash::fadeOut()
{
    d->animId = Plasma::Phase::self()->animateElement(this, Plasma::Phase::ElementDisappear);
    Plasma::Phase::self()->setAnimationPixmap( d->animId, d->renderedPixmap );
}

QPixmap Flash::renderPixmap()
{
    QPixmap pm( width(), height() );
    pm.fill(Qt::transparent);

    QPainter painter( &pm );
    if( d->type == Private::Text ) {
        painter.setPen( d->color );
        painter.setFont( d->font );
        painter.drawText( QRect( 0, 0, pm.width(), pm.height() ), d->text, Qt::AlignHCenter | Qt::AlignVCenter);
    } else if( d->type == Private::Pixmap ) {
        painter.drawPixmap( QPoint( 0, 0 ), d->pixmap );
    }
    return pm;
}

void Flash::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)

    if( d->animId ) {
        painter->drawPixmap(0, 0, Plasma::Phase::self()->animationResult(d->animId));
    }
}

#include "flash.moc"
