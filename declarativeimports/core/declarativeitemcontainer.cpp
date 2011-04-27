/***************************************************************************
 *   Copyright 2011 Marco Martin <mart@kde.org>                            *
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

#include "declarativeitemcontainer_p.h"

DeclarativeItemContainer::DeclarativeItemContainer(QGraphicsItem *parent)
    : QGraphicsWidget(parent)
{
}

DeclarativeItemContainer::~DeclarativeItemContainer()
{
}

void DeclarativeItemContainer::setDeclarativeItem(QDeclarativeItem *item, bool reparent)
{
    if (m_declarativeItem) {
        m_declarativeItem.data()->removeSceneEventFilter(this);
    }
    m_declarativeItem = item;
    if (reparent) {
        static_cast<QGraphicsItem *>(item)->setParentItem(this);
    }
    setMinimumWidth(item->implicitWidth());
    setMinimumHeight(item->implicitHeight());
    resize(item->width(), item->height());
    item->installSceneEventFilter(this);
}

QDeclarativeItem *DeclarativeItemContainer::declarativeItem() const
{
    return m_declarativeItem.data();
}

void DeclarativeItemContainer::resizeEvent(QGraphicsSceneResizeEvent *event)
{
    if (m_declarativeItem) {
        m_declarativeItem.data()->setProperty("width", event->newSize().width());
        m_declarativeItem.data()->setProperty("height", event->newSize().height());
    }
}

bool DeclarativeItemContainer::sceneEventFilter(QGraphicsItem *watched, QEvent *event)
{
    if (event->type() == QEvent::GraphicsSceneResize) {
        resize(watched->boundingRect().size());
    }

    return QGraphicsWidget::sceneEventFilter(watched, event);
}
