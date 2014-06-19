/***************************************************************************
 *   Copyright 2011 Marco Martin <mart@kde.org>                            *
 *   Copyright 2011 Artur Duque de Souza <asouza@kde.org>                  *
 *   Copyright 2013 Sebastian Kügler <sebas@kde.org>                       *
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

#include "colorscope.h"

#include <QQmlContext>
#include <QQmlEngine>


ColorScope *ColorScope::s_colorScope = 0;


ColorScope::ColorScope(QQuickItem *parent)
    : QQuickItem(parent),
      m_group(Plasma::Theme::NormalColorGroup)
{
}

ColorScope::~ColorScope()
{

}

ColorScope *ColorScope::qmlAttachedProperties(QObject *object)
{
    QObject *p = object;
    while (p) {
        ColorScope *c = qobject_cast<ColorScope *>(p);
        if (c) {
            return c;
        }
        //this will be parent() for qobjects, parentItem for QQuickItems
        QQuickItem *item = qobject_cast<QQuickItem *>(p);
        if (item) {
            p = item->parentItem();
        } else {
            p = p->parent();
        }
    }

    if (!s_colorScope) {
        s_colorScope = new ColorScope;
    }

    return s_colorScope;
}



void ColorScope::setColorGroup(Plasma::Theme::ColorGroup group)
{
    if (m_group == group) {
        return;
    }

    m_group = group;

    emit colorGroupChanged();
    emit colorsChanged();
}

Plasma::Theme::ColorGroup ColorScope::colorGroup() const
{
    return m_group;
}

QColor ColorScope::textColor() const
{
    return m_theme.color(Plasma::Theme::TextColor, m_group);
}

QColor ColorScope::highlightColor() const
{
    return m_theme.color(Plasma::Theme::BackgroundColor, m_group);
}

QColor ColorScope::backgroundColor() const
{
    return m_theme.color(Plasma::Theme::BackgroundColor, m_group);
}

#include "moc_colorscope.cpp"
