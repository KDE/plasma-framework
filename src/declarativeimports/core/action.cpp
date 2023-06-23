/*
    SPDX-FileCopyrightText: 2023 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "action.h"

IconGroup::IconGroup(QObject *parent)
    : QObject(parent)
{
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

#include "moc_action.cpp"
