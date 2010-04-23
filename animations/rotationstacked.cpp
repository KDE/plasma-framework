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
    : EasingAnimation(parent)
{
    m_backRotation = new QGraphicsRotation(this);
    m_frontRotation = new QGraphicsRotation(this);
    m_wLayout = new StackedLayout;
}

RotationStackedAnimation::~RotationStackedAnimation()
{
    delete m_wLayout.data();
}

void RotationStackedAnimation::setMovementDirection(const Animation::MovementDirection &direction)
{
    m_animDirection = direction;

    QVector3D animDirection(0, 0, 0);

    if (m_animDirection.testFlag(MoveUp)) {
        animDirection.setX(1);
    } else if (m_animDirection.testFlag(MoveDown)) {
        animDirection.setX(-1);
    }

    if (m_animDirection.testFlag(MoveLeft)) {
        animDirection.setY(-1);
    } else if (m_animDirection.testFlag(MoveRight)) {
        animDirection.setY(1);
    }

    m_frontRotation->setAxis(animDirection);
    m_backRotation->setAxis(animDirection);

    updateTransformations();
}

Animation::MovementDirection RotationStackedAnimation::movementDirection() const
{
    return m_animDirection;
}

void RotationStackedAnimation::setReference(const Animation::Reference &reference)
{
    m_animReference = reference;

    if (!targetWidget() || !backWidget()) {
        return;
    }

    const QSizeF transformArea = targetWidget()->size().expandedTo(backWidget()->size());

    QVector3D frontTransformOrigin(transformArea.width()/2, transformArea.height()/2, 0);
    QVector3D backTransformOrigin(transformArea.width()/2, transformArea.height()/2, 0);

    if (m_animReference.testFlag(Up)) {
        frontTransformOrigin.setY(0);
        backTransformOrigin.setY(0);
    } else if (m_animReference.testFlag(Down)) {
        frontTransformOrigin.setY(transformArea.height());
        backTransformOrigin.setY(transformArea.height());
    }

    if (m_animReference.testFlag(Left)) {
        frontTransformOrigin.setX(0);
        backTransformOrigin.setX(0);
    } else if (m_animReference.testFlag(Right)) {
        frontTransformOrigin.setX(transformArea.width());
        backTransformOrigin.setX(transformArea.width());
    }

    m_frontRotation->setOrigin(frontTransformOrigin);
    m_backRotation->setOrigin(backTransformOrigin);

    updateTransformations();
}

Animation::Reference RotationStackedAnimation::reference() const
{
    return m_animReference;
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

void RotationStackedAnimation::updateEffectiveTime(int currentTime)
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
        delta = (currentTime*2) / qreal(duration());
        delta *= s_sideAngle;
        m_frontRotation->setAngle(delta);
    } else {
        layout->setCurrentWidgetIndex(1);
        delta = 1 - (((currentTime*2) - duration()) / qreal(duration()));
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

} //namespace Plasma

#include <../rotationstacked_p.moc>
