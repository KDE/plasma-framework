/*
 *   Copyright 2009 Mehmet Ali Akmanalp <makmanalp@wpi.edu>
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

#include "fade_p.h"

#include <QRect>

#include <kdebug.h>

namespace Plasma
{

FadeAnimation::FadeAnimation(QObject *parent)
             : EasingAnimation(parent),
               m_startOpacity(0),
               m_targetOpacity(1)
{
}

FadeAnimation::~FadeAnimation()
{
}

void FadeAnimation::setStartOpacity(qreal factor)
{
    m_startOpacity = qBound(qreal(0.0), factor, qreal(1.0));
}

qreal FadeAnimation::startOpacity() const
{
    return m_startOpacity;
}

void FadeAnimation::setTargetOpacity(qreal factor)
{
    m_targetOpacity = qBound(qreal(0.0), factor, qreal(1.0));
}

qreal FadeAnimation::targetOpacity() const
{
    return m_targetOpacity;
}

void FadeAnimation::updateState(QAbstractAnimation::State newState, QAbstractAnimation::State oldState)
{
    QGraphicsWidget *w = targetWidget();
    if (!w) {
        return;
    }

    if (oldState == Stopped && newState == Running) {
        w->setOpacity(direction() == Forward ? m_startOpacity : m_targetOpacity);
    } else if (newState == Stopped) {
        w->setOpacity(direction() == Forward ? m_targetOpacity : m_startOpacity);
    }
}

void FadeAnimation::updateEffectiveTime(int currentTime)
{
    QGraphicsWidget *w = targetWidget();
    if (w) {
        qreal delta = currentTime / qreal(duration());
        delta *= m_startOpacity - m_targetOpacity;
        w->setOpacity(m_startOpacity - delta);
    }
}

} //namespace Plasma

#include <../fade_p.moc>
