/***********************************************************************/
/* rotationstacked.h                                                     */
/*                                                                     */
/* Copyright(C) 2009 Igor Trindade Oliveira <igor.oliveira@indt.org.br>*/
/*                                                                     */
/* This library is free software; you can redistribute it and/or       */
/* modify it under the terms of the GNU Lesser General Public	       */
/* License as published by the Free Software Foundation; either	       */
/* version 2.1 of the License, or (at your option) any later version.  */
/*                                                                     */
/* This library is distributed in the hope that it will be useful,     */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of      */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU   */
/* Lesser General Public License for more details.                     */
/*                                                                     */
/* You should have received a copy of the GNU Lesser General Public    */
/* License along with this library; if not, write to the Free Software */
/* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA       */
/* 02110-1301  USA                                                     */
/***********************************************************************/

#include "rotationstacked.h"

#include <QGraphicsRotation>
#include <QSequentialAnimationGroup>
#include <QWeakPointer>

#include <kdebug.h>

#include "stackedlayout.h"
#include "plasma.h"

namespace Plasma
{

class RotationStackedAnimationPrivate {
public:
    QGraphicsRotation *backRotation;
    QGraphicsRotation *frontRotation;
    int frontStartAngle, frontEndAngle;
    int backStartAngle, backEndAngle;

    qint8 reference;
    /**
     * Animation direction: where the animation will move.
     */
    Plasma::AnimationDirection animDirection;

    QWeakPointer<QGraphicsWidget> backWidget;
    StackedLayout *sLayout;
};

RotationStackedAnimation::RotationStackedAnimation(QObject *parent)
    : Animation(parent),
      d(new RotationStackedAnimationPrivate)
{
    d->backRotation = new QGraphicsRotation(this);
    d->frontRotation = new QGraphicsRotation(this);

    d->sLayout = new StackedLayout;
}

void RotationStackedAnimation::setMovementDirection(const qint8 &direction)
{
    d->animDirection = static_cast<Plasma::AnimationDirection>(direction);
}

qint8 RotationStackedAnimation::movementDirection() const
{
    return static_cast<qint8>(d->animDirection);
}

RotationStackedAnimation::~RotationStackedAnimation()
{
    delete d;
}

void RotationStackedAnimation::setWidgetToAnimate(QGraphicsWidget *widget)
{
    Animation::setWidgetToAnimate(widget);
}

void RotationStackedAnimation::setReference(const qint8 &reference)
{
    d->reference = reference;
}

qint8 RotationStackedAnimation::reference() const
{
    return d->reference;
}

QGraphicsWidget *RotationStackedAnimation::backWidget()
{
    return d->backWidget.data();
}

void RotationStackedAnimation::setBackWidget(QGraphicsWidget *backWidget)
{
    d->backWidget = backWidget;

    if(widgetToAnimate()) {
        d->sLayout->addWidget(widgetToAnimate());
        d->sLayout->addWidget(d->backWidget.data());
    }
    //render(parent());
}

QGraphicsLayoutItem *RotationStackedAnimation::layout()
{
    return d->sLayout;
}

void RotationStackedAnimation::updateState(
        QAbstractAnimation::State newState, QAbstractAnimation::State oldState)
{
    if (!backWidget()) {
        return;
    }

    QPair<QGraphicsWidget *,QGraphicsWidget *> widgets = qMakePair(widgetToAnimate(), backWidget());

    const qreal widgetFrontWidth = widgets.first->size().width();
    const qreal widgetFrontHeight = widgets.first->size().height();

    const qreal widgetBackWidth = widgets.second->size().width();
    const qreal widgetBackHeight = widgets.second->size().height();

    QPair<QVector3D, QVector3D> vector;

    if (reference() == Center) {

        vector.first = QVector3D(widgetFrontWidth/2, widgetFrontHeight/2, 0);
        vector.second = QVector3D(widgetBackWidth/2, widgetBackHeight/2, 0);

        if (d->animDirection == MoveLeft || d->animDirection == MoveRight) {
            d->frontRotation->setAxis(Qt::YAxis);
            d->backRotation->setAxis(Qt::YAxis);

            if (d->animDirection == MoveLeft) {
                /* TODO: the order way */

            } else {
                d->frontStartAngle = 0;
                d->frontEndAngle = 90;
                d->backStartAngle = 265; //hack
                d->backEndAngle = 360;
            }
        }
    }

    d->frontRotation->setOrigin(vector.first);
    d->backRotation->setOrigin(vector.second);

    QList<QGraphicsTransform *> backTransformation;
    QList<QGraphicsTransform *> frontTransformation;

    frontTransformation.append(d->frontRotation);
    backTransformation.append(d->backRotation);

    widgets.first->setTransformations(frontTransformation);
    widgets.second->setTransformations(backTransformation);

    if (oldState == Stopped && newState == Running) {
        d->frontRotation->setAngle(direction() == Forward ? d->frontStartAngle : d->frontEndAngle);
        d->backRotation->setAngle(direction() == Forward ? d->backStartAngle : d->backEndAngle);
    } else if(newState == Stopped) {
        d->frontRotation->setAngle(direction() == Forward ? d->frontEndAngle : d->frontStartAngle);
        d->backRotation->setAngle(direction() == Forward ? d->backEndAngle : d->backStartAngle);
    }
}

void RotationStackedAnimation::updateCurrentTime(int currentTime)
{
    QGraphicsWidget *w = widgetToAnimate();
    if(w) {
        qreal delta;
        if (currentTime <= duration()/2) {
            delta = (currentTime*2)/qreal(duration());
            d->sLayout->setCurrentWidgetIndex(0);
            delta = d->frontEndAngle * delta;
            d->frontRotation->setAngle(delta);
        } else {
            delta = (currentTime/2) / qreal(duration());
            d->sLayout->setCurrentWidgetIndex(1);
            delta = d->backEndAngle * delta;
            d->backRotation->setAngle(delta);
        }
    }
}
}
