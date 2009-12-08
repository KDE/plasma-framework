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
#include "pulser_p.h"
#include "plasma/private/pulsershadow_p.h"
#include <QEvent>
#include <QGraphicsWidget>
#include <QParallelAnimationGroup>
#include <QPropertyAnimation>

#include <kdebug.h>
namespace Plasma
{

PulseAnimation::PulseAnimation(QObject *parent)
        : Animation(parent),
          zvalue(0), scale(0), mopacity(0), endScale(1.5),
          under(0)
{
}

PulseAnimation::~PulseAnimation()
{
}

void PulseAnimation::setWidgetToAnimate(QGraphicsWidget *widget)
{
    if (widget == widgetToAnimate()) {
        return;
    }

    Animation::setWidgetToAnimate(widget);
    if (widget) {
        setCopy();
    }
}

void PulseAnimation::setCopy()
{
    QGraphicsWidget *target = widgetToAnimate();
    /* the parent to an image, the animation will happen on the pixmap copy.
     */
    if (!under)
        under  = new ShadowFake;

    under->copyTarget(target);

    zvalue = target->zValue();
    --zvalue;
    scale = target->scale();

    under->setOpacity(mopacity);
    under->setScale(scale);
    under->setZValue(zvalue);

}

void PulseAnimation::resetPulser()
{
    under->setOpacity(mopacity);
    under->setScale(scale);
    under->setZValue(zvalue);
}


void PulseAnimation::updateState(QAbstractAnimation::State newState, QAbstractAnimation::State oldState)
{

   if (oldState == Stopped && newState == Running) {
       if (under->size() != widgetToAnimate()->size()) {
           setCopy();
       }

       under->setOpacity(direction() == Forward ? 1 : 0);
       under->setScale(direction() == Forward ? scale : endScale);

   } else if (newState == Stopped) {
       resetPulser();
   }

}

void PulseAnimation::updateCurrentTime(int currentTime)
{
    QGraphicsWidget *w = under;
    if (w) {
        qreal delta = easingCurve().valueForProgress(
                currentTime / qreal(duration()));
        delta = (1 - endScale) * delta;
        w->setScale(1 - delta);
    }

    if (w) {
        qreal delta = currentTime / qreal(duration());
        if (direction() == Forward) {
            w->setOpacity(1.0 - delta);
        } else if (direction() == Backward) {
            w->setOpacity(delta);
        }
    }

    Animation::updateCurrentTime(currentTime);
}

} //namespace Plasma
