/*
 *   Copyright 2014 Marco Martin <mart@kde.org>
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

#include "visualmargins.h"
#include <QTimer>
VisualMargins::VisualMargins(QObject *parent)
    : QObject(parent)
{
    QQuickItem *item = qobject_cast<QQuickItem *>(parent);

    if (item) {
        connect(item, &QQuickItem::windowChanged, this, &VisualMargins::connectMarginObject);
        connectMarginObject(item->window());
    }
}

VisualMargins::~VisualMargins()
{
}

void VisualMargins::connectMarginObject(QQuickWindow *window)
{
    if (m_marginsObject) {
        disconnect(m_marginsObject, SIGNAL(marginsChanged()), this, SIGNAL(marginsChanged()));
    }

    if (!window) {
        return;
    }

    m_marginsObject = window->property("margins").value<QObject *>();

    if (m_marginsObject) {
        connect(m_marginsObject, SIGNAL(marginsChanged()), this, SIGNAL(marginsChanged()));
    }

    emit marginsChanged();
}

qreal VisualMargins::marginProperty(const QString &prop) const
{
    if (!m_marginsObject) {
        return 0;
    }

    return m_marginsObject->property(prop.toLatin1()).value<qreal>();
}

qreal VisualMargins::left() const
{
    return marginProperty("left");
}

qreal VisualMargins::top() const
{
    return marginProperty("left");
}

qreal VisualMargins::right() const
{
    return marginProperty("left");
}

qreal VisualMargins::bottom() const
{
    return marginProperty("left");
}

#include "moc_visualmargins.cpp"
