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

#include "pulser.h"
#include <QGraphicsWidget>
#include <QEvent>
#include <QAbstractAnimation>
#include <QParallelAnimationGroup>
#include <QDebug>
#include <QPropertyAnimation>

namespace Plasma
{

PulseAnimation::PulseAnimation()
    : animation(0),
      under(0),
      pulseGeometry(0),
      zvalue(0),
      mscale(0),
      opacityAnimation(0),
      geometryAnimation(0),
      scaleAnimation(0)
{

}

PulseAnimation::~PulseAnimation()
{
    //XXX: current plasma::Animation model will delete all animation objects
    //     delete animation;
    //     delete pulseGeometry;
}

void PulseAnimation::setCopy(QGraphicsWidget *copy)
{
    QGraphicsWidget *target = getAnimatedObject();
    under = copy;
    under->setParentItem(target);
    zvalue = target->zValue();
    --zvalue;
    under->setOpacity(0);
    under->setZValue(zvalue);
    mscale = under->scale();
}


void PulseAnimation::updateGeometry(QRectF updated, qreal zCoordinate, qreal scale)
{
    zvalue = zCoordinate;
    --zvalue;
    under->setGeometry(updated);
    under->setPos(0, 0);
    under->setOpacity(0);
    under->setZValue(zvalue);

    /* TODO: move this to a function */
    QRectF initial(under->geometry());
    qreal W = initial.width() * scale * 0.33;
    qreal H = initial.height() * scale * 0.33;
    QRectF end(initial.x() - W, initial.y() -  H, initial.width() * scale, initial.height() * scale);
    geometryAnimation->setEndValue(end);
}

void PulseAnimation::resetPulser()
{
    under->setGeometry(*pulseGeometry);
    under->setOpacity(0);
    under->setZValue(zvalue);
    under->setScale(mscale);
}


void PulseAnimation::createAnimation(qreal duration, qreal scale)
{
    /* Fallback to parent widget if we don't have one 'shadow' widget */
    if (!under) {
        setCopy(getAnimatedObject());
    }

    pulseGeometry = new QRectF(under->geometry());
    QParallelAnimationGroup *group = new QParallelAnimationGroup(this);
    opacityAnimation = new QPropertyAnimation(under, "opacity");
    opacityAnimation->setDuration(duration);
    opacityAnimation->setEndValue(0);
    group->addAnimation(opacityAnimation);

    /* TODO: move this to a function */
    geometryAnimation = new QPropertyAnimation(under, "geometry");
    geometryAnimation->setDuration(duration);
    QRectF initial(under->geometry());
    qreal W = initial.width() * scale * 0.33;
    qreal H = initial.height() * scale * 0.33;
    QRectF end(initial.x() - W, initial.y() -  H, initial.width() * scale,
		initial.height() * scale);
    geometryAnimation->setEndValue(end);
    group->addAnimation(geometryAnimation);

    scaleAnimation = new QPropertyAnimation(under, "scale");
    scaleAnimation->setDuration(duration);
    scaleAnimation->setEndValue(scale);
    group->addAnimation(scaleAnimation);

    animation = group;

    //XXX: current plasma::Animation model doesn't reuse animation objects
    //connect(animation, SIGNAL(finished()), this, SLOT(resetPulser()));
}

QAbstractAnimation* PulseAnimation::render(QObject* parent)
{
    Q_UNUSED(parent);
    createAnimation();
    under->setOpacity(1);
    return animation;
}

void PulseAnimation::start()
{
    under->setOpacity(1);
    animation->start();
}

} //namespace Plasma
