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
#include "private/abstractanimationprivate_p.h"

#include <QEasingCurve>

#include <kdebug.h>

namespace Plasma
{

AbstractAnimationPrivate::AbstractAnimationPrivate()
    : animVisible(true),
      easingCurve(QEasingCurve::Linear),
      forwards(true)
{
}

AbstractAnimation::AbstractAnimation(QObject *parent)
    : QObject(parent),
      d(new AbstractAnimationPrivate)
{
}

AbstractAnimation::~AbstractAnimation()
{
    delete d;
}

void AbstractAnimation::setWidgetToAnimate(QGraphicsWidget* receiver)
{
    d->animObject = receiver;
    /* Changed the object, delete the animation */
    delete d->animation.data();
    d->animation.clear();

}

QGraphicsWidget* AbstractAnimation::widgetToAnimate()
{
    return d->animObject.data();
}

void AbstractAnimation::setEasingCurveType(QEasingCurve::Type easingCurve)
{
    d->easingCurve = easingCurve;
}

QEasingCurve::Type AbstractAnimation::easingCurveType() const
{
    return d->easingCurve;
}

void AbstractAnimation::setForwards(bool forwards)
{
    d->forwards = forwards;
}

bool AbstractAnimation::forwards() const
{
    return d->forwards;
}

void AbstractAnimation::setDirection(const qint8 &direction)
{
    d->animDirection = direction;
}

qint8 AbstractAnimation::direction() const
{
    return d->animDirection;
}

void AbstractAnimation::setDistance(qreal distance)
{
    d->animDistance = distance;
}

qreal AbstractAnimation::distance() const
{
    return d->animDistance;
}

void AbstractAnimation::setVisible(bool isVisible)
{
    d->animVisible = isVisible;
}

bool AbstractAnimation::isVisible() const
{
    return d->animVisible;
}

QAbstractAnimation* AbstractAnimation::animation()
{
    return d->animation.data();
}

void AbstractAnimation::setAnimation(QAbstractAnimation *obj)
{
    d->animation = obj;
}

void AbstractAnimation::start()
{
    QAbstractAnimation* anim = toQAbstractAnimation(parent());
    if (anim) {
        anim->setDirection(d->forwards ? QAbstractAnimation::Forward :
                                         QAbstractAnimation::Backward);
        anim->start();
    }
}

} //namespace Plasma

#include <../abstractanimation.moc>
