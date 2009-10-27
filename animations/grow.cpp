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

void GrowAnimation::setFactor(const qreal factor)
{
    m_animFactor = qMax(qreal(0.0), factor);
}

qreal GrowAnimation::factor() const
{
    return m_animFactor;
}

QAbstractAnimation* GrowAnimation::render(QObject* parent){

    //get current geometry values
    QGraphicsWidget *m_object = widgetToAnimate();
    QRectF geometry = m_object->geometry();
    QSizeF minimum = m_object->effectiveSizeHint(Qt::MinimumSize);
    QSizeF maximum = m_object->effectiveSizeHint(Qt::MaximumSize);
    qreal w = geometry.width();
    qreal h = geometry.height();

    //compute new geometry values
    qreal factor = m_animFactor;
    qreal newWidth = qBound(minimum.width(), w * factor, maximum.width());
    qreal newHeight = qBound(minimum.width(), h * factor, maximum.width());

    qreal newX = geometry.x() - (newWidth - w)/2;
    qreal newY = geometry.y() - (newHeight - h)/2;

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

