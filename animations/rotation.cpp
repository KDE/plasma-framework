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

RotationAnimation::RotationAnimation(qint8 reference, const Qt::Axis &axis, 
        const qreal &angle)
    : m_angle(angle), m_axis(axis), m_reference(reference)
{
}

void RotationAnimation::setAxis(const Qt::Axis &axis)
{
    m_axis = axis;
}

void RotationAnimation::setReference(const qint8 reference)
{
    m_reference = reference;
}

void RotationAnimation::setAngle(const qreal &angle)
{
    m_angle = angle;
}

QPropertyAnimation *RotationAnimation::render(QObject *parent)
{
    Q_UNUSED(parent);
    QGraphicsWidget *m_object = getAnimatedObject();
    QGraphicsRotation *rotation = new QGraphicsRotation(m_object);

    QVector3D vector(0, 0, 0);

    const qreal widgetWidth = m_object->size().width();
    const qreal widgetHeight = m_object->size().height();

    if (m_axis == Qt::XAxis) {
        switch(m_reference) {
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
    } else if(m_axis == Qt::YAxis) {
        switch(m_reference) {
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
    }else if (m_axis == Qt::ZAxis) {
        switch(m_reference) {
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

    rotation->setOrigin(vector);
    rotation->setAxis(m_axis);

    QList<QGraphicsTransform *> transformation;
    transformation.append(rotation);
    m_object->setTransformations(transformation);

    QPropertyAnimation *rotationAnimation= new QPropertyAnimation(rotation,
            "angle", m_object);
    rotationAnimation->setEndValue(m_angle);
    rotationAnimation->setDuration(getDuration());

    return rotationAnimation;
}

}
