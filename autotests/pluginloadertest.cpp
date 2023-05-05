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

void PluginTest::listAppletCategories()
{
    const QStringList cats = Plasma::PluginLoader::self()->listAppletCategories();
    qDebug() << "Categories" << cats;
    QVERIFY(cats.count() > 0);
    QVERIFY(cats.contains(QLatin1String("System Information")));
}

void PluginTest::listContainmentActions()
{
    const QVector<KPluginMetaData> plugins = Plasma::PluginLoader::self()->listContainmentActionsMetaData(QStringLiteral("plasma-shell"));
    qDebug() << "Categories: " << plugins;
    // QVERIFY(plugins.count() > 0);
}

void PluginTest::listContainmentsOfType()
{
    const QList<KPluginMetaData> pluginsMetaData = Plasma::PluginLoader::listContainmentsMetaDataOfType(QStringLiteral("Desktop"));
    QVERIFY(pluginsMetaData.count() > 0);
}

#include "moc_pluginloadertest.cpp"
