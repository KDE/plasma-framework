/*
    Copyright (C) 2009 Igor Trindade Oliveira <igor.oliveira@indt.org.br>
    Copyright (C) 2009 Adenilson Cavalcanti <adenilson.silva@idnt.org.br>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

/**
 * @file This file contains the definition for the 2D Rotation effect.
 */

#ifndef PLASMA_ROTATION_P_H
#define PLASMA_ROTATION_P_H

#include <plasma/animations/animation.h>
#include <plasma/plasma_export.h>

#include <QVector3D>

class QGraphicsRotation;

namespace Plasma {
/**
 * @class RotationAnimation plasma/animations/rotation_p.h
 * @short 3D rotation animation.
 *
 * This animation rotates a QGraphicsWidget in a axis (bounce, reference, axis
 * and angle can be defined using properties). See also
 * \ref StackedRotationAnimation.
 */
class RotationAnimation : public Animation
{

    Q_OBJECT
    Q_PROPERTY(bool isBounceable READ isBounceable WRITE setBounceable)
    Q_PROPERTY(Qt::Axis axis READ axis WRITE setAxis)
    Q_PROPERTY(qint8 reference READ reference WRITE setReference)
    Q_PROPERTY(qreal angle READ angle WRITE setAngle)

public:
    /** Default constructor
     *
     * @param parent Animation object parent.
     * @param isBounceable Animation bounce behavior.
     * @param reference See \ref Animation::Reference.
     * @param axis Which axis to rotate (XAxis, YAxis, ZAxis).
     * @param angle Rotation angle (0 to 360)
     *
     */
    explicit RotationAnimation(QObject *parent = 0,
                               bool isBounceable = false,
                               qint8 reference = Center,
                               Qt::Axis axis = Qt::ZAxis,
                               qreal angle = 180);

    /** Destructor */
    ~RotationAnimation();

    /**
     * get animation bounce behavior
     */
    bool isBounceable() const;

    /**
     * set animation bounce behavior (it can be bounceable only if not part of
     * an animation group)
     * @arg isBounceable
     */
    void setBounceable(const bool isBounceable);

    /**
     * get animation rotation axis (e.g. YAxis, ZAxis, XAxis)
     */
    Qt::Axis axis() const;

    /**
     * set animation rotation axis
     * @arg axis Rotation (e.g. YAxis, ZAxis, XAxis)
     */
    void setAxis(const Qt::Axis &axis);

    /**
     * Rotation reference (e.g. Center, Up, Down, Left, Right) can
     * be combined (i.e. Center|Up)
     */
    qint8 reference() const;

    /**
     * Set rotation reference (e.g. Center, Up, Down, Left, Right) can
     * be combined (i.e. Center|Up)
     * @arg reference The reference
     */
    void setReference(const qint8 &reference);

    /**
     * Animation rotation angle (e.g. 45, 180, etc)
     */
    qreal angle() const;

    /**
     * Set animation rotation angle (e.g. 45, 180, etc)
     * @arg angle The angle
     */
    void setAngle(const qreal &angle);

protected:
    void updateState(QAbstractAnimation::State newState, QAbstractAnimation::State oldState);
    void updateCurrentTime(int currentTime);

private:
    /** Rotation transform object */
    QGraphicsRotation *m_rotation;
    /** Actual number of bounce steps */
    int m_bounceStep;
    /** Number of bounce steps */
    static const int s_bounceSteps;
    /** Animation bounce behavior */
    bool m_isBounceable;
    /** Rotation angle */
    qreal m_angle;
    /** Axis where to perform the rotation */
    Qt::Axis m_axis;
    /** Reference, the default is Up (see \ref Animation::Reference) */
    qint8 m_reference;
};
} // Plasma

#endif
