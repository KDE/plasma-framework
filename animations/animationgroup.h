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

/**
 * @brief A group of Animations and / or AnimationGroups.
 * @since 4.4
 */
class PLASMA_EXPORT AnimationGroup : public AbstractAnimation
{

    Q_OBJECT

public:

    AnimationGroup(QObject* parent = 0);
    virtual ~AnimationGroup();

    /**
     * Start the animation.
     */
    virtual void start();


    /**
     * @arg parallelness whether the animation should be in parallel or not
     */
    void setParallel(bool parallelness);

    /**
     * @return whether the current animation is parallel or not
     */
    bool isParallel() const;

    /**
     * Add an Animation or AnimationGroup to the group
     * @arg elem element to add
     * @return id of element added
     */
    int add(AbstractAnimation* elem);

    /**
     * Return element with given id
     * @return id of element to get
     */
    AbstractAnimation* at(int id) const;

    /**
     * Remove element with given id
     * @arg id id of element to remove
     */
    void remove(int id);

    /**
     * Take the animation object and turn it into a QAnimationGroup. More
     * specifically, a QSerialAnimation or QParallelAnimation depending on
     * the value of m_parallel at the time of invocation. Returns NULL on error.
     */
    QAnimationGroup* toQAbstractAnimation(QObject* parent = 0);

private:

    /**
     * Parent owner object to use in generated animations.
     */
    QObject* m_parent;

    /**
     * Boolean determining if the animation is parallel. Default is false.
     */
    bool m_parallel;

    /**
     * Map of AbstractAnimations to be run, by id.
     */
    QList<AbstractAnimation *> anims;

};


} //namespace Plasma

#endif
