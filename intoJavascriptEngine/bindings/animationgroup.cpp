/*
 *   Copyright 2009 Aaron J. Seigo <aseigo@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License version 2 as
 *   published by the Free Software Foundation
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

#include "animationgroup_p.h"

namespace Plasma
{

ParallelAnimationGroup::ParallelAnimationGroup(QObject *parent)
    : QParallelAnimationGroup(parent)
{
}

void ParallelAnimationGroup::addAnimation(QAbstractAnimation *animation)
{
    QParallelAnimationGroup::addAnimation(animation);
}

QAbstractAnimation *ParallelAnimationGroup::animationAt(int index) const
{
    return QParallelAnimationGroup::animationAt(index);
}

int ParallelAnimationGroup::animationCount() const
{
    return QParallelAnimationGroup::animationCount();
}

void ParallelAnimationGroup::clear()
{
    QParallelAnimationGroup::clear();
}

int ParallelAnimationGroup::indexOfAnimation(QAbstractAnimation *animation) const
{
    return QParallelAnimationGroup::indexOfAnimation(animation);
}

void ParallelAnimationGroup::insertAnimation(int index, QAbstractAnimation *animation)
{
    QParallelAnimationGroup::insertAnimation(index, animation);
}

void ParallelAnimationGroup::removeAnimation(QAbstractAnimation *animation)
{
    QParallelAnimationGroup::removeAnimation(animation);
}


SequentialAnimationGroup::SequentialAnimationGroup(QObject *parent)
    : QSequentialAnimationGroup(parent)
{
}

void SequentialAnimationGroup::addAnimation(QAbstractAnimation *animation)
{
    QSequentialAnimationGroup::addAnimation(animation);
}

QAbstractAnimation *SequentialAnimationGroup::animationAt(int index) const
{
    return QSequentialAnimationGroup::animationAt(index);
}

int SequentialAnimationGroup::animationCount() const
{
    return QSequentialAnimationGroup::animationCount();
}

void SequentialAnimationGroup::clear()
{
    QSequentialAnimationGroup::clear();
}

int SequentialAnimationGroup::indexOfAnimation(QAbstractAnimation *animation) const
{
    return QSequentialAnimationGroup::indexOfAnimation(animation);
}

void SequentialAnimationGroup::insertAnimation(int index, QAbstractAnimation *animation)
{
    QSequentialAnimationGroup::insertAnimation(index, animation);
}

void SequentialAnimationGroup::removeAnimation(QAbstractAnimation *animation)
{
    QSequentialAnimationGroup::removeAnimation(animation);
}

} // namespace Plasma

#include "animationgroup_p.moc"
