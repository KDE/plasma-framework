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
 * - fix opacity (for some reason is not working)
 */

#include "pulser_p.h"
#include "plasma/private/pulsershadow_p.h"
#include <QAbstractAnimation>
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
        : under(0),
          zvalue(0),
          scale(0),
          endScale(1.5),
          opacityAnimation(0),
          scaleAnimation(0)
    {}

    ~PulseAnimationPrivate()
    { }

    QGraphicsWidget *under;
    qreal zvalue, scale, mopacity;
    qreal endScale;
    QPropertyAnimation *opacityAnimation;
    QPropertyAnimation *scaleAnimation;
    QWeakPointer<QParallelAnimationGroup> animation;
};


void PulseAnimation::setWidgetToAnimate(QGraphicsWidget *widget)
{
    if (widget == widgetToAnimate()) {
        return;
    }

    Animation::setWidgetToAnimate(widget);
    if (widget) {
        if (d->under) {
            delete d->under;
            d->under = 0;
        }
        createAnimation(Animation::duration(), d->endScale);
    }
}

PulseAnimation::PulseAnimation(QObject *parent)
        : Animation(parent), d(new PulseAnimationPrivate)
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
    else
        shadow = dynamic_cast<ShadowFake*>(d->under);

    shadow->copyTarget(target);

    d->mopacity = 0;
    d->zvalue = target->zValue();
    --d->zvalue;
    d->scale = target->scale();

    d->under = shadow;
    d->under->setOpacity(d->mopacity);
    d->under->setScale(d->scale);
    d->under->setZValue(d->zvalue);

}

void PulseAnimation::resetPulser()
{
    d->under->setOpacity(d->mopacity);
    d->under->setScale(d->scale);
    d->under->setZValue(d->zvalue);
}


void PulseAnimation::createAnimation(qreal duration, qreal scale)
{
    if (!d->under) {
        setCopy();
    }

    QParallelAnimationGroup *anim = d->animation.data();
    if (!anim) {
        QParallelAnimationGroup *group = new QParallelAnimationGroup(this);
        connect(group, SIGNAL(finished()), this, SLOT(resetPulser()));

        d->opacityAnimation = new QPropertyAnimation(d->under, "opacity");
        d->opacityAnimation->setDuration(duration);
        d->opacityAnimation->setStartValue(1);
        d->opacityAnimation->setEndValue(0);
        group->addAnimation(d->opacityAnimation);

        d->scaleAnimation = new QPropertyAnimation(d->under, "scale");
        d->scaleAnimation->setDuration(duration);
        d->scaleAnimation->setStartValue(d->scale);
        d->scaleAnimation->setEndValue(scale);

        /* The group takes ownership of all animations */
        group->addAnimation(d->scaleAnimation);
        d->animation = group;
    }
}

void PulseAnimation::updateState(QAbstractAnimation::State newState, QAbstractAnimation::State oldState)
{

   if (oldState == Stopped && newState == Running) {
       QParallelAnimationGroup *anim = d->animation.data();
       QAbstractAnimation::State temp = anim->state();
       if (temp == QAbstractAnimation::Running) {
           anim->stop();
           /* TODO: will need to watch stateChanged signal
            * and *then* reset the geometry
            */
       } else {
           if (d->under->size() != widgetToAnimate()->size()) {
               setCopy();
           }
           d->opacityAnimation->setEndValue(0);
           /* TODO: save the scale factor somewhere */
           d->scaleAnimation->setEndValue(1.5);
       }

       d->animation.data()->start();

   } else if (newState == Stopped) {

   }

}

void PulseAnimation::updateCurrentTime(int currentTime)
{

    /* TODO: reuse this time and update properties without the
     * the need of an internal animation group.
     */
}

} //namespace Plasma
