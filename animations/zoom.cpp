/*
 *   Copyright 2009 Igor Trindade Oliveira <igor.oliveira@indt.org.br>
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

#include "zoom_p.h"

#include <kdebug.h>

namespace Plasma
{

ZoomAnimation::ZoomAnimation(QObject *parent)
    : EasingAnimation(parent),
      m_zoom(0)
{
}

ZoomAnimation::~ZoomAnimation()
{
}

void ZoomAnimation::setZoom(qreal zoom)
{
    m_zoom = zoom;
}

qreal ZoomAnimation::zoom() const
{
    return m_zoom;
}

void ZoomAnimation::updateState(QAbstractAnimation::State newState, QAbstractAnimation::State oldState)
{
    QGraphicsWidget *w = targetWidget();
    if (!w) {
        return;
    }

    if (oldState == Stopped && newState == Running) {
        w->setTransformOriginPoint(w->size().width()/2, w->size().height()/2);
        w->setScale(direction() == Forward ? 1 : m_zoom);
    } else if (newState == Stopped) {
        w->setScale(direction() == Forward ? m_zoom : 1);
    }
}

void ZoomAnimation::updateEffectiveTime(int currentTime)
{
    QGraphicsWidget *w = targetWidget();
    if (w) {
        qreal delta = currentTime / qreal(duration());
        if (m_zoom != 1) {
            delta = (1 - m_zoom) * delta;
            w->setScale(1 - delta);
        } else {
            w->setScale(delta);
        }
    }
}

} //namespace Plasma
