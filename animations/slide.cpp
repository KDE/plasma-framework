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
    m_animDistance = distance;
}

qreal SlideAnimation::distance() const
{
    return m_animDistance;
}

SlideAnimation::~SlideAnimation()
{
}

SlideAnimation::SlideAnimation(QObject *parent,
                               AnimationDirection direction,
                               qreal distance)
          : Animation(parent)
{
    setMovementDirection(direction);
    setDistance(distance);
}

void SlideAnimation::setMovementDirection(const qint8 &direction)
{
    m_animDirection = static_cast<Plasma::AnimationDirection>(direction);
}

qint8 SlideAnimation::movementDirection() const
{
    return static_cast<qint8>(m_animDirection);
}

void SlideAnimation::updateCurrentTime(int currentTime)
{
    QGraphicsWidget *w = widgetToAnimate();
    if (w && state() == QAbstractAnimation::Running) {
        qreal delta = easingCurve().valueForProgress(
                currentTime / qreal(duration()));
        w->setPos(m_startPos * (1-delta) + (m_targetPos * delta));
    }
}

void SlideAnimation::updateState(QAbstractAnimation::State newState, QAbstractAnimation::State oldState)
{
    if (oldState == QAbstractAnimation::Stopped && newState == QAbstractAnimation::Running) {
        if (!widgetToAnimate()) {
            return;
        }
        m_startPos = widgetToAnimate()->pos();

        qreal newX = m_startPos.x();
        qreal newY = m_startPos.y();

        int actualDistance = (direction() == QAbstractAnimation::Forward?distance():-distance());
        switch (movementDirection()) {
        case MoveUp:
            newY -= actualDistance;
            break;

        case MoveRight:
            newX += actualDistance;
            break;

        case MoveDown:
            newY += actualDistance;
            break;

        case MoveLeft:
            newX -= actualDistance;
            break;

        case MoveUpRight:
            newX += actualDistance;
            newY -= actualDistance;
            break;

        case MoveDownRight:
            newX += actualDistance;
            newY += actualDistance;
            break;

        case MoveDownLeft:
            newX -= actualDistance;
            newY += actualDistance;
            break;


        case MoveUpLeft:
            newX -= actualDistance;
            newY -= actualDistance;
            break;

        default:
            kDebug()<<"Compound direction is not supported";
            return;
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

