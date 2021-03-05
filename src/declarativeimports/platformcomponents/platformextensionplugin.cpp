/*
    SPDX-FileCopyrightText: 2013 Ivan Cukic <ivan.cukic(at)kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <QDebug>
#include <QQmlExtensionPlugin>
#include <QtQml>

#include "application.h"
#include "icondialog.h"

class PlatformComponentsPlugin : public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QQmlExtensionInterface")

public:
    PlatformComponentsPlugin(QObject *parent = nullptr)
        : QQmlExtensionPlugin(parent)
    {
    }

    void registerTypes(const char *uri) override
    {
        Q_ASSERT(QLatin1String(uri) == QLatin1String("org.kde.plasma.platformcomponents"));

        qmlRegisterType<Application>(uri, 2, 0, "Application");
        qmlRegisterType<IconDialog>(uri, 2, 0, "IconDialog");
    }
};

#include "platformextensionplugin.moc"
