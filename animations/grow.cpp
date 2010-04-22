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

#include "grow_p.h"

#include <QRect>
#include <kdebug.h>

namespace Plasma
{

GrowAnimation::GrowAnimation(QObject *parent, qreal factor)
    : EasingAnimation(parent), m_animFactor(factor)
{
}

void GrowAnimation::setFactor(const qreal factor)
{
    m_animFactor = qMax(qreal(0.0), factor);
}

qreal GrowAnimation::factor() const
{
    return m_animFactor;
}

void GrowAnimation::updateEffectiveTime(int currentTime)
{
    QGraphicsWidget *w = targetWidget();
    if (w && state() == QAbstractAnimation::Running) {
        const qreal delta = currentTime / qreal(duration());
        QRectF geometry;
        geometry.setTopLeft(m_startGeometry.topLeft() * (1-delta) + (m_targetGeometry.topLeft() * delta));
        geometry.setSize(m_startGeometry.size() * (1-delta) + (m_targetGeometry.size() * delta));
        w->setGeometry(geometry);
    }
}

void GrowAnimation::updateState(QAbstractAnimation::State newState, QAbstractAnimation::State oldState)
{
    if (oldState == QAbstractAnimation::Stopped && newState == QAbstractAnimation::Running) {
        QGraphicsWidget *widget = targetWidget();
        if (!widget) {
            return;
        }

        QSizeF minimum = widget->effectiveSizeHint(Qt::MinimumSize);
        QSizeF maximum = widget->effectiveSizeHint(Qt::MaximumSize);
        m_startGeometry = widget->geometry();
        qreal w = m_startGeometry.width();
        qreal h = m_startGeometry.height();
        qreal factor = m_animFactor;

        //compute new geometry values
        qreal newWidth;
        qreal newHeight;
        if (direction() == QAbstractAnimation::Forward) {
            newWidth = qBound(minimum.width(), w * factor, maximum.width());
            newHeight = qBound(minimum.height(), h * factor, maximum.height());
        } else {
            newWidth = qBound(minimum.width(), w / factor, maximum.width());
            newHeight = qBound(minimum.height(), h / factor, maximum.height());
        }

        qreal newX;
        qreal newY;
        newX = m_startGeometry.x() - (newWidth - w)/2;
        newY = m_startGeometry.y() - (newHeight - h)/2;

        if (direction() == QAbstractAnimation::Forward) {
            //the end geometry gets rounded to prevent things looking ugly
            m_targetGeometry = QRect(newX, newY, newWidth, newHeight);
        } else {
            m_targetGeometry = m_startGeometry;
            m_startGeometry = QRectF(newX, newY, newWidth, newHeight);
        }
    }
}

} //namespace Plasma

#include <../grow_p.moc>

