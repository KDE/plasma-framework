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

class AbstractAnimationPrivate
{
public:

    AbstractAnimationPrivate();
    /**
     * Object the animation(s) should act upon.
     */
    QWeakPointer<QGraphicsWidget> animObject;

    /**
     * All animations will have *at least* one animation property
     */
    QWeakPointer<QAbstractAnimation> animation;

    /**
     * Animation direction: where the animation will move.
     */
    qint8 animDirection;

    /**
     * Animation distance: displacement factor for animations where
     * there is change in the position of animated widget.
     */
    qreal animDistance;

    /**
     * Animation visibility: whether to end the animation being visible
     * or not.
     */
    bool animVisible;

    /**
     * Animation easing curve type
     */
    QEasingCurve::Type easingCurve;

    /**
     * Animation direction, the idea is to offer a way
     * to rewind the animation by setDirection(QAbstractAnimation::Backward).
     */
    bool forwards;
};

}

#endif
