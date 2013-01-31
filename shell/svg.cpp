/*
 *  Copyright 2012 Marco Martin <mart@kde.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "svg.h"

Svg::Svg(QQuickItem *parent)
    : QQuickPaintedItem(parent)
{
    m_svg = new Plasma::FrameSvg(this);
    m_svg->setImagePath("widgets/background");
}

Svg::~Svg()
{
    
}

void Svg::paint(QPainter *painter)
{
    m_svg->resizeFrame(QSize(width(), height()));
    m_svg->paintFrame(painter, QPointF(0, 0));
}

#include "moc_svg.cpp"
