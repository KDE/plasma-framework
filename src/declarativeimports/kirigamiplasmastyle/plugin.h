/*
    SPDX-FileCopyrightText: 2017 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2021 Arjen Hiemstra <ahiemstra@heimr.nl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef PLUGIN_H
#define PLUGIN_H

#include <Kirigami/KirigamiPluginFactory>
#include <QObject>

class Plugin : public Kirigami::KirigamiPluginFactoryV2
{
    Q_OBJECT

    Q_PLUGIN_METADATA(IID "org.kde.kirigami.KirigamiPluginFactory" FILE "plugin.json")

    Q_INTERFACES(Kirigami::KirigamiPluginFactory)

public:
    explicit Plugin(QObject *parent = nullptr);
    ~Plugin() override;

    Kirigami::PlatformTheme *createPlatformTheme(QObject *parent) override;
    Kirigami::Units *createUnits(QObject *parent) override;
};

#endif // PLUGIN_H
