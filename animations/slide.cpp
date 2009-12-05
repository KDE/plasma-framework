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
        qreal delta = currentTime / qreal(duration());
        w->setPos(m_startPos * (1-delta) + (m_target * delta));
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

        kDebug()<<movementDirection();
        switch (movementDirection()) {
        case MoveUp:
            newY -= distance();
            break;

        case MoveRight:
            newX += distance();
            break;

        case MoveDown:
            newY += distance();
            break;

        case MoveLeft:
            newX -= distance();
            break;

        case MoveUpRight:
            newX += distance();
            newY -= distance();
            break;

        case MoveDownRight:
            newX += distance();
            newY += distance();
            break;

        case MoveDownLeft:
            newX -= distance();
            newY += distance();
            break;


        case MoveUpLeft:
            newX -= distance();
            newY -= distance();
            break;

        default:
            kDebug()<<"Compound direction is not supported";
            return;
        }

        m_target = QPointF(newX, newY);
    }
}

} //namespace Plasma

