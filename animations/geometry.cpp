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

#include "geometry_p.h"

#include <QRect>

#include <kdebug.h>

namespace Plasma
{

GeometryAnimation::GeometryAnimation(QObject *parent)
             : EasingAnimation(parent),
             m_startGeometry(-1, -1, -1, -1)
{
}

GeometryAnimation::~GeometryAnimation()
{
}

void GeometryAnimation::setStartGeometry(const QRectF &geometry)
{
    m_startGeometry = geometry;
}

QRectF GeometryAnimation::startGeometry() const
{
    return m_startGeometry;
}

void GeometryAnimation::setTargetGeometry(const QRectF &geometry)
{
    m_targetGeometry = geometry;
}

QRectF GeometryAnimation::targetGeometry() const
{
    return m_targetGeometry;
}

void GeometryAnimation::updateState(QAbstractAnimation::State newState, QAbstractAnimation::State oldState)
{
    QGraphicsWidget *w = targetWidget();
    if (!w) {
        return;
    }

    if (m_startGeometry == QRectF(-1, -1, -1, -1)) {
        m_startGeometry = w->geometry();
    }

    if (oldState == Stopped && newState == Running) {
        w->setGeometry(direction() == Forward ? m_startGeometry : m_targetGeometry);
    } else if (newState == Stopped) {
        w->setGeometry(direction() == Forward ? m_targetGeometry : m_startGeometry);
    }
}

void GeometryAnimation::updateEffectiveTime(int currentTime)
{
    QGraphicsWidget *w = targetWidget();
    if (w) {
        const qreal delta = currentTime / qreal(duration());

        QRectF newGeo;

        newGeo.moveTopLeft(QPointF(m_startGeometry.left()*(1-delta) + m_targetGeometry.left()*(delta),
                        m_startGeometry.top()*(1-delta) + m_targetGeometry.top()*(delta)));
        if (m_startGeometry.size() != m_targetGeometry.size()) {
            newGeo.setSize(QSizeF(m_startGeometry.width()*(1-delta) + m_targetGeometry.width()*(delta),
                    m_startGeometry.height()*(1-delta) + m_targetGeometry.height()*(delta)));
        } else {
            newGeo.setSize(m_targetGeometry.size());
        }
        w->setGeometry(newGeo);
    }
}

} //namespace Plasma

#include "geometry_p.moc"
