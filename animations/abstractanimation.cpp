/*
 *   Copyright 2009 Mehmet Ali Akmanalp <makmanalp@wpi.edu>
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

#include "abstractanimation.h"
#include "private/animationprivate_p.h"

#include <QEasingCurve>

namespace Plasma
{


AbstractAnimation::AbstractAnimation(): d(new AnimationPrivate)
{
    d->easingCurve = QEasingCurve::Linear;
}

AbstractAnimation::~AbstractAnimation()
{
    delete d;
}

void AbstractAnimation::setWidget(QGraphicsWidget* receiver)
{
    d->animObject = receiver;
}

QGraphicsWidget* AbstractAnimation::getAnimatedObject()
{
    return d->animObject;
}

void AbstractAnimation::setEasingCurveType(QEasingCurve::Type easingCurve)
{
    d->easingCurve = easingCurve;
}

QEasingCurve::Type AbstractAnimation::easingCurveType() const
{
    return d->easingCurve;
}

void AbstractAnimation::setAnimationDirection(AnimationDirection animationDirection)
{
    d->animDirection = animationDirection;
}

AnimationDirection AbstractAnimation::animationDirection() const
{
    return d->animDirection;
}

void AbstractAnimation::setAnimationDistance(qreal animationDistance)
{
    d->animDistance = animationDistance;
}

qreal AbstractAnimation::animationDistance() const
{
    return d->animDistance;
}

void AbstractAnimation::setAnimationVisible(bool animationVisible)
{
    d->animVisible = animationVisible;
}

bool AbstractAnimation::animationVisible() const
{
    return d->animVisible;
}

AnimationPrivate* AbstractAnimation::getAnimationPrivate()
{
    return d;
}

} //namespace Plasma

#include <../abstractanimation.moc>
