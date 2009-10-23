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

#include "grow_p.h"

#include <QRect>
#include <kdebug.h>

namespace Plasma
{

GrowAnimation::GrowAnimation(qreal factor)
    : m_animFactor(factor)
{
}

void GrowAnimation::setFactor(qreal factor)
{
    m_animFactor = qBound(qreal(0.0), factor, qreal(1.0));
}

qreal GrowAnimation::factor() const
{
    return m_animFactor;
}

QAbstractAnimation* GrowAnimation::render(QObject* parent){

    //get current geometry values
    QGraphicsWidget *m_object = widgetToAnimate();
    QRectF geometry = m_object->geometry();
    qreal w = geometry.width();
    qreal h = geometry.height();

    //compute new geometry values
    qreal factor = m_animFactor;
    qreal newWidth = w * factor;
    qreal newHeight = h * factor;
    //locfactor is factor by which object must move up and left
    //to compensate for its growth down and right, to keep the
    //center in place.
    qreal locfactor = (factor - 1) / 2.0;
    qreal newX = geometry.x() - w * locfactor;
    qreal newY = geometry.y() - h * locfactor;

    //Recreate only if needed
    QPropertyAnimation *anim = dynamic_cast<QPropertyAnimation* >(animation());
    if (!anim) {
	anim = new QPropertyAnimation(m_object, "geometry", parent);
	setAnimation(anim);
    }

    anim->setEndValue(QRectF(
                newX, newY,
                newWidth, newHeight));
    anim->setDuration(duration());

    //QObject::connect(anim, SIGNAL(finished()), anim, SLOT(deleteLater()));

    return anim;
}

} //namespace Plasma

