/*
 *   Copyright 2009 Mehmet Ali Akmanalp <makmanalp@wpi.edu>
 *   Copyright 2010 Marco Martin <notmart@gmail.com>
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

#include "pixmaptransition_p.h"

#include <QPainter>
#include <QPixmap>

#include <kdebug.h>

#include "paintutils.h"

namespace Plasma
{

PixmapTransition::PixmapTransition(QObject *parent)
             : EasingAnimation(parent)
{
}

PixmapTransition::~PixmapTransition()
{
}

void PixmapTransition::setStartPixmap(const QPixmap &pixmap)
{
    if (state() == Running) {
        stop();
    }

    m_startPixmap = pixmap;

    //this will center the pixmaps if needed
    updateEffectiveTime(0);
}

QPixmap PixmapTransition::startPixmap() const
{
    return m_startPixmap;
}

void PixmapTransition::setTargetPixmap(const QPixmap &pixmap)
{
    if (state() == Running) {
        stop();
    }

    m_targetPixmap = pixmap;

    updateEffectiveTime(0);
}

QPixmap PixmapTransition::targetPixmap() const
{
    return m_targetPixmap;
}

QPixmap PixmapTransition::currentPixmap() const
{
    return m_currentPixmap;
}

void PixmapTransition::updateState(QAbstractAnimation::State newState, QAbstractAnimation::State oldState)
{
    QGraphicsWidget *w = targetWidget();
    if (!w) {
        return;
    }

    if (!m_startPixmap.isNull() && !m_targetPixmap.isNull()) {
        if (oldState == Stopped && newState == Running) {
            Plasma::PaintUtils::centerPixmaps(m_startPixmap, m_targetPixmap);
            m_currentPixmap = (direction() == Forward ? m_startPixmap : m_targetPixmap);
        } else if (newState == Stopped) {
            m_currentPixmap = (direction() == Forward ? m_targetPixmap : m_startPixmap);
        }
    } else if (m_startPixmap.isNull()) {
        if (oldState == Stopped && newState == Running) {
            if (direction() == Forward) {
                m_currentPixmap = QPixmap(m_targetPixmap.size());
                m_currentPixmap.fill(Qt::transparent);
            } else {
                m_currentPixmap = m_targetPixmap;
            }
        } else if (newState == Stopped) {
            if (direction() == Forward) {
                m_currentPixmap = m_targetPixmap;
            } else {
                m_currentPixmap = QPixmap(m_targetPixmap.size());
                m_currentPixmap.fill(Qt::transparent);
            }
        }
    } else if (m_targetPixmap.isNull()) {
        if (oldState == Stopped && newState == Running) {
            if (direction() == Forward) {
                m_currentPixmap = m_targetPixmap;
            } else {
                m_currentPixmap = QPixmap(m_targetPixmap.size());
                m_currentPixmap.fill(Qt::transparent);
            }
        } else if (newState == Stopped) {
            if (direction() == Forward) {
                m_currentPixmap = QPixmap(m_targetPixmap.size());
                m_currentPixmap.fill(Qt::transparent);
            } else {
                m_currentPixmap = m_targetPixmap;
            }
        }
    }

    w->update();
}

void PixmapTransition::updateEffectiveTime(int currentTime)
{
    QGraphicsWidget *w = targetWidget();
    if (w) {
        qreal delta = currentTime / qreal(duration());
        if (!m_startPixmap.isNull() && !m_targetPixmap.isNull()) {
            m_currentPixmap = Plasma::PaintUtils::transition(m_startPixmap, m_targetPixmap, delta);
        } else if (m_startPixmap.isNull()) {
            if (qFuzzyCompare(delta, qreal(1.0))) {
                m_currentPixmap = m_targetPixmap;
                return;
            }
            if (m_currentPixmap.isNull()) {
                m_currentPixmap = QPixmap(m_targetPixmap.size());
            }
            m_currentPixmap.fill(QColor(0, 0, 0, (int)(((qreal)255)*delta)));
            QPainter p(&m_currentPixmap);
            p.setCompositionMode(QPainter::CompositionMode_SourceIn);
            p.drawPixmap(m_currentPixmap.rect(), m_targetPixmap, m_targetPixmap.rect());
            p.end();
        } else if (m_targetPixmap.isNull()) {
            m_currentPixmap = m_startPixmap;
            if (qFuzzyCompare(delta, qreal(1.0))) {
                return;
            }
            QPainter p(&m_currentPixmap);
            p.setCompositionMode(QPainter::CompositionMode_DestinationIn);
            p.fillRect(m_currentPixmap.rect(), QColor(0, 0, 0, (int)(254 - ((qreal)254)*delta)));
            p.end();
        }
    }

    if (w) {
        w->update();
    }
}

} //namespace Plasma

#include <../pixmaptransition_p.moc>
