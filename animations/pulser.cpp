/* Copyright (C)  2009  Adenilson Cavalcanti <cavalcantii@gmail.com>
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

/* TODO:
 * - fix centering of pulsed shadow object
 */

#include "pulser_p.h"
#include "plasma/private/pulsershadow_p.h"
#include <QAbstractAnimation>
#include <QDebug>
#include <QEvent>
#include <QGraphicsWidget>
#include <QParallelAnimationGroup>
#include <QPropertyAnimation>

#include <kdebug.h>
namespace Plasma
{

class PulseAnimationPrivate
{
public :
    PulseAnimationPrivate()
        : animation(0),
          under(0),
          pulseGeometry(0),
          zvalue(0),
          mscale(0),
          opacityAnimation(0),
          geometryAnimation(0),
          scaleAnimation(0)
    {}

    ~PulseAnimationPrivate()
    {
        if (pulseGeometry)
            delete pulseGeometry;
    }

    QAbstractAnimation *animation;
    QGraphicsWidget *under;
    QRectF *pulseGeometry;
    qreal zvalue, mscale, mopacity;
    QPropertyAnimation *opacityAnimation;
    QPropertyAnimation *geometryAnimation;
    QPropertyAnimation *scaleAnimation;
};


PulseAnimation::PulseAnimation(): d(new PulseAnimationPrivate)
{

}

PulseAnimation::~PulseAnimation()
{
    delete d;
}

void PulseAnimation::setCopy()
{
    QGraphicsWidget *target = widgetToAnimate();
    /* copy the parent to an image, the animation will happen on the
     * pixmap copy.
     */
    ShadowFake *shadow = 0;
    if (!d->under)
        shadow  = new ShadowFake;

    shadow->copyTarget(target);

    if (d->pulseGeometry)
        delete d->pulseGeometry;
    d->pulseGeometry = new QRectF(target->geometry());

    d->mopacity = 0;
    d->zvalue = target->zValue();
    --d->zvalue;
    d->mscale = target->scale();

    d->under = shadow;
    d->under->setOpacity(d->mopacity);
    d->under->setScale(d->mscale);
    d->under->setZValue(d->zvalue);

}

void PulseAnimation::resetPulser()
{
    d->under->setGeometry(*(d->pulseGeometry));
    d->under->setOpacity(d->mopacity);
    d->under->setScale(d->mscale);
    d->under->setZValue(d->zvalue);
}


void PulseAnimation::createAnimation(qreal duration, qreal scale)
{
    bool dirty = false;
    if (!d->under)
        setCopy();

    QParallelAnimationGroup *anim = dynamic_cast<QParallelAnimationGroup* >(animation());
    if (!anim) {
        QParallelAnimationGroup *group = new QParallelAnimationGroup(this);
        d->opacityAnimation = new QPropertyAnimation(d->under, "opacity");
        d->opacityAnimation->setDuration(duration);
        d->opacityAnimation->setStartValue(100);
        d->opacityAnimation->setEndValue(0);
        group->addAnimation(d->opacityAnimation);

        QRectF initial(d->under->geometry());
        QPointF tmp(initial.width() * 0.5, initial.height() * 0.5);
        d->under->setTransformOriginPoint(tmp);

        d->scaleAnimation = new QPropertyAnimation(d->under, "scale");
        d->scaleAnimation->setDuration(duration);
        d->scaleAnimation->setStartValue(d->mscale);
        d->scaleAnimation->setEndValue(scale);

        /* The group takes ownership of all animations */
        group->addAnimation(d->scaleAnimation);
        d->animation = group;
        setAnimation(d->animation);
        dirty = true;

    } else {

        QAbstractAnimation::State temp = anim->state();
        if (temp == QAbstractAnimation::Running) {
            anim->stop();
            /* TODO: will need to watch stateChanged signal
             * and *then* reset the geometry
             */
        } else {
            if ((*d->pulseGeometry) != widgetToAnimate()->geometry()) {
                /*FIXME: it crashes when deleting old the image in ShadowFake
                 * setCopy();
                 */
                qDebug() << "PulseAnimation:: it should update the geom....";
            }

            d->opacityAnimation->setEndValue(0);
            d->scaleAnimation->setEndValue(scale);

            QRectF initial(d->under->geometry());
            QPointF tmp(initial.width() * 0.5, initial.height() * 0.5);
            d->under->setTransformOriginPoint(tmp);
        }
    }

    if (dirty)
        connect(d->animation, SIGNAL(finished()), this, SLOT(resetPulser()));
}

QAbstractAnimation* PulseAnimation::render(QObject* parent)
{
    Q_UNUSED(parent)

    createAnimation(duration());
    return d->animation;
}

} //namespace Plasma
