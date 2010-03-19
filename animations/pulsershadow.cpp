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
#include <QDebug>
static const int RECURSION_MAX = 20;

namespace Plasma
{

ShadowFake::ShadowFake(QGraphicsItem *parent)
    : QGraphicsWidget(parent),
      m_iconBig(false),
      stack(0),
      m_target(0)
{
}

ShadowFake::~ShadowFake()
{
}

void ShadowFake::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(widget);

    painter->setRenderHint(QPainter::Antialiasing);
    painter->drawPixmap(option->exposedRect, m_photo, option->exposedRect);
}

void ShadowFake::paintSubChildren(QPainter *painter,
                                  const QStyleOptionGraphicsItem *option,
                                  QGraphicsItem *target)
{
    ++stack;
    QList<QGraphicsItem *> list = target->childItems();
    QGraphicsItem *tmp;
    if (list.size() > 0) {
        for (int i = 0; i < list.size(); ++i) {
            tmp = list.value(i);

            if ((tmp->childItems().size() > 0) && (stack < RECURSION_MAX)) {
                paintSubChildren(painter, option, tmp);
            }

            if (tmp->isVisible()) {
                tmp->paint(painter, option, 0);
            }
        }
    }
    --stack;
}

void ShadowFake::setTarget(QGraphicsWidget *target)
{
    stack = 0;
    m_target = target;
    setParentItem(target);
    QSize size(target->size().toSize());
    m_iconBig = false;

    if (m_target->property("iconRepresentation").isValid()) {
        m_iconBig = true;
        m_photo = QPixmap::fromImage(
            m_target->property("iconRepresentation").value<QImage>());
        if ((m_photo.height() > 0) && (m_photo.width() > 0)) {
            resize(m_photo.size());
            setTransformOriginPoint(target->geometry().center());
            return;
        }
    }

    resize(target->size());

    m_photo = QPixmap(size);
    m_photo.fill(Qt::transparent);

    QPainter painter(&m_photo);
    painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
    painter.fillRect(target->rect(), Qt::transparent);

    QStyleOptionGraphicsItem style;
    style.exposedRect = target->boundingRect();
    style.rect = target->rect().toRect();

    target->paint(&painter, &style, 0);
    paintSubChildren(&painter, &style, target);
    painter.end();
    setTransformOriginPoint(geometry().center());
}

QGraphicsWidget *ShadowFake::target() const
{
    return m_target;
}


bool ShadowFake::isIconBigger()
{
    return m_iconBig;
}


}

