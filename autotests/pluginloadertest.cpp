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
    : m_buildonly(false)
{
}

void PluginTest::listAppletCategories()
{
    const QStringList cats = Plasma::PluginLoader::self()->listAppletCategories();
    qDebug() << "Categories" << cats;
    QVERIFY(cats.count() > 0 || m_buildonly);
}

void PluginTest::listContainmentActions()
{
    const QVector<KPluginMetaData> plugins = Plasma::PluginLoader::self()->listContainmentActionsMetaData(QStringLiteral("plasma-shell"));
    qDebug() << "Categories: " << plugins.count();
    // QVERIFY(plugins.count() > 0 || m_buildonly);
}

void PluginTest::listContainmentsOfType()
{
    const QList<KPluginMetaData> pluginsMetaData = Plasma::PluginLoader::listContainmentsMetaDataOfType(QStringLiteral("Desktop"));
    qDebug() << "Desktop Containments MetaData: " << pluginsMetaData.count();
    QVERIFY(pluginsMetaData.count() > 0 || m_buildonly);
}

#include "moc_pluginloadertest.cpp"
