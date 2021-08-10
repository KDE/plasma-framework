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

Kirigami::Units *Plugin::createUnits(QObject *parent)
{
    Q_ASSERT(parent);
    return new Units(parent);
}

#include "moc_plugin.cpp"
