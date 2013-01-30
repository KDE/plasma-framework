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

#include <KDebug>

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
        disconnect(m_declarativeItem.data(), 0, this, 0);
    }
    m_declarativeItem = item;
    if (reparent) {
        static_cast<QGraphicsItem *>(item)->setParentItem(this);
    }
    setMinimumWidth(item->implicitWidth());
    setMinimumHeight(item->implicitHeight());
    resize(item->width(), item->height());
    connect(m_declarativeItem.data(), SIGNAL(widthChanged()), this, SLOT(widthChanged()));
    connect(m_declarativeItem.data(), SIGNAL(heightChanged()), this, SLOT(heightChanged()));

    if (m_declarativeItem.data()->metaObject()->indexOfProperty("minimumWidth") >= 0) {
        QObject::connect(m_declarativeItem.data(), SIGNAL(minimumWidthChanged()), this, SLOT(minimumWidthChanged()));
    }

    if (m_declarativeItem.data()->metaObject()->indexOfProperty("minimumHeight") >= 0) {
        QObject::connect(m_declarativeItem.data(), SIGNAL(minimumHeightChanged()), this, SLOT(minimumHeightChanged()));
    }
    minimumWidthChanged();
    minimumHeightChanged();
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

void DeclarativeItemContainer::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    event->ignore();
}

void DeclarativeItemContainer::widthChanged()
{
    if (!m_declarativeItem) {
        return;
    }

    QSizeF newSize(size());
    newSize.setWidth(m_declarativeItem.data()->width());
    resize(newSize);
}

void DeclarativeItemContainer::heightChanged()
{
    if (!m_declarativeItem) {
        return;
    }

    QSizeF newSize(size());
    newSize.setHeight(m_declarativeItem.data()->height());
    resize(newSize);
}

void DeclarativeItemContainer::minimumWidthChanged()
{
    if (!m_declarativeItem) {
        return;
    }

    qreal minimumWidth = m_declarativeItem.data()->property("minimumWidth").toReal();
    setMinimumWidth(minimumWidth);
}

void DeclarativeItemContainer::minimumHeightChanged()
{
    if (!m_declarativeItem) {
        return;
    }

    qreal minimumHeight = m_declarativeItem.data()->property("minimumHeight").toReal();
    setMinimumHeight(minimumHeight);
}


#include "declarativeitemcontainer_p.moc"
