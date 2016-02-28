/******************************************************************************
*   Copyright 2013 Sebastian Kügler <sebas@kde.org>                           *
*                                                                             *
*   This library is free software; you can redistribute it and/or             *
*   modify it under the terms of the GNU Library General Public               *
*   License as published by the Free Software Foundation; either              *
*   version 2 of the License, or (at your option) any later version.          *
*                                                                             *
*   This library is distributed in the hope that it will be useful,           *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of            *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU          *
*   Library General Public License for more details.                          *
*                                                                             *
*   You should have received a copy of the GNU Library General Public License *
*   along with this library; see the file COPYING.LIB.  If not, write to      *
*   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,      *
*   Boston, MA 02110-1301, USA.                                               *
*******************************************************************************/

#include "pluginloadertest.h"

#include <qtest.h>
#include <QDebug>
//#include <QJsonObject>

#include <kplugininfo.h>
//#include <kplugintrader.h>

#include <plasma/dataengine.h>
#include <plasma/pluginloader.h>

QTEST_MAIN(PluginTest)

// Switch to true in order to let tests pass, this test usually will only
// work with plugins installed, but there aren't any in plasma-framework
bool buildonly = true;

void PluginTest::listEngines()
{
    KPluginInfo::List plugins = Plasma::PluginLoader::listEngineInfo();
    foreach (const KPluginInfo& info, plugins) {
        //qDebug() << " Found DataEngine: " << info.pluginName() << info.name();
    }
    qDebug() << " Found " << plugins.count() << " DataEngines";
    QVERIFY(plugins.count() > 0 || buildonly);
}

void PluginTest::listAppletCategories()
{
    const QStringList cats = Plasma::PluginLoader::self()->listAppletCategories();
    qDebug() << "Categories" << cats;
    QVERIFY(cats.count() > 0 || buildonly);
}

void PluginTest::listContainmentActions()
{
    const KPluginInfo::List plugins = Plasma::PluginLoader::self()->listContainmentActionsInfo(QStringLiteral("plasma-shell"));
    qDebug() << "Categories: " << plugins.count();
    //QVERIFY(plugins.count() > 0 || buildonly);
}

void PluginTest::listContainmentsOfType()
{
    const KPluginInfo::List plugins = Plasma::PluginLoader::listContainmentsOfType(QStringLiteral("Desktop"));
    qDebug() << "Desktop Containments: " << plugins.count();
    QVERIFY(plugins.count() > 0 || buildonly);

}

void PluginTest::loadDataEngine()
{
    Plasma::DataEngine *engine = Plasma::PluginLoader::self()->loadDataEngine(QStringLiteral("time"));
    //qDebug() << "Engine loaded successfully" << engine->pluginInfo().name();
    QVERIFY(engine != 0 || buildonly);

}

#include "moc_pluginloadertest.cpp"

