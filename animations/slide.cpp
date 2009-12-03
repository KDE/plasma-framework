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

void SlideAnimation::setWidgetToAnimate(QGraphicsWidget *widget)
{
    Animation::setWidgetToAnimate(widget);
    if (m_animation) {
        delete m_animation;
    }

    m_animation = new QPropertyAnimation(widget, "pos", widget);

    syncProperties();

    QObject::connect(m_animation, SIGNAL(finished()), widget, SLOT(show()));
    QObject::connect(m_animation, SIGNAL(finished()), this, SIGNAL(finished()));
}

void SlideAnimation::syncProperties()
{
    QGraphicsWidget *widget = widgetToAnimate();
    qreal x = widget->x();
    qreal y = widget->y();

    qreal newX = x;
    qreal newY = y;

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

    m_animation->setEndValue(QPointF(newX, newY));
    m_animation->setDuration(duration());
}

void SlideAnimation::setDistance(qreal distance)
{
    m_animDistance = distance;
    syncProperties();
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
          : Animation(parent),
          m_animation(0)
{
    setMovementDirection(direction);
    setDistance(distance);
}

void SlideAnimation::setMovementDirection(const qint8 &direction)
{
    m_animDirection = static_cast<Plasma::AnimationDirection>(direction);
    syncProperties();
}

qint8 SlideAnimation::movementDirection() const
{
    return static_cast<qint8>(m_animDirection);
}

void SlideAnimation::start(QAbstractAnimation::DeletionPolicy policy)
{
   /* TODO: Actually treat policy parameter */

   if (m_animation) {
       m_animation->setDirection(direction());
       m_animation->start();
   }
}

QAbstractAnimation* SlideAnimation::render(QObject* parent)
{
}

} //namespace Plasma

