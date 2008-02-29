/*
 *   Copyright 2007 by Matias Valdenegro T. <mvaldenegro@informatica.utem.cl>
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

#include "rectangle.h"

#include <QPainter>

namespace Plasma {

Rectangle::Rectangle(Widget *parent)
    : Widget(parent)
{
    resize(400.0f, 400.0f);
    setFlag(QGraphicsItem::ItemIsMovable);
}

Rectangle::~Rectangle()
{
}

Qt::Orientations Rectangle::expandingDirections() const
{
    return Qt::Horizontal | Qt::Vertical;
}

void Rectangle::paintWidget(QPainter *p, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    p->setBrush(Qt::white);
    p->setPen(Qt::black);
    p->setOpacity(0.5f);
    p->drawRect(localGeometry());
}

}
