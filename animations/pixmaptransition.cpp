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
    : EasingAnimation(parent),
      m_cache(false),
      m_dirty(false)
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

void PixmapTransition::setUsesCache(bool cache)
{
    m_cache = cache;
}

bool PixmapTransition::usesCache() const
{
    return m_cache;
}

QPixmap PixmapTransition::targetPixmap() const
{
    return m_targetPixmap;
}

QPixmap PixmapTransition::currentPixmap() const
{
    if (m_cache && !m_dirty) {
        return m_currentPixmap;
    }

    QPixmap currentPixmap;
    qreal delta = currentTime() / qreal(duration());
    if (!m_startPixmap.isNull() && !m_targetPixmap.isNull()) {
        //kDebug() << "transitioning";
        currentPixmap = Plasma::PaintUtils::transition(m_startPixmap, m_targetPixmap, delta);
    } else if (m_startPixmap.isNull()) {
        if (qFuzzyCompare(delta, qreal(1.0))) {
            currentPixmap = alignedTargetPixmap();
            return currentPixmap;
        }

        if (currentPixmap.isNull()) {
            currentPixmap = QPixmap(m_pixmapSize);
        }

        currentPixmap.fill(QColor(0, 0, 0, (int)(((qreal)255)*delta)));
        QPainter p(&currentPixmap);
        p.setCompositionMode(QPainter::CompositionMode_SourceIn);
        //kDebug() << "painting" << m_targetPixmap.rect() << "into" << m_targetRect << "in size" << currentPixmap.size();
        p.drawPixmap(m_targetRect, m_targetPixmap);
        p.end();
    } else if (m_targetPixmap.isNull()) {
        currentPixmap = alignedStartPixmap();
        if (qFuzzyCompare(delta, qreal(1.0))) {
            return m_currentPixmap;
        }
        //kDebug() << "painting" << m_startPixmap.rect() << "into" << m_targetRect << "in size" << currentPixmap.size();
        QPainter p(&currentPixmap);
        p.setCompositionMode(QPainter::CompositionMode_DestinationIn);
        p.fillRect(currentPixmap.rect(), QColor(0, 0, 0, (int)(254 - ((qreal)254)*delta)));
        p.end();
    }

    if (m_cache) {
        const_cast<PixmapTransition *>(this)->m_currentPixmap = currentPixmap;
    }

    return currentPixmap;
}

QPixmap PixmapTransition::alignedTargetPixmap() const
{
    QPixmap pm(m_pixmapSize);
    pm.fill(Qt::transparent);
    QPainter p(&pm);
    p.drawPixmap(m_targetRect, m_targetPixmap);
    return pm;
}

QPixmap PixmapTransition::alignedStartPixmap() const
{
    QPixmap pm(m_pixmapSize);
    pm.fill(Qt::transparent);
    QPainter p(&pm);
    p.drawPixmap(m_startRect, m_startPixmap);
    return pm;
}

void PixmapTransition::updateState(QAbstractAnimation::State newState, QAbstractAnimation::State oldState)
{
    if (oldState == Stopped && newState == Running) {
        m_targetRect = m_targetPixmap.rect();
        m_startRect = m_startPixmap.rect();
        m_pixmapSize = m_startRect.size().expandedTo(m_targetRect.size());
        QRect actualRect = QRect(QPoint(0,0), m_pixmapSize);
        m_targetRect.moveCenter(actualRect.center());
        m_startRect.moveCenter(actualRect.center());
    } else if (QGraphicsWidget *w = targetWidget()) {
        w->update();
    }

    m_dirty = true;
}

void PixmapTransition::updateEffectiveTime(int currentTime)
{
    Q_UNUSED(currentTime)
    m_dirty = true;
    QGraphicsWidget *w = targetWidget();
    if (w) {
        w->update();
    }
}

} //namespace Plasma

#include "pixmaptransition_p.moc"
