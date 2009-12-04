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

#include "animationgroup.h"
#include <QMapIterator>
#include <QParallelAnimationGroup>
#include <QSequentialAnimationGroup>
#include <QDebug>

namespace Plasma
{

class AnimationGroupPrivate
{
public:
    AnimationGroupPrivate()
        : parallel(false),
          duration(0),
          anim(0)
    { }

    bool parallel;
    int duration;
    QAnimationGroup *anim;

};

AnimationGroup::AnimationGroup(QObject* parent)
    : QAbstractAnimation(parent),
      d(new AnimationGroupPrivate)
{
    d->anim = new QSequentialAnimationGroup(this);
}

AnimationGroup::~AnimationGroup()
{
    delete d;
}

void AnimationGroup::setParallel(bool parallel)
{
    if (isParallel() == parallel) {
        return;
    }

    d->parallel = parallel;

    QAnimationGroup *newGroup;
    if (parallel) {
        newGroup = new QParallelAnimationGroup(this);
    } else {
        newGroup = new QSequentialAnimationGroup(this);
    }

    while (d->anim->animationCount()) {
        newGroup->addAnimation(d->anim->takeAnimation(0));
    }

    delete d->anim;
    d->anim = newGroup;
}

bool AnimationGroup::isParallel() const
{
    return d->parallel;
}

int AnimationGroup::add(QAbstractAnimation* elem)
{
    d->anim->addAnimation(elem);
    calculateGroupDuration();
    return d->anim->animationCount();
}

void AnimationGroup::remove(QAbstractAnimation* elem)
{
    d->anim->removeAnimation(elem);
    calculateGroupDuration();
}

QAbstractAnimation* AnimationGroup::at(int id) const
{
    return d->anim->animationAt(id);
}

void AnimationGroup::remove(int id)
{
    d->anim->removeAnimation(d->anim->animationAt(id));
    calculateGroupDuration();
}

void AnimationGroup::updateCurrentTime(int currentTime)
{
    Q_UNUSED(currentTime)
}

void AnimationGroup::calculateGroupDuration()
{
    QAbstractAnimation *tmp;
    d->duration = 0;
    if (d->parallel) {
        for (int i = 0; i < d->anim->animationCount(); ++i) {
            tmp = d->anim->animationAt(i);
            if (d->duration < tmp->duration()) {
                d->duration = tmp->duration();
            }
        }
    } else {
        for (int i = 0; i < d->anim->animationCount(); ++i) {
            tmp = d->anim->animationAt(i);
            d->duration += tmp->duration();
        }
    }
}

int AnimationGroup::duration() const
{
    return d->duration;
}

void AnimationGroup::updateDirection(QAbstractAnimation::Direction direction)
{
    d->anim->setDirection(direction);
}

void AnimationGroup::updateState(QAbstractAnimation::State oldState, QAbstractAnimation::State newState)
{
    Q_UNUSED(oldState)
    switch (newState) {
        case Running:
            d->anim->start();
        break;

        case Paused:
            d->anim->pause();
        break;

        case Stopped:
            d->anim->stop();
        break;
    }
}

} //namespace Plasma

#include <../animationgroup.moc>
