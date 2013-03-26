/***************************************************************************
 *   Copyright 2011 Viranch Mehta <viranch.mehta@gmail.com>                *
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

#include "qmenuitem.h"

QMenuItem::QMenuItem(QQuickItem *parent)
    : QQuickItem(parent),
    m_action(0)

{
    setAction(new QAction(this));
    connect(m_action, &QAction::triggered, this, &QMenuItem::clicked);
}

QAction* QMenuItem::action() const
{
    return m_action;
}

void QMenuItem::setAction(QAction* a)
{
    if (m_action != a) {
        m_action = a;
        connect(m_action, &QAction::changed, this, &QMenuItem::textChanged);
        connect(m_action, &QAction::changed, this, &QMenuItem::textChanged);
        emit actionChanged();
    }
}

QIcon QMenuItem::icon() const
{
    return m_action->icon();
}

void QMenuItem::setIcon(const QIcon& i)
{
    m_action->setIcon(i);
    emit iconChanged();
}

bool QMenuItem::separator() const
{
    return m_action->isSeparator();
}

void QMenuItem::setSeparator(bool s)
{
    m_action->setSeparator(s);
}

QString QMenuItem::text() const
{
    return m_action->text();
}

void QMenuItem::setText(const QString& t)
{
    if (m_action->text() != t) {
        m_action->setText(t);
        // signal comes from m_action
    }
}


#include "qmenuitem.moc"

