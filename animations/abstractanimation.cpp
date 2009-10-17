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

#include "abstractanimation.h"

namespace Plasma
{

class AnimationPrivate
{
public:
    /**
     * Object the animation(s) should act upon.
     */
    QGraphicsWidget* m_object;
};

AbstractAnimation::AbstractAnimation(): d(new AnimationPrivate)
{

}

AbstractAnimation::~AbstractAnimation()
{
    delete d;
}

void AbstractAnimation::setWidget(QGraphicsWidget* receiver)
{
    d->m_object = receiver;
}

QGraphicsWidget* AbstractAnimation::getAnimatedObject()
{
    return d->m_object;
}

} //namespace Plasma

#include <../abstractanimation.moc>
