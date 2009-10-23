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
#include "rotation_p.h"

#include <QGraphicsRotation>

#include <kdebug.h>

namespace Plasma
{

class RotationAnimationPrivate {
public:

    /* TODO: check if the rotation object will be deleted
     * when the animation runs
     */
    QGraphicsRotation *rotation;

    /**
     * Animation rotation angle (e.g. 45, 180, etc)
     */
    qreal angle;

    /**
     * Rotation axis (e.g. X, Y, Z)
     */
    Qt::Axis axis;

    /**
     * Rotation reference (e.g. Center, Up, Down, Left, Right) can
     * be combined (i.e. Center|Up)
     */
    qint8 reference;


};


RotationAnimation::RotationAnimation(const qint8 &reference, const Qt::Axis &axis, const qreal &angle)
    : d(new RotationAnimationPrivate)
{
    setAngle(angle);
    setAxis(axis);
    setReference(reference);

    d->rotation = new QGraphicsRotation(this);
}

RotationAnimation::~RotationAnimation()
{
    delete d;
}

QPropertyAnimation *RotationAnimation::render(QObject *parent)
{
    Q_UNUSED(parent);
    QGraphicsWidget *m_object = widgetToAnimate();

    QVector3D vector(0, 0, 0);

    const qreal widgetWidth = m_object->size().width();
    const qreal widgetHeight = m_object->size().height();

    if (axis() == Qt::XAxis) {
        switch (reference()) {
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

    } else if(axis() == Qt::YAxis) {
        switch (reference()) {
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

    } else if (axis() == Qt::ZAxis) {
        switch (reference()) {
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
    d->rotation->setAxis(axis());

    QList<QGraphicsTransform *> transformation;
    transformation.append(d->rotation);
    m_object->setTransformations(transformation);

    QPropertyAnimation *rotationAnimation = dynamic_cast<QPropertyAnimation* >(animation());
    if (!rotationAnimation) {
	rotationAnimation = new QPropertyAnimation(d->rotation, "angle", m_object);
	setAnimation(rotationAnimation);
    }

    rotationAnimation->setStartValue(0);
    rotationAnimation->setEndValue(angle());
    rotationAnimation->setDuration(duration());

    return rotationAnimation;
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

}
