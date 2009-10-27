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

#include "expand_p.h"

#include <QRect>
#include <kdebug.h>

namespace Plasma
{

ExpandAnimation::ExpandAnimation(QObject *parent)
    : Animation(parent)
{
}

QAbstractAnimation* ExpandAnimation::render(QObject* parent)
{

    //get current geometry values
    QGraphicsWidget *m_object = widgetToAnimate();
    QRectF geometry = m_object->geometry();

    //compute new geometry values
    switch (direction()) {

    case MoveUp:
        geometry.setTop(geometry.y() - distance());
        break;

    case MoveRight:
        geometry.setRight(geometry.x() + geometry.width() - 1 + distance());
        break;

    case MoveDown:
        geometry.setBottom(geometry.y() + geometry.height() - 1  + distance());
        break;

    case MoveLeft:
        geometry.setLeft(geometry.x() - distance());
        break;

    case MoveUpRight:
    case MoveDownRight:
    case MoveDownLeft:
    case MoveUpLeft:
        break;
    }

    //Recreate only if needed
    QPropertyAnimation *anim = dynamic_cast<QPropertyAnimation* >(animation());
    if (!anim) {
        anim = new QPropertyAnimation(m_object, "geometry", parent);
        setAnimation(anim);
    }
    anim->setEndValue(geometry);
    anim->setDuration(duration());

    return anim;
}

} //namespace Plasma

