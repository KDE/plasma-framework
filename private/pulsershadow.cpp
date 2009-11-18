/*********************************************************************/
/*                                                                   */
/* Copyright(C)2009 Adenilson Cavalcanti adenilson.silva@idnt.org.br */
/*                                                                   */
/* This program is free software; you can redistribute it and/or     */
/* modify it under the terms of the GNU General Public License       */
/* as published by the Free Software Foundation; either version 2    */
/* of the License, or (at your option) any later version.            */
/*                                                                   */
/* This program is distributed in the hope that it will be useful,   */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of    */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the     */
/* GNU General Public License for more details.                      */
/*                                                                   */
/* You should have received a copy of the GNU General Public License */
/* along with this program; if not, write to the Free Software       */
/* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA     */
/* 02110-1301, USA.                                                  */
/*********************************************************************/

#include "pulsershadow_p.h"
#include <QPainter>
#include <QImage>
#include <QPixmap>
#include <QDebug>

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
    target->paint(&painter, 0, 0);
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
