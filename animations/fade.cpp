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

#include "fade.h"
#include "private/animationprivate_p.h"

#include <QRect>
#include <kdebug.h>

namespace Plasma
{

FadeAnimation::FadeAnimation(qreal factor)
{
    AnimationPrivate *obj = getAnimationPrivate();
    obj->animFactor = factor;
    if (obj->animFactor > 1.0) {
        kDebug() << "Opacity must be between 0 and 1.0. Reducing to 1.0.";
        obj->animFactor = 1.0;
    } else if (obj->animFactor < 0) {
        kDebug() << "Opacity must be between 0 and 1.0. Increasing to 0.";
        obj->animFactor = 0;
    }
}

QAbstractAnimation* FadeAnimation::render(QObject* parent){

    //create animation
    QGraphicsWidget *m_object = getAnimatedObject();
    QPropertyAnimation* anim = new QPropertyAnimation(m_object, "opacity", parent);
    anim->setEndValue(getAnimationPrivate()->animFactor);
    anim->setDuration(getDuration());

    //QObject::connect(anim, SIGNAL(finished()), anim, SLOT(deleteLater()));

    return anim;
}

} //namespace Plasma

