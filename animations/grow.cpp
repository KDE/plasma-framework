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

GrowAnimation::GrowAnimation(QObject *parent, qreal factor)
    : Animation(parent), m_animFactor(factor)
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

void GrowAnimation::setWidgetToAnimate(QGraphicsWidget *widget)
{
    Animation::setWidgetToAnimate(widget);
    if (animation.data()) {
        delete animation.data();
        animation.clear();
    }
}

QAbstractAnimation* GrowAnimation::render(QObject* parent){

    //get current geometry values
    QGraphicsWidget *m_object = widgetToAnimate();
    QRectF geometry = m_object->geometry();
    QSizeF minimum = m_object->effectiveSizeHint(Qt::MinimumSize);
    QSizeF maximum = m_object->effectiveSizeHint(Qt::MaximumSize);
    qreal w = geometry.width();
    qreal h = geometry.height();
    qreal factor = m_animFactor;

    //compute new geometry values
    qreal newWidth;
    qreal newHeight;
    if (direction() == QAbstractAnimation::Forward) {
        newWidth = qBound(minimum.width(), w * factor, maximum.width());
        newHeight = qBound(minimum.height(), h * factor, maximum.height());
    } else {
        newWidth = qBound(minimum.width(), w / factor, maximum.width());
        newHeight = qBound(minimum.height(), h / factor, maximum.height());
    }

    qreal newX;
    qreal newY;
    newX = geometry.x() - (newWidth - w)/2;
    newY = geometry.y() - (newHeight - h)/2;

    //Recreate only if needed
    QPropertyAnimation *anim = animation.data();
    if (!anim) {
        anim = new QPropertyAnimation(m_object, "geometry", m_object);
        animation = anim;
    }

    //Prevents from deforming (with multiple clicks)
    QAbstractAnimation::State temp = anim->state();
    if (temp == QAbstractAnimation::Running) {
        return anim;
    }

    if (direction() == QAbstractAnimation::Forward) {
        anim->setStartValue(QRectF(geometry.x(), geometry.y(), w, h));
        anim->setEndValue(QRectF(newX, newY, newWidth, newHeight));
    } else {
        anim->setStartValue(QRectF(newX, newY, newWidth, newHeight));
        anim->setEndValue(QRectF(geometry.x(), geometry.y(), w, h));
    }

    qDebug()<<"start value:"<<anim->startValue()<<"end value:"<<anim->endValue();

    anim->setDuration(duration());

    return anim;
}

} //namespace Plasma

