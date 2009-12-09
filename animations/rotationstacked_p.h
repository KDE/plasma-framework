/////////////////////////////////////////////////////////////////////////
// rotationstacked.h                                                   //
//                                                                     //
// Copyright(C) 2009 Igor Trindade Oliveira <igor.oliveira@indt.org.br>//
//                                                                     //
// This library is free software; you can redistribute it and/or       //
// modify it under the terms of the GNU Lesser General Public          //
// License as published by the Free Software Foundation; either        //
// version 2.1 of the License, or (at your option) any later version.  //
//                                                                     //
// This library is distributed in the hope that it will be useful,     //
// but WITHOUT ANY WARRANTY; without even the implied warranty of      //
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU   //
// Lesser General Public License for more details.                     //
//                                                                     //
// You should have received a copy of the GNU Lesser General Public    //
// License along with this library; if not, write to the Free Software //
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA       //
// 02110-1301  USA                                                     //
/////////////////////////////////////////////////////////////////////////

#ifndef PLASMA_ROTATIONSTACKED_H
#define PLASMA_ROTATIONSTACKED_H

#include <plasma/animations/animation.h>
#include <plasma/plasma_export.h>

#include <QGraphicsLayoutItem>
class QGraphicsRotation;
class StackedLayout;

namespace Plasma {

/* TODO:
 * create a parent class for rotations
 */
class RotationStackedAnimation : public Animation
{
    Q_OBJECT
    Q_PROPERTY(qint8 movementDirection READ movementDirection WRITE setMovementDirection)
    Q_PROPERTY(QGraphicsLayoutItem* layout READ layout)
    Q_PROPERTY(qint8 reference READ reference WRITE setReference)
    Q_PROPERTY(QGraphicsWidget* backWidget READ backWidget WRITE setBackWidget)

public:
    RotationStackedAnimation(QObject *parent = 0);

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

    void setReference(const qint8 &reference);
    qint8 reference() const;

    QGraphicsLayoutItem *layout();

    QGraphicsWidget *backWidget();
    void setBackWidget(QGraphicsWidget *backWidget);

protected:
    void updateState(QAbstractAnimation::State newState, QAbstractAnimation::State oldState);
    void updateCurrentTime(int currentTime);

private:
    qint8 m_reference;
    /**
     * Animation direction: where the animation will move.
     */
    Plasma::AnimationDirection animDirection;
    int frontStartAngle;
    int frontEndAngle;
    int backStartAngle;
    int backEndAngle;
    QWeakPointer<QGraphicsWidget> m_backWidget;
    QGraphicsRotation *backRotation;
    QGraphicsRotation *frontRotation;
    StackedLayout *sLayout;

};
} // Plasma

#endif
