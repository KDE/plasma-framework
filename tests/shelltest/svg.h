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

#ifndef Svg_H
#define Svg_H

#include <QtQuick/QQuickPaintedItem>

#include <plasma/svg.h>
#include <plasma/framesvg.h>

//TODO: this will be replaced by PlasmaCore.SvgItem
class Svg : public QQuickPaintedItem
{
Q_OBJECT
public:
    Svg(QQuickItem *parent = 0);
    virtual ~Svg();
    
    void paint(QPainter *painter);
private:
    Plasma::FrameSvg *m_svg;
};

#endif // Svg_H
