/*
 *   Copyright 2011 Marco Martin <mart@kde.org>
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

#include "qpixmapitem.h"

#include <QtGui/QPainter>


QPixmapItem::QPixmapItem(QDeclarativeItem *parent)
    : QDeclarativeItem(parent),
      m_smooth(false)
{
    setFlag(QGraphicsItem::ItemHasNoContents, false);
}


QPixmapItem::~QPixmapItem()
{
}

void QPixmapItem::setPixmap(const QPixmap &pixmap)
{
    m_pixmap = pixmap;
    update();
    emit nativeWidthChanged();
    emit nativeHeightChanged();
}

QPixmap QPixmapItem::pixmap() const
{
    return m_pixmap;
}

void QPixmapItem::setSmooth(const bool smooth)
{
    if (smooth == m_smooth) {
        return;
    }
    m_smooth = smooth;
    update();
}

bool QPixmapItem::smooth() const
{
    return m_smooth;
}

int QPixmapItem::nativeWidth() const
{
    return m_pixmap.size().width();
}

int QPixmapItem::nativeHeight() const
{
    return m_pixmap.size().height();
}

void QPixmapItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    if (m_pixmap.isNull()) {
        return;
    }
    //do without painter save, faster and the support can be compiled out
    const bool wasAntiAlias = painter->testRenderHint(QPainter::Antialiasing);
    const bool wasSmoothTransform = painter->testRenderHint(QPainter::SmoothPixmapTransform);
    painter->setRenderHint(QPainter::Antialiasing, m_smooth);
    painter->setRenderHint(QPainter::SmoothPixmapTransform, m_smooth);

    painter->drawPixmap(boundingRect(), m_pixmap, m_pixmap.rect());
    painter->setRenderHint(QPainter::Antialiasing, wasAntiAlias);
    painter->setRenderHint(QPainter::SmoothPixmapTransform, wasSmoothTransform);
}


#include "qpixmapitem.moc"
