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

#ifndef PLASMA_ANIMATIONS_SLIDE_P_H
#define PLASMA_ANIMATIONS_SLIDE_P_H

#include "plasma/animations/easinganimation_p.h"
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
class SlideAnimation : public EasingAnimation
{
    Q_OBJECT
    Q_PROPERTY(qreal distance READ distance WRITE setDistance)
    Q_PROPERTY(MovementDirection movementDirection READ movementDirection WRITE setMovementDirection)
    Q_PROPERTY(QPointF distancePointF READ distancePointF WRITE setDistancePointF)

public:
    explicit SlideAnimation(QObject *parent = 0, MovementDirection direction = MoveUp, qreal distance = 0);
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

    void setDistancePointF(const QPointF &distance);
    QPointF distancePointF() const;

    /**
     * Set the animation direction
     * @arg direction animation direction
     */
    void setMovementDirection(const Animation::MovementDirection&direction);

    /**
     * Get the animation direction
     */
    Animation::MovementDirection movementDirection() const;

protected:
    void updateEffectiveTime(int currentTime);
    void updateState(QAbstractAnimation::State newState, QAbstractAnimation::State oldState);

private:
    /**
     * Animation direction: where the animation will move.
     */
    Animation::MovementDirection m_animDirection;

    /**
     * Animation distance: displacement factor for animations where
     * there is change in the position of animated widget.
     */
    QPointF m_animDistance;
    QPointF m_startPos;
    QPointF m_targetPos;
};

}

#endif // PLASMA_ANIMATIONS_SLIDE_P_H
