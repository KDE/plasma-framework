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
 * @file This file contains the abstract base class for all singular
 * animations.
 */

#ifndef PLASMA_ANIMATION_H
#define PLASMA_ANIMATION_H

#include <QGraphicsWidget>
#include <QObject>
#include <QPropertyAnimation>

#include <plasma/animations/abstractanimation.h>
#include <plasma/plasma_export.h>

namespace Plasma
{

class AnimationPrivate;

/**
 * Abstract representation of a single animation.
 */
class PLASMA_EXPORT Animation : public AbstractAnimation
{

    Q_OBJECT

public:

    Animation(QObject* parent = 0);
    virtual ~Animation() = 0;

    /**
     * Start the animation.
     */
    virtual void start();

    /**
     * Take the animation object and turn it into a QPropertyAnimation. Returns
     * NULL on error. This function just does some boilerplate checking and then
     * calls render().
     */
    QAbstractAnimation* getQtAnimation(QObject* parent = 0);

protected:

    /**
     * Change the animation duration. Default is 1000ms.
     * @arg duration The new duration of the animation.
     */
    virtual void setDuration(int duration = 250);

    /**
     * Each individual class must override this function to place their main
     * functionality. This function must take the values from the constructor,
     * do the appropriate calculations, and return a corresponding
     * QPropertyAnimation initialized with the given parent.
     */
    virtual QAbstractAnimation* render(QObject* parent = 0) = 0;

    /**
     * Get the animation duration.
     * @return duration in ms.
     */
    int getDuration();

private:

    AnimationPrivate *d;
};

} //namespace Plasma

#endif
