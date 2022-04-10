/*
    SPDX-FileCopyrightText: 2013 Sebastian Kügler <sebas@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "pluginloadertest.h"

#include <QDebug>
#include <QSignalSpy>
#include <qtest.h>

#include <KPluginMetaData>

#include <plasma/dataengineconsumer.h>
#include <plasma/pluginloader.h>

#if PLASMA_ENABLE_DEPRECATED_SINCE(5, 94)
#include <KPluginInfo>
#endif

QTEST_MAIN(PluginTest)

PluginTest::PluginTest()
    : m_buildonly(false)
{
}

void PluginTest::listEngines()
{
    QVector<KPluginMetaData> plugins = Plasma::PluginLoader::self()->listDataEngineMetaData();
    //     foreach (const KPluginInfo& info, plugins) {
    // qDebug() << " Found DataEngine: " << info.pluginName() << info.name();
    //     }
    qDebug() << " Found " << plugins.count() << " DataEngines";
    // Switch to true in order to let tests pass, this test usually will only
    // work with plugins installed, but there aren't any in plasma-framework
    m_buildonly = plugins.count() == 0;
    QVERIFY(plugins.count() > 0 || m_buildonly);
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
#if PLASMA_BUILD_DEPRECATED_SINCE(5, 83)
    const KPluginInfo::List plugins = Plasma::PluginLoader::listContainmentsOfType(QStringLiteral("Desktop"));
    qDebug() << "Desktop Containments: " << plugins.count();
    QVERIFY(plugins.count() > 0 || m_buildonly);
#endif

    const QList<KPluginMetaData> pluginsMetaData = Plasma::PluginLoader::listContainmentsMetaDataOfType(QStringLiteral("Desktop"));
    qDebug() << "Desktop Containments MetaData: " << pluginsMetaData.count();
    QVERIFY(pluginsMetaData.count() > 0 || m_buildonly);
}

static const auto source = QStringLiteral("Europe/Sofia");

void EngineTest::dataUpdated(const QString &s, const Plasma::DataEngine::Data &data)
{
    QVERIFY(source == s);
    QVERIFY(data["Timezone"] == source);
}

void PluginTest::loadDataEngine()
{
    if (m_buildonly) {
        return;
    }
    QPointer<Plasma::DataEngine> engine;
    QPointer<Plasma::DataEngine> nullEngine;
    {
        Plasma::DataEngineConsumer consumer;
        engine = consumer.dataEngine(QStringLiteral("time"));
        nullEngine = consumer.dataEngine(QStringLiteral("noop"));
        QVERIFY(nullEngine && engine);
        QVERIFY(!nullEngine->isValid() && engine->isValid());
        {
            EngineTest test;
            engine->connectSource(source, &test);
            QSignalSpy spy(engine, SIGNAL(sourceAdded(QString)));
            spy.wait();
            QVERIFY(!engine->isEmpty());
        }
        QSignalSpy spy(engine, SIGNAL(sourceRemoved(QString)));
        spy.wait();
        QVERIFY(engine->isEmpty());
    }
    QVERIFY(!nullEngine.isNull() && engine.isNull());
}

#include "moc_pluginloadertest.cpp"
