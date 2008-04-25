/*
 *   Copyright 2007 by André Duffeck <duffeck@kde.org>
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

#include <plasma/animator.h>

using namespace Plasma;

class Flash::Private
{
    public:
        enum FlashType { Text, Pixmap };
        enum State { Visible, Invisible };

        Private() { }
        ~Private() { }

        QString text;
        QColor color;
        QFont font;
        QPixmap pixmap;
        int duration;
        int defaultDuration;
        FlashType type;

        Plasma::Phase::AnimId animId;
        QPixmap renderedPixmap;

        QTextOption textOption;
        Qt::Alignment alignment;

        State state;
};


Flash::Flash(QGraphicsItem *parent)
    : Plasma::Widget(parent),
      d(new Private)
{
    d->defaultDuration = 3000;
    d->type = Private::Text;
    d->color = Qt::black;
    d->animId = 0;
    d->state = Private::Invisible;
    resize(QSizeF(40, 100));

    setCacheMode(NoCache);
}

Flash::~Flash()
{
    delete d;
}

QRectF Flash::boundingRect() const
{
    return QRectF(0, 0, size().width(), size().height());
}

void Flash::setDuration( int duration )
{
    d->defaultDuration = duration;
}

void Flash::setColor( const QColor &color )
{
    d->color = color;
}

void Flash::setFont( const QFont &font )
{
    d->font = font;
}

void Flash::flash( const QString &text, int duration, const QTextOption &option)
{
    kDebug() << duration;
    d->type = Private::Text;
    d->duration = (duration == 0) ? d->defaultDuration : duration;
    d->text = text;
    d->textOption = option;
    QTimer::singleShot( 0, this, SLOT(fadeIn()) );
}

void Flash::flash( const QPixmap &pixmap, int duration, Qt::Alignment align )
{
    d->type = Private::Pixmap;
    d->duration = (duration == 0) ? d->defaultDuration : duration;
    d->pixmap = pixmap;
    d->alignment = align;
    QTimer::singleShot( 0, this, SLOT(fadeIn()) );
}

void Flash::kill()
{
    if( d->state == Private::Visible )
        fadeOut();
}

void Flash::fadeIn()
{
    d->state = Private::Visible;
    d->renderedPixmap = renderPixmap();
    d->animId = Plasma::Phase::self()->animateElement(this, Plasma::Phase::ElementAppear);
    Plasma::Phase::self()->setAnimationPixmap( d->animId, d->renderedPixmap );
    if( d->duration > 0 )
        QTimer::singleShot( d->duration, this, SLOT(fadeOut()) );
}

void Flash::fadeOut()
{
    if( d->state == Private::Invisible )
        return;    // Flash was already killed - do not animate again

    d->state = Private::Invisible;
    d->animId = Plasma::Phase::self()->animateElement(this, Plasma::Phase::ElementDisappear);
    Plasma::Phase::self()->setAnimationPixmap( d->animId, d->renderedPixmap );
}

QPixmap Flash::renderPixmap()
{
    QPixmap pm( size().toSize() );
    pm.fill(Qt::transparent);

    QPainter painter( &pm );
    if( d->type == Private::Text ) {
        painter.setPen( d->color );
        painter.setFont( d->font );
        painter.drawText( QRect( QPoint(0, 0), size().toSize() ), d->text, d->textOption);
    } else if( d->type == Private::Pixmap ) {
        QPoint p;
        if( d->alignment & Qt::AlignLeft )
            p.setX( 0 );
        else if( d->alignment & Qt::AlignRight )
            p.setX( pm.width() - d->pixmap.width() );
        else
            p.setX( (pm.width() - d->pixmap.width())/2 );

        if( d->alignment & Qt::AlignTop )
            p.setY( 0 );
        else if( d->alignment & Qt::AlignRight )
            p.setY( pm.height() - d->pixmap.height() );
        else
            p.setY( (pm.height() - d->pixmap.height())/2 );

        painter.drawPixmap( p, d->pixmap );
    }
    return pm;
}
void Flash::paintWidget(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)

    if( d->animId && !Plasma::Phase::self()->animationResult(d->animId).isNull() ) {
        painter->drawPixmap( 0, 0, Plasma::Phase::self()->animationResult(d->animId) );
    } else if( d->state == Private::Visible ) {
        painter->drawPixmap( 0, 0, d->renderedPixmap );
    }
}

#include "flash.moc"
