/*
 * Copyright (C) 2010 Bruno Abinader <bruno.abinader@indt.org.br>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <plasma/private/effects/ripple_p.h>
#include "water_p.h"

namespace Plasma
{

WaterAnimation::WaterAnimation(QObject *parent)
    : EasingAnimation(parent),
      m_offset(1)
{
}

qint8 WaterAnimation::offset() const
{
    return m_offset;
}

void WaterAnimation::setOffset(qint8 offset)
{
    m_offset = offset;
}

void WaterAnimation::updateState(QAbstractAnimation::State newState, QAbstractAnimation::State oldState)
{
    QGraphicsWidget *widget = targetWidget();
    if (!widget) {
        return;
    }

    RippleEffect *effect = qobject_cast<RippleEffect*>(widget->graphicsEffect());
    if (!effect) {
        effect = new RippleEffect(widget);
        widget->setGraphicsEffect(effect);
    }

    if (oldState == Stopped && newState == Running) {
        effect->setOffset(m_offset);
        effect->setEnabled(true);
    } else if (newState == Stopped) {
        effect->setEnabled(false);
    }
}

void WaterAnimation::updateEffectiveTime(int currentTime)
{
    QGraphicsWidget *widget = targetWidget();
    if (widget && widget->graphicsEffect()) {
        widget->graphicsEffect()->setProperty("opacity", currentTime / qreal(duration()));
    }
}

} // namespace Plasma
