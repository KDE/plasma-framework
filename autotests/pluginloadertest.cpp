/*
    SPDX-FileCopyrightText: 2013 Sebastian Kügler <sebas@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "pluginloadertest.h"

#include <QDebug>
#include <QSignalSpy>
#include <qtest.h>

#include <KPluginMetaData>

#include <plasma/pluginloader.h>

QTEST_MAIN(PluginTest)

PluginTest::PluginTest()
{
    // To pick up the simpelcontianment dummy package
    qputenv("XDG_DATA_DIRS", QFINDTESTDATA("data/").toLocal8Bit().constData());
}

void PluginTest::listContainmentActions()
{
    const QList<KPluginMetaData> plugins = Plasma::PluginLoader::self()->listContainmentActionsMetaData(QStringLiteral("plasma-shell"));
    const bool pluginFound = std::any_of(plugins.begin(), plugins.end(), [](const KPluginMetaData &data) {
        return data.pluginId() == QLatin1String("dummycontainmentaction");
    });
    QVERIFY(pluginFound);
}

void PluginTest::listContainmentsOfType()
{
    const QList<KPluginMetaData> plugins = Plasma::PluginLoader::listContainmentsMetaDataOfType(QStringLiteral("Desktop"));
    const bool pluginFound = std::any_of(plugins.begin(), plugins.end(), [](const KPluginMetaData &data) {
        return data.pluginId() == QLatin1String("simplecontainment");
    });
    QVERIFY(pluginFound);
}

#include "moc_pluginloadertest.cpp"
