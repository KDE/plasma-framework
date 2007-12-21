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

#include "layouts/layout.h"
#include "widgets/widget.h"

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
    bool autoDeleteOnRemoval;

    Private() 
        : lastValue(0)
        , autoDeleteOnRemoval(false)
    {
    }

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

    void prepareItemForState( LayoutItem *item , LayoutAnimator::State state ) {
        
        // opacity setting for widgets
        if ( state == InsertedState && effects[state] == LayoutAnimator::FadeInMoveEffect ) {
            Widget *widget = dynamic_cast<Widget*>(item->graphicsItem());

            if ( widget ) {
                widget->setOpacity(0); // item is invisible immediately after insertion     
            }
        }
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

void LayoutAnimator::setAutoDeleteOnRemoval(bool autoDelete) 
{
    if ( d->autoDeleteOnRemoval == autoDelete )
        return;

    d->autoDeleteOnRemoval = autoDelete;

    if ( autoDelete ) { 
        connect( this , SIGNAL(stateChanged(LayoutItem*,State,State)) , this , 
                 SLOT(itemAutoDeleter(LayoutItem*,State,State)) );
    } else { 
        disconnect( this , SIGNAL(stateChanged(LayoutItem*,State,State)) , this , 
                SLOT(itemAutoDeleter(LayoutItem*,State,State)) );
    } 
}

bool LayoutAnimator::autoDeleteOnRemoval() const
{
    return d->autoDeleteOnRemoval;
}

void LayoutAnimator::itemAutoDeleter(LayoutItem *item , State oldState , State newState)
{
    if ( oldState == RemovedState && newState == DeadState ) {
        if ( item->graphicsItem() ) {
            item->graphicsItem()->scene()->removeItem( item->graphicsItem() );

            if ( dynamic_cast<QGraphicsItem*>(item) != dynamic_cast<QGraphicsItem*>(item->graphicsItem()) )
                delete item->graphicsItem();
        }

        delete item;
    }
}

void LayoutAnimator::setEffect( State action , int effect )
{
    d->effects[action] = effect;
}
int LayoutAnimator::effect(State action) const
{
    return d->effects[action];
}

void LayoutAnimator::setCurrentState( LayoutItem* item , State state )
{
    if (state == RemovedState && !d->states.contains(item)) {
        return;
    }

    State oldState = d->states[item];

    d->states[item] = state;
    d->prepareItemForState(item,state);

    emit stateChanged(item,oldState,state);
}
LayoutAnimator::State LayoutAnimator::state( LayoutItem* item ) const
{
    if ( !d->states.contains(item) ) {
        return DeadState;
    } else {
        return d->states[item];
    }
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
void LayoutAnimator::animationCompleted()
{
    foreach( LayoutItem* item , d->states.keys() ) {
        animationFinished(item);
    }
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
        widget->setOpacity( qMin(qreal(1.0),widget->opacity()+d->delta(value)) );
    else if ( widget && effect == FadeOutMoveEffect )
        widget->setOpacity( qMax(qreal(0.0),widget->opacity()-d->delta(value)) );

    
    if ( effect == FadeInMoveEffect || effect == FadeOutMoveEffect ) {
        const QRectF finalGeometry = d->geometries[item].endGeometry;

        if ( item->geometry() != finalGeometry )
            item->setGeometry( finalGeometry ); 
    }
    else 
        item->setGeometry( d->interpolateGeometry(item,value) );
}

void LayoutAnimator::noEffectUpdateItem( qreal , LayoutItem* item )
{
    const QRectF finalGeometry = d->geometries[item].endGeometry;

    if ( item->geometry() != finalGeometry )
        item->setGeometry( finalGeometry );
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
    switch ( state(item) ) 
    {
        case InsertedState:
                setCurrentState(item,StandardState);
            break;
        case RemovedState:
                d->states.remove(item);
                d->geometries.remove(item);

                emit stateChanged( item , RemovedState , DeadState );
            break;
        case StandardState:
                    d->geometries[item].startGeometry = d->geometries[item].endGeometry;
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

