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

/**
 * @file This file contains the class for animation groups in plasma.
 */

#ifndef PLASMA_ANIMATIONGROUP_H
#define PLASMA_ANIMATIONGROUP_H

#include <QAbstractAnimation>
#include <QAnimationGroup>
#include <QGraphicsWidget>
#include <QList>
#include <QObject>

#include <plasma/animations/abstractanimation.h>
#include <plasma/plasma_export.h>

namespace Plasma
{

class AnimationGroupPrivate;

/**
 * @brief A group of Animations and / or AnimationGroups.
 * @since 4.4
 */
class PLASMA_EXPORT AnimationGroup : public AbstractAnimation
{
    Q_OBJECT
    Q_PROPERTY(bool parallel READ isParallel WRITE setParallel)

public:

    explicit AnimationGroup(QObject* parent = 0);
    virtual ~AnimationGroup();

    /**
     * @arg parallelness whether the animation should be in parallel or not
     */
    void setParallel(bool parallelness);

    /**
     * @return whether the current animation is parallel or not
     */
    bool isParallel() const;

    /**
     * Take the animation object and turn it into a QAnimationGroup. More
     * specifically, a QSerialAnimation or QParallelAnimation depending on
     * the value of m_parallel at the time of invocation. Returns NULL on error.
     */
    QAnimationGroup* toQAbstractAnimation(QObject* parent = 0);

public Q_SLOTS:
    /**
     * Add an Animation or AnimationGroup to the group
     * @arg elem element to add
     * @return id of element added
     */
    Q_INVOKABLE int add(Plasma::AbstractAnimation* elem);

    /**
     * Remove an Animation or AnimationGroup from this group
     * @arg eleme element to remove
     */
    Q_INVOKABLE void remove(Plasma::AbstractAnimation* elem);

    /**
     * Return element with given id
     * @return id of element to get
     */
    Q_INVOKABLE AbstractAnimation* at(int id) const;

    /**
     * Remove element with given id
     * @arg id id of element to remove
     */
    Q_INVOKABLE void remove(int id);

private:
    AnimationGroupPrivate * const d;
};


} //namespace Plasma

#endif
