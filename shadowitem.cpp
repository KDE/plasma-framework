/*
 *   Copyright 2007 by Zack Rusin <zack@kde.org>
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

#include "shadowitem_p.h"

#include "effects/blur.cpp"

#include <QPainter>
#include <QImage>
#include <QDebug>

namespace Plasma
{

ShadowItem::ShadowItem(QGraphicsItem *item)
{
    setZValue(20);
    m_shadowParent = 0;
    m_offset = QPointF(12, 12);
    setShadowParent(item);
}

void ShadowItem::setShadowParent(QGraphicsItem *item)
{
    m_shadowParent = item;
    adjustPosition();
    generate();
}


QGraphicsItem * ShadowItem::shadowParent() const
{
    return m_shadowParent;
}

QSize ShadowItem::shadowedSize() const
{
    QSize s = boundingRect().size().toSize();
    return s - QSize(32, 32);
}

void ShadowItem::generate()
{
    if (!m_shadowParent) {
        return;
    }

    QPainterPath path = m_shadowParent->shape();
    QRectF rect = path.boundingRect();
    QSize s = rect.size().toSize() + QSize(30, 30);
    QImage img(s, QImage::Format_ARGB32_Premultiplied);
    img.fill(0);
    QPainter p(&img);
    p.translate(15, 15);
    p.setRenderHint(QPainter::Antialiasing);
    p.setCompositionMode(QPainter::CompositionMode_Source);
    p.fillPath(path, Qt::gray);
    p.end();
#ifdef DO_GLOW
    QImage blurred = img;
    expblur<16, 7>(img, 7);
    p.begin(&img);
    p.setCompositionMode(QPainter::CompositionMode_Plus);
    p.drawImage(0, 0, blurred);
    p.end();
#else
    expblur<16, 7>(img, 7);
#endif

    setPixmap(QPixmap::fromImage(img));
}


void ShadowItem::adjustPosition()
{
    if (!m_shadowParent) {
        return;
    }

    setPos(m_shadowParent->pos() - m_offset);
}

void ShadowItem::setOffset(const QPointF &offset)
{
    m_offset = offset;
}

QPointF ShadowItem::offset() const
{
    return m_offset;
}

}
