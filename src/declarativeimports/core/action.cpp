/*
    SPDX-FileCopyrightText: 2023 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "action.h"
#include <QMenu>

IconGroup::IconGroup(ActionExtension *parent)
    : QObject(parent)
    , m_action(parent->action())
{
}

IconGroup::~IconGroup()
{
}

void IconGroup::setName(const QString &name)
{
    if (name == m_name) {
        return;
    }

    m_name = name;
    m_action->setIcon(QIcon::fromTheme(name));
    Q_EMIT nameChanged();
}

QString IconGroup::name() const
{
    return m_name;
}

void IconGroup::setIcon(const QIcon &icon)
{
    m_action->setIcon(icon);
    Q_EMIT iconChanged();
}

QIcon IconGroup::icon() const
{
    return m_action->icon();
}

ActionExtension::ActionExtension(QObject *parent)
    : QObject(parent)
    , m_action(qobject_cast<QAction *>(parent))
    , m_iconGroup(new IconGroup(this))
{
}

ActionExtension::~ActionExtension()
{
}

bool ActionExtension::isSeparator() const
{
    return m_action->isSeparator();
}

void ActionExtension::setSeparator(bool separator)
{
    if (separator == m_action->isSeparator()) {
        return;
    }

    m_action->setSeparator(separator);

    Q_EMIT isSeparatorChanged();
}

void ActionExtension::setActionGroup(QActionGroup *group)
{
    if (group == m_action->actionGroup()) {
        return;
    }

    m_action->setActionGroup(group);
    Q_EMIT actionGroupChanged();
}

QActionGroup *ActionExtension::actionGroup() const
{
    return m_action->actionGroup();
}

static QKeySequence variantToKeySequence(const QVariant &var)
{
    if (var.metaType().id() == QMetaType::Int)
        return QKeySequence(static_cast<QKeySequence::StandardKey>(var.toInt()));
    return QKeySequence::fromString(var.toString());
}

QVariant ActionExtension::shortcut() const
{
    return m_action->shortcut();
}

void ActionExtension::setShortcut(const QVariant &var)
{
    const QKeySequence seq = variantToKeySequence(var);
    if (seq == m_action->shortcut()) {
        return;
    }
    m_action->setShortcut(seq);
    Q_EMIT shortcutChanged();
}

void ActionExtension::setMenu(QMenu *menu)
{
    if (menu == m_action->menu()) {
        return;
    }

    m_action->setMenu(menu);
    Q_EMIT menuChanged();
}

QMenu *ActionExtension::menu()
{
    return m_action->menu();
}

QAction *ActionExtension::action() const
{
    return m_action;
}

#include "moc_action.cpp"
