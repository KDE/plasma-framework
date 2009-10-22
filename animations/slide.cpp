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

SlideAnimation::SlideAnimation(AnimationDirection direction, qreal distance)
{
    setDirection(direction);
    setDistance(distance);
    setVisible(true);
}

QAbstractAnimation* SlideAnimation::render(QObject* parent)
{
    QGraphicsWidget *m_object = widgetToAnimate();
    qreal x = m_object->x();
    qreal y = m_object->y();

    qreal newX = x;
    qreal newY = y;

    switch (direction()) {
    case MoveUp:
        newY = y - distance();
        break;

    case MoveRight:
        newX = x + distance();
        break;

    case MoveDown:
        newY = y + distance();
        break;

    case MoveLeft:
        newX = x - distance();
        break;

    case MoveUpRight:
    case MoveDownRight:
    case MoveDownLeft:
    case MoveUpLeft:
        /* TODO: support compound directions */
        kDebug() << "Compound directions (UpRight, DownRight, DownLeft, UpLeft) are not supported";
        break;
    }

    QPropertyAnimation* anim = new QPropertyAnimation(m_object, "pos", parent);
    anim->setEndValue(QPointF(newX, newY));
    anim->setDuration(duration());

    //QObject::connect(anim, SIGNAL(finished()), anim, SLOT(deleteLater()));

    if (isVisible()) {
        QObject::connect(anim, SIGNAL(finished()), m_object, SLOT(show()));
    } else {
        QObject::connect(anim, SIGNAL(finished()), m_object, SLOT(hide()));
    }

    return anim;

}

void SlideAnimation::setVisibleAtEnd(bool visibility)
{
    setVisible(visibility);
}

} //namespace Plasma

