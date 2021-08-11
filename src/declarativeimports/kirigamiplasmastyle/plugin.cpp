/*
    SPDX-FileCopyrightText: 2017 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2021 Arjen Hiemstra <ahiemstra@heimr.nl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "plugin.h"
#include "units.h"

Plugin::Plugin(QObject *parent)
    : Kirigami::KirigamiPluginFactoryV2(parent)
{
}

Plugin::~Plugin() = default;

Kirigami::PlatformTheme *Plugin::createPlatformTheme(QObject *parent)
{
    Q_UNUSED(parent);
    // TODO: Implement a proper C++ version of PlatformTheme. This relies on fallback
    // behaviour in Kirigami to load the Theme.qml file.
    return nullptr;
}

Kirigami::Units *Plugin::createUnits(QObject *parent)
{
    Q_ASSERT(parent);
    return new Units(parent);
}

#include "moc_plugin.cpp"
