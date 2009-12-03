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
 * @file This file contains the definition for the Slide effect.
 */

#ifndef PLASMA_ANIMATIONS_SLIDE_H
#define PLASMA_ANIMATIONS_SLIDE_H

#include "plasma/animations/animation.h"
#include "plasma/plasma_export.h"
#include "plasma/plasma.h"


namespace Plasma
{

class SlideAnimationPrivate;
/**
 * @class Slide plasma/animations/slide.h
 * @short Slide effect
 *
 * Effect that moves the object a specific distance in a given direction. The
 * object is optionally made invisible at the beginning or at the end.
 */
class SlideAnimation : public Animation
{
    Q_OBJECT
    Q_PROPERTY(qint8 movementDirection READ movementDirection WRITE setMovementDirection)
    Q_PROPERTY(qreal distance READ distance WRITE setDistance)

public:
    SlideAnimation(QObject *parent = 0,
                   AnimationDirection direction = MoveUp, qreal distance = 0);
    ~SlideAnimation();

    /**
     * Set the animation distance
     * @distance animation distance
     */
    void setDistance(qreal distance);

    /**
     * Get the animation distance
     */
    qreal distance() const;

    /**
     * Set the animation direction
     * @arg direction animation direction
     */
    void setMovementDirection(const qint8 &direction);

    /**
     * Get the animation direction
     */
    qint8 movementDirection() const;

    void setWidgetToAnimate(QGraphicsWidget *widget);

protected:
    virtual QAbstractAnimation* render(QObject* parent = 0);

private:
    /**
     * Animation direction: where the animation will move.
     */
    Plasma::AnimationDirection animDirection;

    /**
     * Animation distance: displacement factor for animations where
     * there is change in the position of animated widget.
     */
    qreal animDistance;

    QWeakPointer<QPropertyAnimation> animation;
};

}

#endif
