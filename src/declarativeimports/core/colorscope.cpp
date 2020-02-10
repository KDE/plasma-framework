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
#include <QQuickWindow>

#include <PlasmaQuick/AppletQuickItem>

QHash<QObject *, ColorScopeAttached *> ColorScopeAttached::s_attachedScopes = QHash<QObject *, ColorScopeAttached *>();

QWeakPointer<Plasma::Theme> ColorScopeAttached::s_theme;

ColorScopeAttached::ColorScopeAttached(QObject *parent)
    : QObject(parent),
      m_inherit(false),
      m_group(Plasma::Theme::NormalColorGroup),
      m_parent(parent),
      m_actualGroup(Plasma::Theme::NormalColorGroup)
{
    m_theme = s_theme.toStrongRef();
    if (!m_theme) {
        QSharedPointer<Plasma::Theme> themePtr(new Plasma::Theme);
        s_theme = themePtr;
        m_theme = s_theme.toStrongRef();
    }

    connect(m_theme.data(), &Plasma::Theme::themeChanged, this, &ColorScopeAttached::colorsChanged);

    connect(this, &ColorScopeAttached::colorGroupChanged, this, &ColorScopeAttached::colorsChanged);

    QQuickItem *parentItem = qobject_cast<QQuickItem *>(m_parent);

    auto scopeChange = [this] () {
        findParentScope();
        checkColorGroupChanged();
    };

    if (parentItem) {
        connect(parentItem, &QQuickItem::windowChanged,
                this, scopeChange);

        connect(parentItem, &QQuickItem::parentChanged,
                this, scopeChange);
    }
    findParentScope();
}

ColorScopeAttached::~ColorScopeAttached()
{
    m_deleting = true;
    s_attachedScopes.remove(m_parent);
}

void ColorScopeAttached::setParentScope(ColorScopeAttached* parentScope)
{
    if (parentScope == m_parentScope)
        return;

    if (m_parentScope) {
        disconnect(m_parentScope.data(), &ColorScopeAttached::colorGroupChanged,
                this, &ColorScopeAttached::checkColorGroupChanged);
    }

    m_parentScope = parentScope;

    if (parentScope) {
        connect(parentScope, &ColorScopeAttached::colorGroupChanged,
                this, &ColorScopeAttached::checkColorGroupChanged);
    }
}

void ColorScopeAttached::setColorGroup(Plasma::Theme::ColorGroup group)
{
    if (m_group == group) {
        return;
    }

    m_group = group;

    checkColorGroupChanged();
}

Plasma::Theme::ColorGroup ColorScopeAttached::colorGroup() const
{
    return m_actualGroup;
}

QColor ColorScopeAttached::textColor() const
{
    return m_theme->color(Plasma::Theme::TextColor, colorGroup());
}

QColor ColorScopeAttached::highlightColor() const
{
    return m_theme->color(Plasma::Theme::HighlightColor, colorGroup());
}

QColor ColorScopeAttached::highlightedTextColor() const
{
    return m_theme->color(Plasma::Theme::HighlightedTextColor, colorGroup());
}

QColor ColorScopeAttached::backgroundColor() const
{
    return m_theme->color(Plasma::Theme::BackgroundColor, colorGroup());
}

QColor ColorScopeAttached::positiveTextColor() const
{
    return m_theme->color(Plasma::Theme::PositiveTextColor, colorGroup());
}

QColor ColorScopeAttached::neutralTextColor() const
{
    return m_theme->color(Plasma::Theme::NeutralTextColor, colorGroup());
}

QColor ColorScopeAttached::negativeTextColor() const
{
    return m_theme->color(Plasma::Theme::NegativeTextColor, colorGroup());
}

QColor ColorScopeAttached::disabledTextColor() const
{
    return m_theme->color(Plasma::Theme::DisabledTextColor, colorGroup());
}

bool ColorScopeAttached::inherit() const
{
    return m_inherit;
}

void ColorScopeAttached::setInherit(bool inherit)
{
    if (m_inherit == inherit) {
        return;
    }
    m_inherit = inherit;
    emit inheritChanged();
    checkColorGroupChanged();
}

void ColorScopeAttached::checkColorGroupChanged()
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

ColorScopeAttached *ColorScopeAttached::findParentScope()
{
    QObject *candidate = parent();

    while (candidate) {
        auto *quickCandidate = qobject_cast<QQuickItem *>(candidate);
        if (quickCandidate && quickCandidate->parentItem()) {
            candidate = quickCandidate->parentItem();
        } else {
            candidate = candidate->parent();
        }

        // Make sure AppletInterface always has a ColorScopeAttached
        ColorScopeAttached *s = static_cast<ColorScopeAttached *>(qmlAttachedPropertiesObject<ColorScope>(candidate, qobject_cast<PlasmaQuick::AppletQuickItem *>(candidate)));

        if (s && !s->m_deleting) {
            setParentScope(s);
            return s;
        }
    }

    return nullptr;
}


/////////////////////////////////////////////////////


ColorScope::ColorScope(QQuickItem *parent)
    : QQuickItem(parent)
{
    m_ownAttached = qobject_cast<ColorScopeAttached *>(qmlAttachedPropertiesObject<ColorScope>(this, true));

    connect(m_ownAttached, &ColorScopeAttached::colorGroupChanged,
            this, &ColorScope::colorGroupChanged);
    connect(m_ownAttached, &ColorScopeAttached::colorsChanged, 
            this, &ColorScope::colorsChanged);
    connect(m_ownAttached, &ColorScopeAttached::inheritChanged,
            this, &ColorScope::inheritChanged);
}

ColorScope::~ColorScope()
{}

void ColorScope::setColorGroup(Plasma::Theme::ColorGroup group)
{
    m_ownAttached->setColorGroup(group);
}

Plasma::Theme::ColorGroup ColorScope::colorGroup() const
{
    return m_ownAttached->colorGroup();
}

QColor ColorScope::textColor() const
{
    return m_ownAttached->textColor();
}

QColor ColorScope::highlightColor() const
{
    return m_ownAttached->highlightColor();
}

QColor ColorScope::highlightedTextColor() const
{
    return m_ownAttached->highlightedTextColor();
}

QColor ColorScope::backgroundColor() const
{
    return m_ownAttached->backgroundColor();
}

QColor ColorScope::positiveTextColor() const
{
    return m_ownAttached->positiveTextColor();
}

QColor ColorScope::neutralTextColor() const
{
    return m_ownAttached->neutralTextColor();
}

QColor ColorScope::negativeTextColor() const
{
    return m_ownAttached->negativeTextColor();
}

QColor ColorScope::disabledTextColor() const
{
    return m_ownAttached->disabledTextColor();
}

bool ColorScope::inherit() const
{
    return m_ownAttached->inherit();
}

void ColorScope::setInherit(bool inherit)
{
    m_ownAttached->setInherit(inherit);
}

void ColorScope::itemChange(ItemChange change, const ItemChangeData &value)
{
    if (change == QQuickItem::ItemSceneChange) {
        //we have a window: create the representations if needed
        if (value.window) {
            m_ownAttached->findParentScope();
            m_ownAttached->checkColorGroupChanged();
        }
    }

    QQuickItem::itemChange(change, value);
}

ColorScopeAttached *ColorScope::qmlAttachedProperties(QObject *object)
{
    const auto cs = ColorScopeAttached::s_attachedScopes.value(object);
    if (cs) {
        return cs;
    }

    ColorScopeAttached *s = new ColorScopeAttached(object);
    ColorScopeAttached::s_attachedScopes[object] = s;
    s->m_inherit = true;
    s->setParent(object);
    s->checkColorGroupChanged();

    return s;
}

#include "moc_colorscope.cpp"
