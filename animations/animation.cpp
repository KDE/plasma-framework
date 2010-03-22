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
    : easingCurve(QEasingCurve::Linear),
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

void Animation::setTargetWidget(QGraphicsWidget* widget)
{
    d->animObject = widget;
    if (parent() == 0) {
        setParent(widget);
    }
}

QGraphicsWidget* Animation::targetWidget() const
{
    return d->animObject.data();
}

void Animation::setEasingCurve(const QEasingCurve &curve)
{
    d->easingCurve = curve;
}

QEasingCurve Animation::easingCurve() const
{
    return d->easingCurve;
}

void Animation::updateCurrentTime(int currentTime)
{
    Q_UNUSED(currentTime)
}

} //namespace Plasma

#include <../animation.moc>
