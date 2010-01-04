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
          m_zvalue(0),
          m_scale(0),
          m_opacity(0),
          m_endScale(1.5),
          m_under(0)
{
}

PulseAnimation::~PulseAnimation()
{
    delete m_under;
}

void PulseAnimation::setCopy()
{
    QGraphicsWidget *target = targetWidget();

    if (!target) {
        delete m_under;
        m_under = 0;
        return;
    }

    if (!m_under) {
        m_under = new ShadowFake;
    }

    m_under->setTarget(target);

    m_zvalue = target->zValue() - 1;
    m_scale = target->scale();

    m_under->setOpacity(m_opacity);
    m_under->setScale(m_scale);
    m_under->setZValue(m_zvalue);
}

void PulseAnimation::resetPulser()
{
    if (m_under) {
        m_under->setOpacity(m_opacity);
        m_under->setScale(m_scale);
        m_under->setZValue(m_zvalue);
    }
}

void PulseAnimation::updateState(QAbstractAnimation::State newState, QAbstractAnimation::State oldState)
{
    if (!targetWidget()) {
        return;
    }

    if (oldState == Stopped && newState == Running) {
        if (m_under->target() != targetWidget() || m_under->size() != targetWidget()->size()) {
            setCopy();
        }

        m_under->setOpacity(direction() == Forward ? 1 : 0);
        m_under->setScale(direction() == Forward ? m_scale : m_endScale);
    } else if (newState == Stopped) {
        resetPulser();
    }
}

void PulseAnimation::updateCurrentTime(int currentTime)
{
    if (m_under) {
        qreal delta = Animation::easingCurve().valueForProgress( currentTime / qreal(duration()));
        delta = (1 - m_endScale) * delta;
        m_under->setScale(1 - delta);

        delta = currentTime / qreal(duration());
        if (direction() == Forward) {
            m_under->setOpacity(1.0 - delta);
        } else if (direction() == Backward) {
            m_under->setOpacity(delta);
        }
    }
}

} //namespace Plasma
