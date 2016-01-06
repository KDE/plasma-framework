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

QHash<QObject *, ColorScope *> ColorScope::s_attachedScopes = QHash<QObject *, ColorScope *>();

ColorScope::ColorScope(QQuickItem *parent)
    : QQuickItem(parent),
      m_inherit(false),
      m_group(Plasma::Theme::NormalColorGroup)
{
    connect(&m_theme, &Plasma::Theme::themeChanged, this, &ColorScope::colorsChanged);

    if (parent) {
        connect(parent, &QQuickItem::parentChanged, this, &ColorScope::colorGroupChanged);
        connect(parent, &QQuickItem::parentChanged, this, &ColorScope::colorsChanged);
    }
}

ColorScope::~ColorScope()
{
    s_attachedScopes.remove(parentItem());
}

ColorScope *ColorScope::qmlAttachedProperties(QObject *object)
{
    QQuickItem *qp = qobject_cast<QQuickItem *>(object);

    if (ColorScope::s_attachedScopes.contains(object)) {
        return s_attachedScopes.value(object);
    }

    ColorScope *s = new ColorScope(qp);
    s_attachedScopes[object] = s;

    if (!qp) {
        s->setParent(object);
    }
    s->m_inherit = true;
    return s;
}

ColorScope *ColorScope::findParentScope() const
{
    QQuickItem *p = 0;
    if (parentItem()) {
        p = parentItem()->parentItem();
    }

    if (!p || !parentItem()) {
        if (m_parentScope) {
            disconnect(m_parentScope.data(), &ColorScope::colorGroupChanged,
                    this, &ColorScope::colorGroupChanged);
            disconnect(m_parentScope.data(), &ColorScope::colorsChanged,
                    this, &ColorScope::colorsChanged);
        }
        return 0;
    }

    ColorScope *c = qobject_cast<ColorScope *>(p);
    if (!c) {
        c = qmlAttachedProperties(p);
    }

    if (c != m_parentScope) {
        if (m_parentScope) {
            disconnect(m_parentScope.data(), &ColorScope::colorGroupChanged,
                    this, &ColorScope::colorGroupChanged);
            disconnect(m_parentScope.data(), &ColorScope::colorsChanged,
                    this, &ColorScope::colorsChanged);
        }
        connect(c, &ColorScope::colorGroupChanged,
                this, &ColorScope::colorGroupChanged);
        connect(c, &ColorScope::colorsChanged,
                this, &ColorScope::colorsChanged);
        //HACK
        const_cast<ColorScope *>(this)->m_parentScope = c;
    }


    return m_parentScope;
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
    if (m_inherit) {
        ColorScope *s = findParentScope();
        if (s) {
            return s->colorGroup();
        }
    }
    return m_group;
}

QColor ColorScope::textColor() const
{
    if (m_inherit) {
        ColorScope *s = findParentScope();
        if (s) {
            return s->textColor();
        }
    }
    return m_theme.color(Plasma::Theme::TextColor, m_group);
}

QColor ColorScope::highlightColor() const
{
    if (m_inherit) {
        ColorScope *s = findParentScope();
        if (s) {
            return s->highlightColor();
        }
    }
    return m_theme.color(Plasma::Theme::HighlightColor, m_group);
}

QColor ColorScope::backgroundColor() const
{
    if (m_inherit) {
        ColorScope *s = findParentScope();
        if (s) {
            return s->backgroundColor();
        }
    }
    return m_theme.color(Plasma::Theme::BackgroundColor, m_group);
}

void ColorScope::itemChange(ItemChange change, const ItemChangeData &value)
{
    if (change == QQuickItem::ItemSceneChange) {
        //we have a window: create the representations if needed
        if (value.window) {
            emit colorGroupChanged();
            emit colorsChanged();
        }
    }

    QQuickItem::itemChange(change, value);
}

bool ColorScope::event(QEvent *event)
{
    if (event->type() == QEvent::ParentChange) {
        emit colorGroupChanged();
        emit colorsChanged();
    }

    return QQuickItem::event(event);
}

#include "moc_colorscope.cpp"
