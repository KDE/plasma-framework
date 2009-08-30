/***************************************************************************
 *   Copyright 2009 by Alessandro Diaferia <alediaferia@gmail.com>         *
 *   Copyright 2009 by Marco Martin <notmart@gmail.com>                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#ifndef ITEMBACKGROUND_H
#define ITEMBACKGROUND_H

#include <QGraphicsWidget>


/**
 * @class ItemBackground plasma/widgets/itembackground.h
 *
 * @short a background for QGraphicsWidget based item views with animation effects
 *
 * @since 4.4
 */
namespace Plasma {

class FrameSvg;
class ItemBackgroundPrivate;

class ItemBackground : public QGraphicsWidget
{
    Q_OBJECT
public:
    ItemBackground(QGraphicsWidget *parent = 0);
    ~ItemBackground();

    /**
     * Sets a new target geometry we want at the end of animation
     *
     * @arg newGeometry the final geometry target
     */
    void setTarget(const QRectF &newGeometry);
    
    /**
     * set the ItemBackground geometry to be the target geometry, plus the ItemBackground margins 
     */
    void setTargetItem(QGraphicsItem *target);
 
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

protected:
    QVariant itemChange(GraphicsItemChange change, const QVariant &value);

private Q_SLOTS:
    void animationUpdate(qreal progress);
private:
    ItemBackgroundPrivate *d;
};
}

#endif
