/*
    SPDX-FileCopyrightText: 2009 Alan Alpert <alan.alpert@nokia.com>
    SPDX-FileCopyrightText: 2010 Ménard Alexis <menard@kde.org>
    SPDX-FileCopyrightText: 2022 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "../../scriptengines/qml/plasmoid/appletinterface.h"
#include "../../scriptengines/qml/plasmoid/containmentinterface.h"
#include "../../scriptengines/qml/plasmoid/wallpaperinterface.h"
#include <QQmlExtensionPlugin>

class PlasmoidPlugin : public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QQmlExtensionInterface")

public:
    void registerTypes(const char *uri) override
    {
        qmlRegisterUncreatableType<AppletInterface>(uri, 2, 0, "Plasmoid", QStringLiteral("Do not create objects of type Plasmoid"));
        qmlRegisterUncreatableType<ContainmentInterface>(uri, 2, 0, "Containment", QStringLiteral("Do not create objects of type Containment"));
        qmlRegisterUncreatableType<WallpaperInterface>(uri, 2, 0, "Wallpaper", QStringLiteral("Do not create objects of type Wallpaper"));
    }
};

#include "plasmoidplugin.moc"
