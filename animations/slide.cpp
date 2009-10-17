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
#include "private/animationprivate_p.h"

#include <QPointF>
#include <kdebug.h>

namespace Plasma
{

SlideAnimation::SlideAnimation(AnimationDirection direction, qreal distance)
{
    AnimationPrivate *obj = getAnimationPrivate();
    obj->animDirection = direction;
    //: m_direction(direction),
    obj->animDistance = distance;
    //m_distance(distance),
    obj->animVisible = true;
    //m_end_visibility(true)

}

QAbstractAnimation* SlideAnimation::render(QObject* parent)
{

    //get current geometry values
    QGraphicsWidget *m_object = getAnimatedObject();
    qreal x = m_object->x();
    qreal y = m_object->y();

    qreal newX = x;
    qreal newY = y;

    AnimationPrivate *obj = getAnimationPrivate();
    //compute new geometry values
    switch (obj->animDirection) {

    case MoveUp:
	newY = y - obj->animDistance;
	break;

    case MoveRight:
	newX = x + obj->animDistance;
	break;

    case MoveDown:
	newY = y + obj->animDistance;
	break;

    case MoveLeft:
	newX = x - obj->animDistance;
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

    if (obj->animVisible) {
        QObject::connect(anim, SIGNAL(finished()), m_object, SLOT(show()));
    } else {
        QObject::connect(anim, SIGNAL(finished()), m_object, SLOT(hide()));
    }

    return anim;

}

void SlideAnimation::setVisibleAtEnd(bool visibility)
{
    getAnimationPrivate()->animVisible = visibility;
}

} //namespace Plasma

