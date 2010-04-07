/*
    Copyright (C) 2009 Igor Trindade Oliveira <igor.oliveira@indt.org.br>
    Copyright (C) 2009 Adenilson Cavalcanti <adenilson.silva@idnt.org.br>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "rotation_p.h"

#include <QGraphicsRotation>
#include <QEasingCurve>

#include <kdebug.h>

namespace Plasma
{

const int RotationAnimation::s_bounceSteps = 10;

RotationAnimation::RotationAnimation(QObject *parent, bool isBounceable, qint8 reference, Qt::Axis axis, qreal angle)
    : Animation(parent),
      m_bounceStep(0)
{
    setBounceable(isBounceable);
    setAngle(angle);
    setAxis(axis);
    setReference(reference);

    m_rotation = new QGraphicsRotation(this);
}

RotationAnimation::~RotationAnimation()
{
}

bool RotationAnimation::isBounceable() const
{
    return m_isBounceable;
}

void RotationAnimation::setBounceable(const bool isBounceable)
{
    m_isBounceable = isBounceable;
}

Qt::Axis RotationAnimation::axis() const
{
    return m_axis;
}

void RotationAnimation::setAxis(const Qt::Axis &axis)
{
    m_axis = axis;
}

qint8 RotationAnimation::reference() const
{
    return m_reference;
}

void RotationAnimation::setReference(const qint8 &reference)
{
    m_reference = reference;
}

qreal RotationAnimation::angle() const
{
    return m_angle;
}

void RotationAnimation::setAngle(const qreal &angle)
{
    m_angle = angle;
}

void RotationAnimation::updateState(QAbstractAnimation::State newState, QAbstractAnimation::State oldState)
{
    QGraphicsWidget *m_object = targetWidget();

    if (!m_object) {
        return;
    }

    QVector3D vector(0, 0, 0);

    const qreal widgetWidth = m_object->size().width();
    const qreal widgetHeight = m_object->size().height();

    if (axis() == Qt::XAxis) {
        switch (reference()) {
            case Center:
                vector.setX(widgetWidth/2);
                vector.setY(widgetHeight/2);
                break;
            case Up:
                vector.setX(widgetWidth/2);
                vector.setY(0);
                break;
            case Down:
                vector.setX(widgetWidth/2);
                vector.setY(widgetHeight);
                break;
        }

    } else if(axis() == Qt::YAxis) {
        switch (reference()) {
            case Center:
                vector.setX(widgetWidth/2);
                vector.setY(widgetHeight/2);
                break;
            case Left:
                vector.setX(0);
                vector.setY(widgetHeight/2);
                break;
            case Right:
                vector.setX(widgetWidth);
                vector.setY(widgetHeight/2);
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

    m_rotation->setOrigin(vector);
    m_rotation->setAxis(axis());

    QList<QGraphicsTransform *> transformation;
    transformation.append(m_rotation);
    m_object->setTransformations(transformation);

    if ((oldState == Stopped) && (newState == Running)) {
        m_rotation->setAngle(direction() == Forward ? 0 : angle());
    } else if (newState == Stopped) {
        // Do not bounce if animation is part of a group
        if (!m_isBounceable || group()) {
            m_rotation->setAngle(direction() == Forward ? angle() : 0);
        } else {
            setDirection(direction() == Forward ? Backward : Forward);
            if (++m_bounceStep < s_bounceSteps) {
                start();
            } else {
                m_rotation->setAngle(direction() == Forward ? 0 : angle());
                m_bounceStep = 0;
            }
        }
    }
}

void RotationAnimation::updateCurrentTime(int currentTime)
{
    QGraphicsWidget *w = targetWidget();
    if (w) {
        qreal delta = Animation::easingCurve().valueForProgress(
            currentTime / qreal(duration()));
        const qreal currentAngle = angle();
        if (m_isBounceable) {
            const qreal percentage = 1 - qBound(0.0, qreal(m_bounceStep) / qreal(s_bounceSteps), 1.0);
            if (m_bounceStep == 0) { // initial bounce
                delta = qBound(0.0, currentAngle * percentage * delta * 0.5, (currentAngle / 2.0));
            } else {
                delta -= 0.5;
                delta = qBound(-(currentAngle / 2.0), currentAngle * percentage * delta, (currentAngle / 2.0));
            }
        } else {
            delta = qBound(0.0, currentAngle * delta, currentAngle);
        }
        m_rotation->setAngle(delta);
    }
}

}

#include <../rotation_p.moc>
