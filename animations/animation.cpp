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

#include "animation.h"
#include "private/animationprivate_p.h"

#include <QMapIterator>
#include <QObject>
#include <QParallelAnimationGroup>
#include <QSequentialAnimationGroup>
#include <QDebug>
#include <kdebug.h>
#include <kglobalsettings.h>

namespace Plasma
{


AnimationPrivate::AnimationPrivate()
    : dirtyFlag(false),
      easingCurve(QEasingCurve::Linear),
      forwards(QAbstractAnimation::Forward),
      duration(250)
{
}

Animation::Animation(QObject* parent)
    : QAbstractAnimation(parent),
      d(new AnimationPrivate)
{
}

Animation::~Animation()
{
    delete d;
}

int Animation::duration() const
{
    return d->duration;
}

void Animation::setDuration(int duration)
{
    d->duration = qMax(0, duration);
}

void Animation::setWidgetToAnimate(QGraphicsWidget* receiver)
{
    d->animObject = receiver;
}

QGraphicsWidget* Animation::widgetToAnimate()
{
    return d->animObject.data();
}

void Animation::setEasingCurveType(QEasingCurve::Type easingCurve)
{
    d->easingCurve = easingCurve;
}

QEasingCurve::Type Animation::easingCurveType() const
{
    return d->easingCurve;
}

QAbstractAnimation::Direction Animation::direction() const
{
    return d->forwards;
}

void Animation::updateDirection(QAbstractAnimation::Direction direction)
{
    d->forwards = direction;
}

void Animation::start(QAbstractAnimation::DeletionPolicy policy)
{
    /* TODO: Actually treat policy parameter */

    QAbstractAnimation* anim = render(parent());
    if (anim) {
        anim->setDirection(direction());
        anim->start();
    }
}


void Animation::updateCurrentTime(int currentTime)
{
    /**
     * XXX: not sure if is a bug in my code or Qt, but 'start()' is not being
     * called when the animation is inside of an animatin group.
     * The solution for while is to explicitly call it in 'updateCurrentTime'
     * and use this flag for control.
     */
    if (!d->dirtyFlag) {
        d->dirtyFlag = true;
        start();
    }

     if (d->forwards == QAbstractAnimation::Forward) {
        if (currentTime == duration()) {
            d->dirtyFlag = false;
            emit finished();
        }
    } else if (d->forwards == QAbstractAnimation::Backward) {
        if (currentTime == 0) {
            d->dirtyFlag = false;
            emit finished();
        }
    }

}

} //namespace Plasma

#include <../animation.moc>
