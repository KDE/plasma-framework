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

const int RotationStackedAnimation::s_sideAngle = 90;

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

    QVector3D animDirection(0, 0, 0);

    switch (m_animDirection) {
    case MoveLeft:
        animDirection.setY(-1);
        break;

    case MoveRight:
        animDirection.setY(1);
        break;

    case MoveAny:
    case MoveUp:
        animDirection.setX(1);
        break;

    case MoveDown:
        animDirection.setX(-1);
        break;

    case MoveUpLeft:
        animDirection.setX(1);
        animDirection.setY(-1);
        break;

    case MoveUpRight:
        animDirection.setX(1);
        animDirection.setY(1);
        break;

    case MoveDownLeft:
        animDirection.setX(-1);
        animDirection.setY(-1);
        break;

    case MoveDownRight:
        animDirection.setX(-1);
        animDirection.setY(1);
        break;
    }

    m_frontRotation->setAxis(animDirection);
    m_backRotation->setAxis(animDirection);

    updateTransformations();
}

qint8 RotationStackedAnimation::movementDirection() const
{
    return static_cast<qint8>(m_animDirection);
}

void RotationStackedAnimation::setReference(const qint8 &reference)
{
    m_reference = reference;

    if (!targetWidget() || !backWidget()) {
        return;
    }

    const QSizeF transformArea = targetWidget()->size().expandedTo(backWidget()->size());

    QVector3D frontTransformOrigin(transformArea.width()/2, transformArea.height()/2, 0);
    QVector3D backTransformOrigin(transformArea.width()/2, transformArea.height()/2, 0);

    if ((m_reference & Left) == Left) {
        frontTransformOrigin.setX(0);
        backTransformOrigin.setX(0);
    } else if ((m_reference & Right) == Right) {
        frontTransformOrigin.setX(transformArea.width());
        backTransformOrigin.setX(transformArea.width());
    }

    if ((m_reference & Up) == Up) {
        frontTransformOrigin.setY(0);
        backTransformOrigin.setY(0);
    } else if ((m_reference & Down) == Down) {
        frontTransformOrigin.setY(transformArea.height());
        backTransformOrigin.setY(transformArea.height());
    }

    m_frontRotation->setOrigin(frontTransformOrigin);
    m_backRotation->setOrigin(backTransformOrigin);

    updateTransformations();
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

void RotationStackedAnimation::updateState(QAbstractAnimation::State newState,
        QAbstractAnimation::State oldState)
{
    if (oldState == Stopped && newState == Running) {
        setReference(reference());
    }
}

void RotationStackedAnimation::updateCurrentTime(int currentTime)
{
    if (!targetWidget() || !backWidget()) {
        return;
    }

    StackedLayout *layout = m_wLayout.data();
    if (!layout) {
        return;
    }

    qreal delta;
    if (currentTime <= duration()/2) {
        layout->setCurrentWidgetIndex(0);
        delta = easingCurve().valueForProgress((currentTime*2) / qreal(duration()));
        delta *= s_sideAngle;
        m_frontRotation->setAngle(delta);
    } else {
        layout->setCurrentWidgetIndex(1);
        delta = 1 - easingCurve().valueForProgress(((currentTime*2) - duration()) / qreal(duration()));
        delta = -delta;
        delta *= s_sideAngle;
        m_backRotation->setAngle(delta);
    }
}

void RotationStackedAnimation::updateTransformations()
{
    if (!targetWidget() || !backWidget()) {
        return;
    }

    QList<QGraphicsTransform *> frontTransformation;
    QList<QGraphicsTransform *> backTransformation;

    frontTransformation.append(m_frontRotation);
    backTransformation.append(m_backRotation);

    targetWidget()->setTransformations(frontTransformation);
    backWidget()->setTransformations(backTransformation);
}
}
