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

namespace Plasma
{

class AnimationGroupPrivate
{
public:
    /**
     * Boolean determining if the animation is parallel. Default is false.
     */
    bool parallel;

    /**
     * Map of AbstractAnimations to be run, by id.
     */
    QList<AbstractAnimation *> anims;
};

AnimationGroup::AnimationGroup(QObject* parent)
    : AbstractAnimation(parent),
      d(new AnimationGroupPrivate)
{
    d->parallel = false;
}

AnimationGroup::~AnimationGroup()
{
    delete d;
}

void AnimationGroup::setParallel(bool parallelness)
{
    d->parallel = parallelness;
}

bool AnimationGroup::isParallel() const
{
    return d->parallel;
}

int AnimationGroup::add(AbstractAnimation* elem)
{
    d->anims.insert(d->anims.count(), elem);
    return d->anims.count();
}

AbstractAnimation* AnimationGroup::at(int id) const
{
    return d->anims.value(id);
}

void AnimationGroup::remove(int id)
{
    if (id >= d->anims.count() || id < 0) {
        return;
    }

    d->anims.removeAt(id);
}

void AnimationGroup::start()
{
    QAbstractAnimation* anim = toQAbstractAnimation(parent());
    if (anim){
        /* FIXME: why to create and delete all the animations
         * every single time it runs?
         */
        anim->start(QAbstractAnimation::DeleteWhenStopped);
    }
}

QAnimationGroup* AnimationGroup::toQAbstractAnimation(QObject* parent)
{
    //if supplied, use parent given in args.
    if (!parent){
        parent = this->parent();
    }

    QAnimationGroup* g;
    if (d->parallel) {
        g = new QParallelAnimationGroup(parent);
    } else {
        g = new QSequentialAnimationGroup(parent);
    }

    QListIterator<AbstractAnimation*> it(d->anims);
    while (it.hasNext()) {
        //add with group as owner
        g->addAnimation(it.next()->toQAbstractAnimation(g));
    }

    return g;
}

} //namespace Plasma

#include <../animationgroup.moc>
