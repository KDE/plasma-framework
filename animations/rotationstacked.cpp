/*
    Copyright (C) 2009 Igor Trindade Oliveira <igor.oliveira@indt.org.br>

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

#include "rotationstacked_p.h"
#include <QGraphicsRotation>
#include <QSequentialAnimationGroup>
#include <QWeakPointer>
#include <kdebug.h>

#include "stackedlayout.h"
#include "plasma.h"

namespace Plasma
{

RotationStackedAnimation::RotationStackedAnimation(QObject *parent)
    : Animation(parent)
{
    m_backRotation = new QGraphicsRotation(this);
    m_frontRotation = new QGraphicsRotation(this);
    m_wLayout = new StackedLayout;
}

RotationStackedAnimation::~RotationStackedAnimation()
{
    delete m_wLayout.data();
}

void RotationStackedAnimation::setMovementDirection(const qint8 &direction)
{
    m_animDirection = static_cast<MovementDirection>(direction);
}

qint8 RotationStackedAnimation::movementDirection() const
{
    return static_cast<qint8>(m_animDirection);
}

void RotationStackedAnimation::setReference(const qint8 &reference)
{
    m_reference = reference;
}

qint8 RotationStackedAnimation::reference() const
{
    return m_reference;
}

QGraphicsWidget *RotationStackedAnimation::backWidget()
{
    return m_backWidget.data();
}

void RotationStackedAnimation::setBackWidget(QGraphicsWidget *backWidget)
{
    m_backWidget = backWidget;

    StackedLayout *layout = m_wLayout.data();

    if(targetWidget() && backWidget && layout) {
        layout->addWidget(targetWidget());
        layout->addWidget(backWidget);
    }
}

QGraphicsLayoutItem *RotationStackedAnimation::layout()
{
    return m_wLayout.data();
}

void RotationStackedAnimation::updateState(
        QAbstractAnimation::State newState, QAbstractAnimation::State oldState)
{
    if (!backWidget()) {
        return;
    }

    QPair<QGraphicsWidget *,QGraphicsWidget *> widgets = qMakePair(targetWidget(), backWidget());

    const qreal widgetFrontWidth = widgets.first->size().width();
    const qreal widgetFrontHeight = widgets.first->size().height();

    const qreal widgetBackWidth = widgets.second->size().width();
    const qreal widgetBackHeight = widgets.second->size().height();

    QPair<QVector3D, QVector3D> vector;

    if (reference() == Center) {

        vector.first = QVector3D(widgetFrontWidth/2, widgetFrontHeight/2, 0);
        vector.second = QVector3D(widgetBackWidth/2, widgetBackHeight/2, 0);

        if (m_animDirection == MoveLeft || m_animDirection == MoveRight) {
            m_frontRotation->setAxis(Qt::YAxis);
            m_backRotation->setAxis(Qt::YAxis);

            if (m_animDirection == MoveLeft) {
                /* TODO: the order way */

            } else {
                m_frontStartAngle = 0;
                m_frontEndAngle = 90;
                m_backStartAngle = 265; //hack
                m_backEndAngle = 360;
            }
        }
    }

    m_frontRotation->setOrigin(vector.first);
    m_backRotation->setOrigin(vector.second);

    QList<QGraphicsTransform *> backTransformation;
    QList<QGraphicsTransform *> frontTransformation;

    frontTransformation.append(m_frontRotation);
    backTransformation.append(m_backRotation);

    widgets.first->setTransformations(frontTransformation);
    widgets.second->setTransformations(backTransformation);

    if (oldState == Stopped && newState == Running) {
        m_frontRotation->setAngle(direction() == Forward ? m_frontStartAngle : m_frontEndAngle);
        m_backRotation->setAngle(direction() == Forward ? m_backStartAngle : m_backEndAngle);
    } else if(newState == Stopped) {
    m_frontRotation->setAngle(direction() == Forward ? m_frontEndAngle : m_frontStartAngle);
    m_backRotation->setAngle(direction() == Forward ? m_backEndAngle : m_backStartAngle);
}
}

void RotationStackedAnimation::updateCurrentTime(int currentTime)
{
    StackedLayout *layout = m_wLayout.data();
    if (!layout) {
        return;
    }

    QGraphicsWidget *w = targetWidget();
    if (w) {
        qreal delta;
        if (currentTime <= duration()/2) {
            delta = Animation::easingCurve().valueForProgress(
                    (currentTime * 2) / qreal(duration()));
            layout->setCurrentWidgetIndex(0);
            delta = m_frontEndAngle * delta;
            m_frontRotation->setAngle(delta);
        } else {
            delta = Animation::easingCurve().valueForProgress(
                    (currentTime/2) / qreal(duration()));
            layout->setCurrentWidgetIndex(1);
            delta = m_backEndAngle * delta;
            m_backRotation->setAngle(delta);
        }
    }
}
}
