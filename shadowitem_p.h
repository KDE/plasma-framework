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

#ifndef SHADOWITEM_P_H
#define SHADOWITEM_P_H

#include <QtGui/QGraphicsPixmapItem>

namespace Plasma
{

class ShadowItem : public QGraphicsPixmapItem
{
public:
    explicit ShadowItem(QGraphicsItem* item);

    void setShadowParent(QGraphicsItem *item);
    QGraphicsItem *shadowParent() const;

    void setOffset(const QPointF &offset);
    QPointF offset() const;

    QSize shadowedSize() const;
    void generate();
    void adjustPosition();

private:
    QGraphicsItem *m_shadowParent;
    QPointF m_offset;
};

}

#endif
