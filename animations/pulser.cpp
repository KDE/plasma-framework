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
#include "widgetsnapshot_p.h"
#include "pulser_p.h"

#include <QEvent>
#include <QGraphicsWidget>
#include <QParallelAnimationGroup>
#include <QPropertyAnimation>
#include <QWeakPointer>

#include <kdebug.h>

namespace Plasma
{

PulseAnimation::PulseAnimation(QObject *parent)
        : EasingAnimation(parent),
          m_zvalue(0),
          m_scale(0),
          m_opacity(0),
          m_endScale(1.5)
{
}

PulseAnimation::~PulseAnimation()
{
    delete m_under.data();
}

void PulseAnimation::setTargetScale(qreal scale)
{
    m_endScale = scale;
}

qreal PulseAnimation::targetScale() const
{
    return m_endScale;
}

void PulseAnimation::setCopy()
{
    QGraphicsWidget *target = targetWidget();

    if (!target) {
        m_under.clear();
        return;
    }

    if (!m_under.data()) {
        m_under = new WidgetSnapShot;
    }

    m_under.data()->setTarget(target);

    m_zvalue = target->zValue() - 1;
    m_scale = target->scale();

    m_under.data()->setOpacity(m_opacity);
    m_under.data()->setScale(m_scale);
    m_under.data()->setZValue(m_zvalue);
}

void PulseAnimation::resetPulser()
{
    if (m_under.data()) {
        m_under.data()->setOpacity(m_opacity);
        m_under.data()->setScale(m_scale);
        m_under.data()->setZValue(m_zvalue);
        m_under.data()->hide();
    }
}

void PulseAnimation::updateState(QAbstractAnimation::State newState, QAbstractAnimation::State oldState)
{
    if (!targetWidget()) {
        return;
    }

    if (oldState == Stopped && newState == Running) {
        if (!m_under.data() || m_under.data()->target() != targetWidget() || m_under.data()->size() != targetWidget()->size()) {
            setCopy();
        }

        if (m_under.data()->isIconBigger()) {
            m_under.data()->setScale(0);
            m_endScale = 1.0;

        } else {
            m_scale = 0;
            m_endScale = 1.5;
        }

        if (m_under.data()->isVisible() == false) {
            m_under.data()->setVisible(true);
        }

        m_under.data()->setOpacity(direction() == Forward ? 1 : 0);
        m_under.data()->setScale(direction() == Forward ? m_scale : m_endScale);
    } else if (newState == Stopped) {
        resetPulser();
    }
}

void PulseAnimation::updateEffectiveTime(int currentTime)
{
    if (m_under.data()) {
        qreal delta = currentTime / qreal(duration());

        if (m_under.data()->isIconBigger()) {
            m_under.data()->setScale(delta);
        } else {
            m_under.data()->setScale(delta);
            delta = (1 - m_endScale) * delta;
            m_under.data()->setScale(1 - delta);
        }

        delta = currentTime / qreal(duration());
        if (direction() == Forward) {
            m_under.data()->setOpacity(1.0 - delta);
        } else if (direction() == Backward) {
            m_under.data()->setOpacity(delta);
        }
    }
}

} //namespace Plasma

#include <../pulser_p.moc>
