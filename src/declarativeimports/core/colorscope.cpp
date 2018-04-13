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
#include <QColor>

QHash<QObject *, ColorScope *> ColorScope::s_attachedScopes = QHash<QObject *, ColorScope *>();

ColorScope::ColorScope(QQuickItem *parent, QObject *parentObject)
    : QQuickItem(parent),
      m_inherit(false),
      m_group(Plasma::Theme::NormalColorGroup),
      m_parent(parentObject),
      m_actualGroup(Plasma::Theme::NormalColorGroup)
{
    connect(&m_theme, &Plasma::Theme::themeChanged, this, &ColorScope::colorsChanged);
    connect(this, &ColorScope::colorGroupChanged, this, &ColorScope::colorsChanged);

    QQuickItem *parentItem = qobject_cast<QQuickItem *>(parentObject);
    if (parentItem) {
        connect(parentItem, &QQuickItem::parentChanged, this, &ColorScope::checkColorGroupChanged);
    } else if (m_parent) {
        m_parent->installEventFilter(this);
    }
}

ColorScope::~ColorScope()
{
    s_attachedScopes.remove(m_parent);
}

ColorScope *ColorScope::qmlAttachedProperties(QObject *object)
{
    const auto cs = s_attachedScopes.value(object);
    if (cs) {
        return cs;
    }

    ColorScope *s = new ColorScope(nullptr, object);
    s_attachedScopes[object] = s;
    s->m_inherit = true;
    s->setParent(object);
    s->checkColorGroupChanged();

    return s;
}

bool ColorScope::eventFilter(QObject* watched, QEvent* event)
{
    Q_ASSERT(watched == m_parent && !qobject_cast<QQuickItem *>(watched));
    if (event->type() == QEvent::ParentChange) {
        checkColorGroupChanged();
    }
    return QQuickItem::eventFilter(watched, event);
}

void ColorScope::setParentScope(ColorScope* parentScope)
{
    if (parentScope == m_parentScope)
        return;

    if (m_parentScope) {
        disconnect(m_parentScope.data(), &ColorScope::colorGroupChanged,
                this, &ColorScope::checkColorGroupChanged);
    }

    m_parentScope = parentScope;

    if (parentScope) {
        connect(parentScope, &ColorScope::colorGroupChanged,
                this, &ColorScope::checkColorGroupChanged);
    }
}

ColorScope *ColorScope::findParentScope()
{
    QObject *p = nullptr;
    if (m_parent) {
        QQuickItem *gp = qobject_cast<QQuickItem *>(m_parent);
        if (gp) {
            p = gp->parentItem();
        } else {
            p = m_parent->parent();
        }
    }

    if (!p || !m_parent) {
        setParentScope(nullptr);
        return nullptr;
    }

    ColorScope *c = qobject_cast<ColorScope *>(p);
    if (!c) {
        c = qmlAttachedProperties(p);
    }

    setParentScope(c);

    return m_parentScope;
}

void ColorScope::setColorGroup(Plasma::Theme::ColorGroup group)
{
    if (m_group == group) {
        return;
    }

    m_group = group;

    checkColorGroupChanged();
}

Plasma::Theme::ColorGroup ColorScope::colorGroup() const
{
    return m_actualGroup;
}

QColor ColorScope::textColor() const
{
    return m_theme.color(Plasma::Theme::TextColor, colorGroup());
}

QColor ColorScope::highlightColor() const
{
    return m_theme.color(Plasma::Theme::HighlightColor, colorGroup());
}

QColor ColorScope::highlightedTextColor() const
{
    return m_theme.color(Plasma::Theme::HighlightedTextColor, colorGroup());
}

QColor ColorScope::backgroundColor() const
{
    return m_theme.color(Plasma::Theme::BackgroundColor, colorGroup());
}

QColor ColorScope::positiveTextColor() const
{
    return m_theme.color(Plasma::Theme::PositiveTextColor, colorGroup());
}

QColor ColorScope::neutralTextColor() const
{
    return m_theme.color(Plasma::Theme::NeutralTextColor, colorGroup());
}

QColor ColorScope::negativeTextColor() const
{
    return m_theme.color(Plasma::Theme::NegativeTextColor, colorGroup());
}

bool ColorScope::inherit() const
{
    return m_inherit;
}

void ColorScope::setInherit(bool inherit)
{
    if (m_inherit == inherit) {
        return;
    }
    m_inherit = inherit;
    emit inheritChanged();
    checkColorGroupChanged();
}

void ColorScope::itemChange(ItemChange change, const ItemChangeData &value)
{
    if (change == QQuickItem::ItemSceneChange) {
        //we have a window: create the representations if needed
        if (value.window) {
            checkColorGroupChanged();
        }
    }

    QQuickItem::itemChange(change, value);
}

void ColorScope::checkColorGroupChanged()
{
    const auto last = m_actualGroup;
    if (m_inherit) {
        findParentScope();
        m_actualGroup = m_parentScope ? m_parentScope->colorGroup() : m_group;
    } else {
        m_actualGroup = m_group;
    }

    if (m_actualGroup != last) {
        Q_EMIT colorGroupChanged();
    }
}

#include "moc_colorscope.cpp"
