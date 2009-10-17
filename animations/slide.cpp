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

#include "slide.h"

#include <QPointF>

#include <kdebug.h>


namespace Plasma
{

SlideAnimation::SlideAnimation(AnimationDirection direction, qreal distance)
    : m_direction(direction),
      m_distance(distance),
      m_end_visibility(true)
{

}

QAbstractAnimation* SlideAnimation::render(QObject* parent){

    //get current geometry values
    QGraphicsWidget *m_object = getAnimatedObject();
    qreal x = m_object->x();
    qreal y = m_object->y();

    qreal newX = x;
    qreal newY = y;

    //compute new geometry values
    switch (m_direction){

        case MoveUp:
            newY = y - m_distance;
            break;

        case MoveRight:
            newX = x + m_distance;
            break;

        case MoveDown:
            newY = y + m_distance;
            break;

        case MoveLeft:
            newX = x - m_distance;
            break;

        case MoveUpRight:
        case MoveDownRight:
        case MoveDownLeft:
        case MoveUpLeft:
	    /* TODO: support compound directions */
            kDebug() << "Compound directions (UpRight, DownRight, DownLeft, \
UpLeft) are not supported\n";
            break;
    }

    //create animation
    QPropertyAnimation* anim = new QPropertyAnimation(m_object, "pos", parent);
    anim->setEndValue(QPointF(newX, newY));
    anim->setDuration(getDuration());

    //QObject::connect(anim, SIGNAL(finished()), anim, SLOT(deleteLater()));

    if(m_end_visibility){
        QObject::connect(anim, SIGNAL(finished()), m_object, SLOT(show()));
    } else {
        QObject::connect(anim, SIGNAL(finished()), m_object, SLOT(hide()));
    }

    return anim;

}

void SlideAnimation::setVisibleAtEnd(bool visibility){
    m_end_visibility = visibility;
}

} //namespace Plasma

