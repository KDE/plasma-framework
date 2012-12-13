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

    setMinimumWidth(item->implicitWidth());
    setMinimumHeight(item->implicitHeight());

    resize(item->width(), item->height());

    m_declarativeItem = item;
    if (reparent) {
        static_cast<QGraphicsItem *>(item)->setParentItem(this);
    }

    connect(m_declarativeItem.data(), SIGNAL(widthChanged()), this, SLOT(widthChanged()));
    connect(m_declarativeItem.data(), SIGNAL(heightChanged()), this, SLOT(heightChanged()));

    qreal minimumWidth = -1;
    qreal minimumHeight = -1;
    qreal maximumWidth = -1;
    qreal maximumHeight = -1;
    qreal preferredWidth = -1;
    qreal preferredHeight = -1;


    if (item->metaObject()->indexOfProperty("minimumWidth") >= 0 ) {
        minimumWidth = item->property("minimumWidth").toReal();
        QObject::connect(item, SIGNAL(minimumWidthChanged()), this, SLOT(minimumWidthChanged()));
    }

    if (item->metaObject()->indexOfProperty("minimumHeight") >= 0 ) {
        minimumHeight = item->property("minimumHeight").toReal();
        QObject::connect(item, SIGNAL(minimumHeightChanged()), this, SLOT(minimumHeightChanged()));
    }

    if (item->metaObject()->indexOfProperty("maximumWidth") >= 0 ) {
        maximumWidth = item->property("maximumWidth").toReal();
        QObject::connect(item, SIGNAL(maximumWidthChanged()), this, SLOT(maximumWidthChanged()));
    }

    if (item->metaObject()->indexOfProperty("maximumHeight") >= 0 ) {
        maximumHeight = item->property("maximumHeight").toReal();
        QObject::connect(item, SIGNAL(maximumHeightChanged()), this, SLOT(maximumHeightChanged()));
    }

    if (item->metaObject()->indexOfProperty("preferredWidth") >= 0 ) {
        preferredWidth = item->property("preferredWidth").toReal();
        QObject::connect(item, SIGNAL(preferredWidthChanged()), this, SLOT(preferredWidthChanged()));
    }

    if (item->metaObject()->indexOfProperty("preferredHeight") >= 0 ) {
        preferredHeight = item->property("preferredHeight").toReal();
        QObject::connect(item, SIGNAL(preferredHeightChanged()), this, SLOT(preferredHeightChanged()));
    }

    if (minimumWidth > 0 && minimumHeight > 0) {
        setMinimumSize(minimumWidth, minimumHeight);
    } else {
        setMinimumSize(-1, -1);
    }

    if (maximumWidth > 0 && maximumHeight > 0) {
        setMaximumSize(maximumWidth, maximumHeight);
    } else {
        setMaximumSize(-1, -1);
    }

    if (preferredWidth > 0 && preferredHeight > 0) {
        setPreferredSize(preferredWidth, preferredHeight);
    } else {
        setPreferredSize(-1, -1);
    }
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
    qreal minimumWidth = m_declarativeItem.data()->property("minimumWidth").toReal();
    setMinimumWidth(minimumWidth);
}

void DeclarativeItemContainer::minimumHeightChanged()
{
    qreal minimumHeight = m_declarativeItem.data()->property("minimumHeight").toReal();
    setMinimumHeight(minimumHeight);
}

void DeclarativeItemContainer::maximumWidthChanged()
{
    qreal maximumWidth = m_declarativeItem.data()->property("maximumWidth").toReal();
    setMaximumWidth(maximumWidth);
}

void DeclarativeItemContainer::maximumHeightChanged()
{
    qreal maximumHeight = m_declarativeItem.data()->property("maximumHeight").toReal();
    setMaximumHeight(maximumHeight);
}

void DeclarativeItemContainer::preferredWidthChanged()
{
    qreal preferredWidth = m_declarativeItem.data()->property("preferredWidth").toReal();
    setPreferredWidth(preferredWidth);
}

void DeclarativeItemContainer::preferredHeightChanged()
{
    qreal preferredHeight = m_declarativeItem.data()->property("preferredHeight").toReal();
    setPreferredHeight(preferredHeight);
}

#include "declarativeitemcontainer_p.moc"
