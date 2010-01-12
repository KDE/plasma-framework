/*
    Copyright (C) 2009 Igor Trindade Oliveira <igor.oliveira@indt.org.br>

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
 * @file This file contains the definition for the StackedRotationAnimation.
 */

#ifndef PLASMA_ROTATIONSTACKED_P_H
#define PLASMA_ROTATIONSTACKED_P_H

#include <plasma/animations/animation.h>
#include <plasma/plasma_export.h>

#include <QGraphicsLayoutItem>
class QGraphicsRotation;
class StackedLayout;

namespace Plasma {

/* TODO:
 * create a parent class for rotations
 */
/**
 * @class RotationStackedAnimation plasma/animations/rotationstacked_p.h
 * @short 3D like rotation animation
 * Use this class when you want to rotate a widget along an axis (e.g. Y)
 * and display a 'hidden' widget behind it. See also \ref RotationAnimation.
 */
class RotationStackedAnimation : public Animation
{
    Q_OBJECT
    Q_PROPERTY(qint8 movementDirection READ movementDirection WRITE setMovementDirection)
    Q_PROPERTY(QGraphicsLayoutItem* layout READ layout)
    Q_PROPERTY(qint8 reference READ reference WRITE setReference)
    Q_PROPERTY(QGraphicsWidget* backWidget READ backWidget WRITE setBackWidget)

public:
    explicit RotationStackedAnimation(QObject *parent = 0);

    ~RotationStackedAnimation();

    /**
     * Set the animation direction
     * @arg direction animation direction
     */
    void setMovementDirection(const qint8 &direction);

    /**
     * Get the animation direction
     */
    qint8 movementDirection() const;

    /**
     * Set rotation reference (e.g. Center, Up, Down, Left, Right) can
     * be combined (i.e. Center|Up)
     * @arg reference The reference
     */
    void setReference(const qint8 &reference);

    /**
     * Rotation reference (e.g. Center, Up, Down, Left, Right) can
     * be combined (i.e. Center|Up)
     */
    qint8 reference() const;

    /**
     * Get the layout where the widgetToAnimate and backWidget are.
     */
    QGraphicsLayoutItem *layout();

    /**
     * Get the back widget
     */

    QGraphicsWidget *backWidget();

    /**
     * Set the back widget that is used after the animation to be finished
     * @arg backWidget The back widget
     */
    void setBackWidget(QGraphicsWidget *backWidget);

protected:
    void updateState(QAbstractAnimation::State newState, QAbstractAnimation::State oldState);
    void updateCurrentTime(int currentTime);

private:
    /** Reference, the default is Up (see \ref Animation::Reference) */
    qint8 m_reference;
    /**
     * Animation direction: where the animation will move.
     */
    MovementDirection m_animDirection;
    /** Initial rotation angle from front widget */
    int m_frontStartAngle;
    /** End value of the rotation angle of the front widget */
    int m_frontEndAngle;
     /** Initial rotation angle from back widget */
    int m_backStartAngle;
    /** End value of the rotation angle of the back widget */
    int m_backEndAngle;
    /**  Object the animation(s) should act upon. */
    QWeakPointer<QGraphicsWidget> m_backWidget;
    /** Back Widget Rotation transform object */
    QGraphicsRotation *m_backRotation;
    /** Front Widget Rotation transform object */
    QGraphicsRotation *m_frontRotation;
    /** rotation stacked layout where the widget would be added */
    StackedLayout *m_sLayout;

};
} // Plasma

#endif // PLASMA_ROTATIONSTACKED_P_H
