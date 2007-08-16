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

#ifndef __LAYOUTANIMATOR__
#define __LAYOUTANIMATOR__ 

#include <QtCore/QRectF>
#include <QtCore/QObject>

#include <plasma/plasma_export.h>

class QTimeLine;

namespace Plasma
{

class Layout;
class LayoutItem;

/** 
 * LayoutAnimator can be used to animate changes
 * in Layouts.
 *
 * @code
 *
 * LayoutAnimator* animator = new LayoutAnimator;
 * QTimeLine* timeLine = new QTimeLine;
 *
 * animator->setTimeLine(timeLine);
 * animator->setEffect( LayoutAnimator::InsertedState , LayoutAnimator::FadeInMoveEffect );
 * animator->setEffect( LayoutAnimator::NormalState , LayoutAnimator::MoveEffect );
 * animator->setEffect( LayoutAnimator::RemovedState , LayoutAnimator::FadeOutMoveEffect );
 * myLayout->setAnimator(animator);
 *
 * @endcode
 */
class PLASMA_EXPORT LayoutAnimator : public QObject
{
Q_OBJECT

public:
    /** Constructs a new layout animator with the specified parent. */
    explicit LayoutAnimator(QObject* parent = 0);
    ~LayoutAnimator();

    /**
     * This enum describes the possible states which a layout item may be in.  
     * Different effects can be defined for items which are being inserted, 
     * moved or resized or removed from layouts.
     */
    enum State
    {
        /** 
         * State for an item which has recently been added to a layout. 
         * When the animation completes, the item's state will change to
         * NormalState 
         */
        InsertedState,
        /** 
         * Normal state for items in the layout. 
         * Items will remain in this state until it is explicitly changed
         * via setCurrentState()
         */
        NormalState,
        /** 
         * State for an item which is being removed from a layout. 
         * When the animation completes, the item will be removed from the 
         * animator and its state will be undefined. 
         */
        RemovedState
    };

    /** 
     * This enum describes the available effects which can be used 
     * to animate changes in a layout.
     */
    enum Effect
    {
        /** 
         * No effect.  When the animation begins, the item immediately appears 
         * in its final position and size. 
         */
        NoEffect,
        /**
         * The item is smoothly moved and resized from its initial geometry to its final
         * geometry as the animation progresses. 
         */
        MoveEffect,
        /** 
         * The item fades out during the first half of the animation in its initial geometry
         * and then fades in at its final position and size during the second half of
         * the animation.
         */
        FadeEffect,
        /**
         * The item is initially invisible and fades in whilst moving and resizing to
         * its final position as the animation progresses.
         */
        FadeInMoveEffect,
        /**
         * The item is initially fully opqaue and fades out whilst moving and resizing
         * to its final position as the animation progresses.
         */
        FadeOutMoveEffect
    };

    /**
     * Sets the @p effect for items in the layout which are under-going a change
     * specified by @p action.
     *
     * This allows different effects to be defined for items which are being added to,
     * removed from, or repositioned inside layouts.
     */
    void setEffect( State state , int effect );
    /** See setEffect() */
    int effect( State state ) const;

    /** 
     * Sets the current action for a particular layout item.  The Layout class
     * should call this before changing an item so that the animator can apply the correct
     * animation.
     *
     * When the current animation completes, depending on the current @p state, the item
     * may advance into a new state.
     */
    void setCurrentState( LayoutItem* item , State state );
    /** See setCurrentState() */
    State state( LayoutItem* item ) const;

    /** 
     * Sets the new geometry for a layout item.  
     * The item will animate from its current geometry to @p geometry, using
     * the effect specified in setEffect() 
     */
    virtual void setGeometry( LayoutItem* item , const QRectF& geometry );

    /** 
     * Sets the time line used by this animator. 
     * 
     * The duration of the animation can be changed by altering @p timeLine 's duration
     */
    void setTimeLine( QTimeLine* timeLine );
    /** Returns the QTimeLine used by this animator. */
    QTimeLine* timeLine() const;

protected:
    virtual void updateItem( qreal value , LayoutItem* item );

private slots:
    void valueChanged(qreal value);
    
private:
    void moveEffectUpdateItem(qreal value,LayoutItem* item,Effect effect);
    void noEffectUpdateItem(qreal value,LayoutItem* item);
    void fadeEffectUpdateItem(qreal value,LayoutItem* item);

    void animationFinished(LayoutItem* item);

private:
    class Private;
    Private* const d;

};

}

#endif // __LAYOUTANIMATOR__ 

