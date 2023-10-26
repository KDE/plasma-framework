/*
    SPDX-FileCopyrightText: 2017 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2021 Arjen Hiemstra <ahiemstra@heimr.nl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef PLUGIN_H
#define PLUGIN_H

#include <Kirigami/Platform/PlatformPluginFactory>
#include <QObject>

class Plugin : public Kirigami::Platform::PlatformPluginFactory
{
    Q_OBJECT

    Q_PLUGIN_METADATA(IID PlatformPluginFactory_iid FILE "plugin.json")

    Q_INTERFACES(Kirigami::Platform::PlatformPluginFactory)

public:
    explicit Plugin(QObject *parent = nullptr);
    ~Plugin() override;

    Kirigami::Platform::PlatformTheme *createPlatformTheme(QObject *parent) override;
    Kirigami::Platform::Units *createUnits(QObject *parent) override;
};

#endif // PLUGIN_H
