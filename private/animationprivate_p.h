/***********************************************************************/
/* animationprivate.h                                                  */
/*                                                                     */
/* Copyright(C) 2009 Adenilson Cavalcanti <adenilson.silva@idnt.org.br>*/
/*                                                                     */
/* This library is free software; you can redistribute it and/or       */
/* modify it under the terms of the GNU Lesser General Public          */
/* License as published by the Free Software Foundation; either        */
/* version 2.1 of the License, or (at your option) any later version.  */
/*                                                                     */
/* This library is distributed in the hope that it will be useful,     */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of      */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU   */
/* Lesser General Public License for more details.                     */
/*                                                                     */
/* You should have received a copy of the GNU Lesser General Public    */
/* License along with this library; if not, write to the Free Software */
/* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA       */
/* 02110-1301  USA                                                     */
/***********************************************************************/
#ifndef PLASMA_ANIMATIONPRIVATE_H
#define PLASMA_ANIMATIONPRIVATE_H

#include <QEasingCurve>
#include <QWeakPointer>
class QAbstractAnimation;

namespace Plasma
{

class AnimationPrivate
{
public:

    AnimationPrivate();
    /**
     * Object the animation(s) should act upon.
     */
    QWeakPointer<QGraphicsWidget> animObject;

    /**
     * Animation visibility: whether to end the animation being visible
     * or not.
     */
    bool animVisible;

    /**
     * XXX: not sure if is a bug in my code or Qt, but 'start()' is not being
     * called when the animation is inside of an animatin group.
     * The solution for while is to explicitly call it in 'updateCurrentTime'
     * and use this flag for control.
     */
    bool dirtyFlag;

    /**
     * Animation easing curve type
     */
    QEasingCurve::Type easingCurve;

    /**
     * Animation direction, the idea is to offer a way
     * to rewind the animation by setDirection(QAbstractAnimation::Backward).
     * TODO: map this to QAbstractAnimation::Direction
     */
    QAbstractAnimation::Direction forwards;

    /**
     * Duration of the animation. Default is 250ms.
     * TODO: map this to QAbstractAnimation::duration
     */
    int duration;

};

}

#endif
