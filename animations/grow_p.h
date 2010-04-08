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
 * @file This file contains the definition for the Grow effect.
 */

#ifndef PLASMA_ANIMATIONS_GROW_P_H
#define PLASMA_ANIMATIONS_GROW_P_H

#include <plasma/animations/easinganimation_p.h>
#include <plasma/plasma_export.h>

namespace Plasma
{

/**
 * @class GrowAnimation plasma/animations/grow.h
 * @short Grow effect
 *
 * Effect that grows any QGraphicsWidget by a multiple given in the
 * constructor. The center of the object stays in place while the sides grow
 * (it does the animation by changing the objects geometry). Also see
 * \ref ZoomAnimation.
 */
class GrowAnimation : public EasingAnimation
{
    Q_OBJECT
    Q_PROPERTY(qreal factor READ factor WRITE setFactor)

public:
    /** Default Constructor
     * @param parent Animation object parent.
     * @param factor Expand factor (default is twice the size of
     * animated widget).
     */
    explicit GrowAnimation(QObject *parent = 0, qreal factor = 2);

    /** Destructor */
    virtual ~GrowAnimation(){};

    /**
     * Access expansion factor of the shadow pulsable copy.
     *
     * If not set, the default is twice (2x) the size of animated widget.
     * See \ref setFactor.
     * @return Expansion factor.
     */
    qreal factor() const;
    /**
     * Set expansion factor of target widget.
     *
     * If not set, the default is twice (2x) the size of animated widget.
     * @param factor A expansion factor
     */
    void setFactor(const qreal factor);

protected:
    void updateEffectiveTime(int currentTime);
    void updateState(QAbstractAnimation::State newState, QAbstractAnimation::State oldState);

private:
    /** Animation grow factor */
    qreal m_animFactor;
    /** Widget start geometry */
    QRectF m_startGeometry;
    /** Widget final geometry */
    QRectF m_targetGeometry;
};

}

#endif // PLASMA_ANIMATIONS_GROW_P_H
