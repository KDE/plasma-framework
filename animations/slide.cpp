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

#include "slide_p.h"

#include <QPointF>
#include <kdebug.h>

namespace Plasma
{

void SlideAnimation::setDistance(qreal distance)
{
    m_animDistance = QPointF(distance,  0.0);
}

qreal SlideAnimation::distance() const
{
    return m_animDistance.x();
}

void SlideAnimation::setDistancePointF(const QPointF &distance)
{
    m_animDistance = distance;
}

QPointF SlideAnimation::distancePointF() const
{
    return m_animDistance;
}

SlideAnimation::~SlideAnimation()
{
}

SlideAnimation::SlideAnimation(QObject *parent,
                               MovementDirection direction,
                               qreal distance) : EasingAnimation(parent)
{
    setMovementDirection(direction);
    setDistance(distance);
    setEasingCurve(QEasingCurve::OutCirc);
}

void SlideAnimation::setMovementDirection(const Animation::MovementDirection &direction)
{
    m_animDirection = direction;
}

Animation::MovementDirection SlideAnimation::movementDirection() const
{
    return m_animDirection;
}

void SlideAnimation::updateEffectiveTime(int currentTime)
{
    QGraphicsWidget *w = targetWidget();
    if (w && state() == QAbstractAnimation::Running) {
        const qreal delta = currentTime / qreal(duration());
        w->setPos(m_startPos * (1-delta) + (m_targetPos * delta));
    }
}

void SlideAnimation::updateState(QAbstractAnimation::State newState, QAbstractAnimation::State oldState)
{
    if (oldState == QAbstractAnimation::Stopped && newState == QAbstractAnimation::Running) {
        if (!targetWidget()) {
            return;
        }
        m_startPos = targetWidget()->pos();

        qreal newX = m_startPos.x();
        qreal newY = m_startPos.y();

        QPointF actualDistance = (direction() == \
                                  QAbstractAnimation::Forward ? \
                                  distancePointF():-distancePointF());

        bool moveAnyOnly = true;

        if (m_animDirection.testFlag(MoveUp)) {
            newY -= actualDistance.x();
            moveAnyOnly = false;
        } else if (m_animDirection.testFlag(MoveDown)) {
            newY += actualDistance.x();
            moveAnyOnly = false;
        }

        if (m_animDirection.testFlag(MoveRight)) {
            newX += actualDistance.x();
            moveAnyOnly = false;
        } else if (m_animDirection.testFlag(MoveLeft)) {
            newX -= actualDistance.x();
            moveAnyOnly = false;
        }

        if (moveAnyOnly && m_animDirection.testFlag(MoveAny)) {
            newX += actualDistance.x();
            newY += actualDistance.y();
        }

        if (direction() == QAbstractAnimation::Forward) {
            m_targetPos = QPointF(newX, newY);
        } else {
            m_targetPos = m_startPos;
            m_startPos = QPointF(newX, newY);
        }
    }
}

} //namespace Plasma

#include <../slide_p.moc>
