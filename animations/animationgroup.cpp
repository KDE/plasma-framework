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

AnimationGroup::AnimationGroup(QObject* parent)
    : m_parent(parent),
      m_parallel(false)
{
}

AnimationGroup::~AnimationGroup()
{
}

void AnimationGroup::setParallel(bool parallelness)
{
    m_parallel = parallelness;
}

bool AnimationGroup::isParallel() const
{
    return m_parallel;
}

int AnimationGroup::add(AbstractAnimation* elem)
{
    anims.insert(anims.count(), elem);
    return anims.count();
}

AbstractAnimation* AnimationGroup::at(int id) const
{
    return anims[id];
}

void AnimationGroup::remove(int id)
{
    anims.removeAt(id);
}

void AnimationGroup::start()
{
    QAbstractAnimation* anim = asQAnimation(m_parent);
    if (anim){
        /* FIXME: why to create and delete all the animations
         * every single time it runs?
         */
        anim->start(QAbstractAnimation::DeleteWhenStopped);
    }
}

QAnimationGroup* AnimationGroup::asQAnimation(QObject* parent)
{
    //if supplied, use parent given in args.
    QObject* correctParent;
    if (parent){
        correctParent = parent;
    } else {
        correctParent = m_parent;
    }

    QAnimationGroup* g;
    if (m_parallel) {
        g = new QParallelAnimationGroup(correctParent);
    } else {
        g = new QSequentialAnimationGroup(correctParent);
    }

    QListIterator<AbstractAnimation*> it(anims);
    while (it.hasNext()) {
        //add with group as owner
        g->addAnimation(it.next()->asQAnimation(g));
    }

    return g;
}

} //namespace Plasma

#include <../animationgroup.moc>
