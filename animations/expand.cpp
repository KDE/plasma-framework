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

#include "expand.h"
#include "private/animationprivate_p.h"

#include <QRect>
#include <kdebug.h>

namespace Plasma
{

ExpandAnimation::ExpandAnimation(AnimationDirection direction, qreal distance)
{
    AnimationPrivate *obj = getAnimationPrivate();
    obj->animDirection = direction;
    obj->animDistance = distance;
}

QAbstractAnimation* ExpandAnimation::render(QObject* parent){

    //get current geometry values
    QGraphicsWidget *m_object = getAnimatedObject();
    QRectF geometry = m_object->geometry();
    AnimationPrivate *obj = getAnimationPrivate();

    //compute new geometry values
    switch (obj->animDirection) {

    case MoveUp:
	geometry.setTop(geometry.y() - obj->animDistance);
	break;

    case MoveRight:
	geometry.setRight(geometry.x() + geometry.width() - 1 + obj->animDistance);
	break;

    case MoveDown:
	geometry.setBottom(geometry.y() + geometry.height() - 1  + obj->animDistance);
	break;

    case MoveLeft:
	geometry.setLeft(geometry.x() - obj->animDistance);
	break;

    case MoveUpRight:
    case MoveDownRight:
    case MoveDownLeft:
    case MoveUpLeft:
	break;
    }

    //create animation
    QPropertyAnimation* anim = new QPropertyAnimation(m_object, "geometry", parent);
    anim->setEndValue(geometry);
    anim->setDuration(getDuration());

    //QObject::connect(anim, SIGNAL(finished()), anim, SLOT(deleteLater()));

    return anim;

}

} //namespace Plasma

