/*
    Copyright (C) 2009 Adenilson Cavalcanti <adenilson.silva@idnt.org.br>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "pulsershadow_p.h"
#include <QPainter>
#include <QImage>
#include <QPixmap>
#include <QStyleOptionGraphicsItem>

namespace Plasma
{

ShadowFake::ShadowFake(QGraphicsItem *parent): QGraphicsWidget(parent),
                                                   photo(0)
{ }

ShadowFake::~ShadowFake()
{
    delete photo;
}

void ShadowFake::copyTarget(QGraphicsWidget *target)
{
    setParentItem(target);
    resize(target->size());
    setTransformOriginPoint(geometry().center());
    QSize size(target->size().toSize());

    if (photo)
        delete photo;
    photo = new QPixmap(size);
    photo->fill(Qt::transparent);

    QPainter painter(photo);
    painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
    painter.fillRect(target->rect(), Qt::transparent);
    QStyleOptionGraphicsItem style;
    //XXX: some widgets follow exposedRect viewport (e.g. QGraphicsWebView)
    style.exposedRect = target->boundingRect();
    target->paint(&painter, &style, 0);
    painter.end();

}


void ShadowFake::paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
                       QWidget *widget)
{
    Q_UNUSED(widget);
    Q_UNUSED(option);

    painter->setRenderHint(QPainter::Antialiasing);
    painter->drawPixmap(0, 0, *photo);
}

}
