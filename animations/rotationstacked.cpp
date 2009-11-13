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
#include "stackedlayout.h"

#include <QGraphicsRotation>
#include <QSequentialAnimationGroup>
#include <kdebug.h>

namespace Plasma
{

class RotationStackedAnimationPrivate {
    public:
        QGraphicsRotation *backRotation;
        QGraphicsRotation *frontRotation;

        qint8 reference;

        QGraphicsWidget *backWidget;
        QSequentialAnimationGroup *groupAnim;
        StackedLayout *sLayout;
};

RotationStackedAnimation::RotationStackedAnimation(QObject *parent)
    : d(new RotationStackedAnimationPrivate)
{
    d->backRotation = new QGraphicsRotation(this);
    d->frontRotation = new QGraphicsRotation(this);

    d->sLayout = new StackedLayout;
}

RotationStackedAnimation::~RotationStackedAnimation()
{
    /* XXX: do we need to delete the layout if the 'parent' layout is
     * deleted too ?
     */
    delete d->sLayout;
    delete d;
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
    return d->backWidget;
}

void RotationStackedAnimation::setBackWidget(QGraphicsWidget *backWidget)
{
    d->backWidget = backWidget;

    if(widgetToAnimate()) {
        d->sLayout->addWidget(widgetToAnimate());
        d->sLayout->addWidget(backWidget);
    }
}

QGraphicsLayoutItem *RotationStackedAnimation::layout()
{
    return d->sLayout;
}

QAbstractAnimation *RotationStackedAnimation::render(QObject *parent)
{
    Q_UNUSED(parent);
    bool dirty = false;
    QPair<QGraphicsWidget *,QGraphicsWidget *> widgets = qMakePair(widgetToAnimate(), d->backWidget);
    QPropertyAnimation *frontAnim, *backAnim;
    d->groupAnim = dynamic_cast<QSequentialAnimationGroup* >(animation());
    if (!d->groupAnim) {

        d->groupAnim = new QSequentialAnimationGroup(parent);
        frontAnim = new QPropertyAnimation(d->frontRotation, "angle", d->groupAnim);
        backAnim = new QPropertyAnimation(d->backRotation, "angle", d->groupAnim);
        setAnimation(d->groupAnim);
        dirty = true;
    } else {
        if (d->groupAnim->animationCount() == 2) {
            frontAnim = dynamic_cast<QPropertyAnimation* >(d->groupAnim->animationAt(0));
            backAnim = dynamic_cast<QPropertyAnimation* >(d->groupAnim->animationAt(1));
        } else {
            kDebug() << "_ Where are my little animations? Duh!";
            return d->groupAnim;
        }
    }

    const qreal widgetFrontWidth = widgets.first->size().width();
    const qreal widgetFrontHeight = widgets.first->size().height();

    const qreal widgetBackWidth = widgets.second->size().width();
    const qreal widgetBackHeight = widgets.second->size().height();

    QPair<QVector3D, QVector3D> vector;

    if(reference() == Center) {

        vector.first = QVector3D(widgetFrontWidth/2, widgetFrontHeight/2, 0);
        vector.second = QVector3D(widgetBackWidth/2, widgetBackHeight/2, 0);

        if(direction() == MoveLeft || direction() == MoveRight) {
            d->frontRotation->setAxis(Qt::YAxis);
            d->backRotation->setAxis(Qt::YAxis);

        if(direction() == MoveLeft) {
        } else {
                d->backRotation->setAngle(265);
                backAnim->setStartValue(d->backRotation->angle());
                frontAnim->setStartValue(0);
                backAnim->setEndValue(360);
                frontAnim->setEndValue(90);
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

    frontAnim->setDuration(duration()/2);
    backAnim->setDuration(duration()/2);

    if (dirty) {
        connect(frontAnim, SIGNAL(stateChanged(QAbstractAnimation::State, QAbstractAnimation::State)),
                this, SLOT(animationStateChange(QAbstractAnimation::State, QAbstractAnimation::State)));
        d->groupAnim->addAnimation(frontAnim);
        d->groupAnim->addAnimation(backAnim);
    }

    return d->groupAnim;
}

void RotationStackedAnimation::animationStateChange(
        QAbstractAnimation::State oldState, QAbstractAnimation::State newState)
{
    if(d->groupAnim->direction() == QAbstractAnimation::Backward) {
        if(newState == QAbstractAnimation::Running) {
            d->sLayout->setCurrentWidgetIndex(0);
        }
    } else {
        if(newState == QAbstractAnimation::Stopped) {
            d->sLayout->setCurrentWidgetIndex(1);
        }
    }
}

}
