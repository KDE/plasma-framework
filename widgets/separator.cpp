/*
 *   Copyright 2009 by Davide Bettio <davide.bettio@kdemail.net>
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

#include "separator.h"

#include <QtGui/QPainter>
#include <QtGui/QGraphicsSceneMouseEvent>

#include "svg.h"

namespace Plasma
{
class SeparatorPrivate
{
    public:
        Svg *svg;
        Qt::Orientation orientation;
};

Separator::Separator(QGraphicsItem *parent, Qt::WindowFlags wFlags)
    : QGraphicsWidget(parent, wFlags),
      d(new SeparatorPrivate())
{
    d->svg = new Svg();
    d->svg->setImagePath("widgets/line");
    d->svg->setContainsMultipleImages(true);
    
    setMaximumHeight(3);
}

Separator::~Separator()
{
    delete d;
}

void Separator::setOrientation(Qt::Orientation orientation)
{
    d->orientation = orientation;
}

Qt::Orientation Separator::orientation()
{
    return d->orientation;
}

void Separator::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    if (d->svg){
        if (d->orientation == Qt::Horizontal){
            d->svg->paint(painter, boundingRect(), "horizontal-line");
        }else{
            d->svg->paint(painter, boundingRect(), "vertical-line");
        }
    }
}

} // Plasma namespace

#include "separator.moc"
