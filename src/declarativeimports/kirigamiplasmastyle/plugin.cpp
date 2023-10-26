/*
    SPDX-FileCopyrightText: 2017 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2021 Arjen Hiemstra <ahiemstra@heimr.nl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "plugin.h"
#include "plasmatheme.h"
#include "units.h"

Plugin::Plugin(QObject *parent)
    : Kirigami::Platform::PlatformPluginFactory(parent)
{
}

Plugin::~Plugin() = default;

Kirigami::Platform::PlatformTheme *Plugin::createPlatformTheme(QObject *parent)
{
    Q_UNUSED(parent);
    // TODO: Implement a proper C++ version of PlatformTheme. This relies on fallback
    // behaviour in Kirigami to load the Theme.qml file.
    return new PlasmaTheme(parent);
}

Kirigami::Platform::Units *Plugin::createUnits(QObject *parent)
{
    Q_ASSERT(parent);
    return new Units(parent);
}

#include "moc_plugin.cpp"
