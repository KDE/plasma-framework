/*
    SPDX-FileCopyrightText: 2011 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2011 Artur Duque de Souza <asouza@kde.org>
    SPDX-FileCopyrightText: 2013 Sebastian Kügler <sebas@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "colorscope.h"

#include <QColor>
#include <QQmlContext>
#include <QQmlEngine>
#include <QQuickWindow>

#include <PlasmaQuick/AppletQuickItem>

QHash<QObject *, ColorScope *> ColorScope::s_attachedScopes = QHash<QObject *, ColorScope *>();

QWeakPointer<Plasma::Theme> ColorScope::s_theme;

ColorScope::ColorScope(QQuickItem *parent, QObject *parentObject)
    : QQuickItem(parent)
    , m_inherit(false)
    , m_group(Plasma::Theme::NormalColorGroup)
    , m_parent(parentObject)
    , m_actualGroup(Plasma::Theme::NormalColorGroup)
{
    m_theme = s_theme.toStrongRef();
    if (!m_theme) {
        QSharedPointer<Plasma::Theme> themePtr(new Plasma::Theme);
        s_theme = themePtr;
        m_theme = s_theme.toStrongRef();
    }

    connect(m_theme.data(), &Plasma::Theme::themeChanged, this, &ColorScope::colorsChanged);

    connect(this, &ColorScope::colorGroupChanged, this, &ColorScope::colorsChanged);

    if (parentObject && qobject_cast<QQuickItem *>(parentObject)) {
        connect(static_cast<QQuickItem *>(parentObject), &QQuickItem::windowChanged, this, [this]() {
            findParentScope();
            checkColorGroupChanged();
        });

        connect(static_cast<QQuickItem *>(parentObject), &QQuickItem::parentChanged, this, [this]() {
            findParentScope();
            checkColorGroupChanged();
        });
    } else if (parent) {
        connect(parent, &QQuickItem::parentChanged, this, &ColorScope::checkColorGroupChanged);
    }
}

ColorScope::~ColorScope()
{
    m_deleting = true;
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

void ColorScope::setParentScope(ColorScope *parentScope)
{
    if (parentScope == m_parentScope) {
        return;
    }

    if (m_parentScope) {
        disconnect(m_parentScope.data(), &ColorScope::colorGroupChanged, this, &ColorScope::checkColorGroupChanged);
    }

    m_parentScope = parentScope;

    if (parentScope) {
        connect(parentScope, &ColorScope::colorGroupChanged, this, &ColorScope::checkColorGroupChanged);
    }
}

ColorScope *ColorScope::findParentScope()
{
    QObject *candidate = parentItem();
    if (!candidate) {
        candidate = parent();
    }

    while (candidate) {
        auto *quickCandidate = qobject_cast<QQuickItem *>(candidate);
        if (quickCandidate && quickCandidate->parentItem()) {
            candidate = quickCandidate->parentItem();
        } else {
            candidate = candidate->parent();
        }

        ColorScope *s = qobject_cast<ColorScope *>(candidate);
        if (!s) {
            // Make sure AppletInterface always has a ColorScope
            s = static_cast<ColorScope *>(qmlAttachedPropertiesObject<ColorScope>(candidate, qobject_cast<PlasmaQuick::AppletQuickItem *>(candidate)));
        }
        if (s && !s->m_deleting) {
            setParentScope(s);
            return s;
        }
    }

    return nullptr;
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
    return m_theme->color(Plasma::Theme::TextColor, colorGroup());
}

QColor ColorScope::highlightColor() const
{
    return m_theme->color(Plasma::Theme::HighlightColor, colorGroup());
}

QColor ColorScope::highlightedTextColor() const
{
    return m_theme->color(Plasma::Theme::HighlightedTextColor, colorGroup());
}

QColor ColorScope::backgroundColor() const
{
    return m_theme->color(Plasma::Theme::BackgroundColor, colorGroup());
}

QColor ColorScope::positiveTextColor() const
{
    return m_theme->color(Plasma::Theme::PositiveTextColor, colorGroup());
}

QColor ColorScope::neutralTextColor() const
{
    return m_theme->color(Plasma::Theme::NeutralTextColor, colorGroup());
}

QColor ColorScope::negativeTextColor() const
{
    return m_theme->color(Plasma::Theme::NegativeTextColor, colorGroup());
}

QColor ColorScope::disabledTextColor() const
{
    return m_theme->color(Plasma::Theme::DisabledTextColor, colorGroup());
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
    Q_EMIT inheritChanged();
    checkColorGroupChanged();
}

void ColorScope::itemChange(ItemChange change, const ItemChangeData &value)
{
    if (change == QQuickItem::ItemSceneChange) {
        // we have a window: create the representations if needed
        if (value.window) {
            findParentScope();
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
