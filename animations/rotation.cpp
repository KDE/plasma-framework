////////////////////////////////////////////////////////////////////////
// rotation.cpp                                                        //
//                                                                     //
// Copyright(C) 2009 Igor Trindade Oliveira <igor.oliveira@indt.org.br>//
// Copyright(C) 2009 Adenilson Cavalcanti <adenilson.silva@idnt.org.br>//
//                                                                     //
// This library is free software; you can redistribute it and/or       //
// modify it under the terms of the GNU Lesser General Public          //
// License as published by the Free Software Foundation; either        //
// version 2.1 of the License, or (at your option) any later version.  //
//                                                                     //
// This library is distributed in the hope that it will be useful,     //
// but WITHOUT ANY WARRANTY; without even the implied warranty of      //
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU   //
// Lesser General Public License for more details.                     //
//                                                                     //
// You should have received a copy of the GNU Lesser General Public    //
// License along with this library; if not, write to the Free Software //
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA       //
// 02110-1301  USA                                                     //
/////////////////////////////////////////////////////////////////////////
#include "rotation.h"

#include <QGraphicsRotation>

namespace Plasma
{

class RotationAnimationPrivate {
    public:
        RotationAnimationPrivate()
        {
        }

        ~RotationAnimationPrivate()
        {
        }

        QGraphicsRotation *rotation;
        qreal angle;
        Qt::Axis axis;
        qint8 reference;
};


RotationAnimation::RotationAnimation(const qint8 &reference, const Qt::Axis &axis, const qreal &angle)
    : d(new RotationAnimationPrivate)
{
    d->angle = angle;
    d->axis = axis;
    d->reference = reference;

    d->rotation = new QGraphicsRotation(this);
}

RotationAnimation::~RotationAnimation()
{
    delete d;
}

Qt::Axis RotationAnimation::axis() const
{
    return d->axis;
}

void RotationAnimation::setAxis(const Qt::Axis &axis)
{
    d->axis = axis;
}

qint8 RotationAnimation::reference() const
{
    return d->reference;
}

void RotationAnimation::setReference(const qint8 &reference)
{
    d->reference = reference;
}

qreal RotationAnimation::angle() const
{
    return d->angle;
}

void RotationAnimation::setAngle(const qreal &angle)
{
    d->angle = angle;
}

QPropertyAnimation *RotationAnimation::render(QObject *parent)
{
    Q_UNUSED(parent);
    QGraphicsWidget *m_object = getAnimatedObject();

    QVector3D vector(0, 0, 0);

    const qreal widgetWidth = m_object->size().width();
    const qreal widgetHeight = m_object->size().height();

    if (d->axis == Qt::XAxis) {
        switch(d->reference) {
            case Center:
                vector.setY(widgetHeight/2);
                break;
            case Up:
                vector.setY(0);
                break;
            case Down:
                vector.setY(widgetHeight);
                break;
        }
    } else if(d->axis == Qt::YAxis) {
        switch(d->reference) {
            case Center:
                vector.setX(widgetWidth/2);
                break;
            case Left:
                vector.setX(0);
                break;
            case Right:
                vector.setX(widgetWidth);
                break;
        }
    }else if (d->axis == Qt::ZAxis) {
        switch(d->reference) {
            case Center:
                vector.setX(widgetWidth/2);
                vector.setY(widgetHeight/2);
                break;

            case Center|Up:
                vector.setX(widgetWidth/2);
                vector.setY(0);
                break;

            case Center|Down:
                vector.setX(widgetWidth/2);
                vector.setY(widgetHeight);
                break;

            case Center|Left:
                vector.setX(0);
                vector.setY(widgetHeight/2);
                break;

            case Center|Right:
                vector.setX(widgetWidth);
                vector.setY(widgetHeight/2);
                break;
        }
    }

    d->rotation->setOrigin(vector);
    d->rotation->setAxis(d->axis);

    QList<QGraphicsTransform *> transformation;
    transformation.append(d->rotation);
    m_object->setTransformations(transformation);

    QPropertyAnimation *rotationAnimation= new QPropertyAnimation(d->rotation,
            "angle", m_object);
    rotationAnimation->setEndValue(d->angle);
    rotationAnimation->setDuration(getDuration());

    return rotationAnimation;
}

}
