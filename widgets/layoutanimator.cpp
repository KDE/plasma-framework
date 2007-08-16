/*
    Copyright 2007 by Robert Knight <robertknight@gmail.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301  USA.
*/

#include "layoutanimator.h"

#include <QtCore/QHash>
#include <QtCore/QList>
#include <QtCore/QPointer>
#include <QtCore/QTimeLine>
#include <QtGui/QGraphicsItem>
#include <QtGui/QGraphicsScene>
#include <QtDebug>

#include "layout.h"
#include "widget.h"

using namespace Plasma;

class LayoutAnimator::Private
{
public:
    QHash<LayoutAnimator::State,int> effects;  
    
    class ItemGeometry
    {
    public:
        QRectF startGeometry;
        QRectF endGeometry;
    };  
    
    QHash<LayoutItem*,ItemGeometry> geometries;
    QHash<LayoutItem*,LayoutAnimator::State> states;

    QPointer<QTimeLine> timeLine;

    qreal lastValue;

    qreal delta(qreal currentValue) const
    {
        if ( currentValue > lastValue ) 
            return currentValue - lastValue;
        else
            return (1.0-lastValue) + currentValue;
    }

    QRectF interpolateGeometry(LayoutItem* item,qreal value) const
    {
        QRectF newGeometry;

        const QRectF& current = geometries[item].startGeometry;
        const QRectF& next = geometries[item].endGeometry;

        newGeometry.setLeft(current.left() + (next.left()-current.left()) * value);
        newGeometry.setRight(current.right() + (next.right()-current.right()) * value);
        newGeometry.setTop(current.top() + (next.top()-current.top()) * value);
        newGeometry.setBottom(current.bottom() + (next.bottom()-current.bottom()) * value);

        return newGeometry;
    }
};

LayoutAnimator::LayoutAnimator(QObject* parent)
: QObject(parent),
  d(new Private)
{
    d->lastValue = 0;
}

LayoutAnimator::~LayoutAnimator()
{
    delete d;
}

void LayoutAnimator::setEffect( State action , int effect )
{
    d->effects[action] = effect;
}
int LayoutAnimator::effect(State action) const
{
    return d->effects[action];
}

void LayoutAnimator::setCurrentState( LayoutItem* item , State action )
{
    d->states[item] = action;
}
LayoutAnimator::State LayoutAnimator::state( LayoutItem* item ) const
{
    return d->states[item];
}

void LayoutAnimator::setTimeLine(QTimeLine* timeLine)
{
    if ( d->timeLine ) {
        disconnect( d->timeLine , SIGNAL(valueChanged(qreal)) , this , 
                SLOT(valueChanged(qreal)) );
        disconnect( d->timeLine , SIGNAL(finished()) , this , 
                SLOT(animationCompleted()) );
    }

    d->timeLine = timeLine;

    connect( d->timeLine , SIGNAL(valueChanged(qreal)) , this , 
             SLOT(valueChanged(qreal)) );
    connect( d->timeLine , SIGNAL(finished()) , this , 
             SLOT(animationCompleted()) );
}
QTimeLine* LayoutAnimator::timeLine() const
{
    return d->timeLine;
}
void LayoutAnimator::valueChanged(qreal value)
{
    foreach( LayoutItem* item , d->geometries.keys() ) {
        updateItem(value,item);
    }

    d->lastValue = value;
}

void LayoutAnimator::setGeometry( LayoutItem* item , const QRectF& destGeometry )
{
    Q_ASSERT( item );

    Private::ItemGeometry& itemGeometry = d->geometries[item];

    itemGeometry.startGeometry = item->geometry();
    itemGeometry.endGeometry = destGeometry;
}

void LayoutAnimator::moveEffectUpdateItem( qreal value , LayoutItem* item , Effect effect )
{
    Widget* widget = dynamic_cast<Widget*>(item->graphicsItem());

    if ( widget && effect == FadeInMoveEffect )
        widget->setOpacity( qMin(1.0,widget->opacity()+d->delta(value)) );
    else if ( widget && effect == FadeOutMoveEffect )
        widget->setOpacity( qMax(0.0,widget->opacity()-d->delta(value)) );

    item->setGeometry( d->interpolateGeometry(item,value) );
}

void LayoutAnimator::noEffectUpdateItem( qreal , LayoutItem* item )
{
    item->setGeometry( d->geometries[item].endGeometry );
}

void LayoutAnimator::fadeEffectUpdateItem( qreal value , LayoutItem* item )
{
    Widget* widget = dynamic_cast<Widget*>(item->graphicsItem());

    qreal threshold = 0;
    
    if ( widget != 0 && d->geometries[item].startGeometry != d->geometries[item].endGeometry ) { 
        widget->setOpacity( qAbs( (value*2)-1.0 ) );
        threshold = 0.5; 
    }

    QRectF newGeometry;
    
    if ( value < threshold )
        newGeometry = d->geometries[item].startGeometry;
    else
        newGeometry = d->geometries[item].endGeometry;

    item->setGeometry(newGeometry);
}

void LayoutAnimator::animationFinished(LayoutItem* item)
{
    switch (d->states[item]) 
    {
        case InsertedState:
            d->states[item] = NormalState;
            break;
        case RemovedState:
                d->states.remove(item);
                d->geometries.remove(item);
            break;
        case NormalState:
                // do nothing
            break;
        default:
            Q_ASSERT(false);
    }
}

void LayoutAnimator::updateItem( qreal value , LayoutItem* item )
{
    Q_ASSERT( value >= 0 && value <= 1.0 );
    Q_ASSERT( item );
    Q_ASSERT( d->geometries.contains(item) );

    switch ( effect(d->states[item]) )
    {
        case NoEffect:
                noEffectUpdateItem(value,item);
            break;
        case MoveEffect:
                moveEffectUpdateItem(value,item,MoveEffect);
            break;
        case FadeInMoveEffect:
                moveEffectUpdateItem(value,item,FadeInMoveEffect);
            break;
        case FadeOutMoveEffect:
                moveEffectUpdateItem(value,item,FadeOutMoveEffect);
            break;
        case FadeEffect:
                fadeEffectUpdateItem(value,item);
            break;
        default:
            Q_ASSERT(false);
    }
}

#include "layoutanimator.moc"

