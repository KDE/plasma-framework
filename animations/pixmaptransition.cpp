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

#include <QPixmap>

#include <kdebug.h>

#include "paintutils.h"

namespace Plasma
{

PixmapTransition::PixmapTransition(QObject *parent)
             : Animation(parent)
{
}

PixmapTransition::~PixmapTransition()
{
}

void PixmapTransition::setStartPixmap(const QPixmap &pixmap)
{
    if (state() != Running) {
        m_startPixmap = pixmap;
        m_currentPixmap = pixmap;
    } else {
        m_startPixmap = m_currentPixmap;
        stop();
    }
}

QPixmap PixmapTransition::startPixmap() const
{
    return m_startPixmap;
}

void PixmapTransition::setTargetPixmap(const QPixmap &pixmap)
{
    m_targetPixmap = pixmap;
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

    if (oldState == Stopped && newState == Running) {
        Plasma::PaintUtils::centerPixmaps(m_startPixmap, m_targetPixmap);
        m_currentPixmap = (direction() == Forward ? m_startPixmap : m_targetPixmap);
    } else if (newState == Stopped) {
        m_currentPixmap = (direction() == Forward ? m_targetPixmap : m_startPixmap);
    }

    w->update();
}

void PixmapTransition::updateCurrentTime(int currentTime)
{
    QGraphicsWidget *w = targetWidget();
    if (w) {
        qreal delta = currentTime / qreal(duration());
              delta *= Animation::easingCurve().valueForProgress(delta);
        m_currentPixmap = Plasma::PaintUtils::transition(m_startPixmap, m_targetPixmap, delta);
    }

    Animation::updateCurrentTime(currentTime);
    if (w) {
        w->update();
    }
}

} //namespace Plasma
