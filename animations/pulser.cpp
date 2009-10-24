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

    QAbstractAnimation *animation;
    QGraphicsWidget *under;
    /* FIXME: clean up this guy */
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
    //XXX: current plasma::Animation model will delete all animation objects
    //     delete animation;
    //     delete pulseGeometry;
    delete d;
}

void PulseAnimation::setCopy(QGraphicsWidget *copy)
{
    QGraphicsWidget *target = widgetToAnimate();
    d->under = copy;
    if (d->under == target) {
        d->mopacity = d->under->opacity();
    } else {
        d->under->setParentItem(target);
        d->mopacity = 0;
    }

    d->zvalue = target->zValue();
    --d->zvalue;
    d->under->setOpacity(0);
    d->under->setZValue(d->zvalue);
    d->mscale = d->under->scale();
}

void PulseAnimation::resetPulser()
{
    d->under->setGeometry(*d->pulseGeometry);
    d->under->setOpacity(d->mopacity);
    d->under->setZValue(d->zvalue);
    d->under->setScale(d->mscale);
}


void PulseAnimation::createAnimation(qreal duration, qreal scale)
{
    bool dirty = false;
    QGraphicsWidget *target = widgetToAnimate();
    /* Fallback to parent widget if we don't have one 'shadow' widget */
    if (!d->under) {
        setCopy(target);
    } else if (d->under != target) {
        delete d->under;
        d->under = new QGraphicsWidget(target);
        setCopy(d->under);
    }


    QParallelAnimationGroup *anim = dynamic_cast<QParallelAnimationGroup* >(animation());
    if (!anim) {
	d->pulseGeometry = new QRectF(d->under->geometry());
	QParallelAnimationGroup *group = new QParallelAnimationGroup(this);
	d->opacityAnimation = new QPropertyAnimation(d->under, "opacity");
	d->opacityAnimation->setDuration(duration);
	d->opacityAnimation->setEndValue(0);
	group->addAnimation(d->opacityAnimation);

	/* TODO: move this to a function */
	d->geometryAnimation = new QPropertyAnimation(d->under, "geometry");
	d->geometryAnimation->setDuration(duration);
	QRectF initial(d->under->geometry());
	qreal W = initial.width() * scale * 0.33;
	qreal H = initial.height() * scale * 0.33;
	QRectF end(initial.x() - W, initial.y() -  H, initial.width() * scale,
		   initial.height() * scale);
	d->geometryAnimation->setEndValue(end);
	group->addAnimation(d->geometryAnimation);

	d->scaleAnimation = new QPropertyAnimation(d->under, "scale");
	d->scaleAnimation->setDuration(duration);
	d->scaleAnimation->setEndValue(scale);
	/* The group takes ownership of all animations */
	group->addAnimation(d->scaleAnimation);
	d->animation = group;
	setAnimation(d->animation);
	dirty = true;

    } else {

        /* Stop the animation or the widget will be deformed.
         */
        QAbstractAnimation::State temp = anim->state();
        if (temp == QAbstractAnimation::Running) {
            anim->stop();
            /* TODO: will need to watch stateChanged signal
             * and *then* reset the geometry
             */
        } else {
            /* TODO: move this to a function */
            *(d->pulseGeometry) = d->under->geometry();
            d->opacityAnimation->setEndValue(0);
            QRectF initial(d->under->geometry());
            qreal W = initial.width() * scale * 0.33;
            qreal H = initial.height() * scale * 0.33;
            QRectF end(initial.x() - W, initial.y() -  H, initial.width() * scale,
                       initial.height() * scale);
            d->geometryAnimation->setEndValue(end);
        }

    }

    if (dirty) {
	//This makes sure that if there is *not* a shadow widget, the
	//parent widget will still remain visible
	connect(d->animation, SIGNAL(finished()), this, SLOT(resetPulser()));
    }
}

QAbstractAnimation* PulseAnimation::render(QObject* parent)
{
    Q_UNUSED(parent)

    createAnimation(duration());
    d->under->setOpacity(1);
    return d->animation;
}

} //namespace Plasma
