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
 * @file This file contains the classes for AbstractAnimation, which is the
 * abstract base class for all animations.
 */

#ifndef PLASMA_ABSTRACTANIMATION_H
#define PLASMA_ABSTRACTANIMATION_H

#include <QAbstractAnimation>
#include <QGraphicsWidget>
#include <QObject>

#include <plasma/plasma.h>
#include <plasma/plasma_export.h>

namespace Plasma
{

class AnimationPrivate;

/**
 * Abstract base class for AnimationGroup and Animation.
 */
class PLASMA_EXPORT AbstractAnimation : public QObject
{

    Q_OBJECT

public:

    AbstractAnimation();
    virtual ~AbstractAnimation() = 0;

    /**
     * Set the widget on which the animation is to be performed.
     * @arg receiver The QGraphicsWidget to be animated.
     */
    virtual void setWidget(QGraphicsWidget* receiver);

    /**
     * Take an AbstractAnimation and turn it into a 
     * QAbstractAnimation.
     */
    virtual QAbstractAnimation* toQAbstractAnimation(QObject* parent) = 0;

public slots:

    /**
     * Start the animation.
     */
    virtual void start() = 0;

protected:

    QGraphicsWidget *getAnimatedObject();

    AnimationPrivate *getAnimationPrivate();

private:

    AnimationPrivate *d;
};

} //namespace Plasma

#endif
