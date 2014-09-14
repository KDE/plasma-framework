/******************************************************************************
*   Copyright 2014 Sebastian Kügler <sebas@kde.org>                           *
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

#include "plugintradertest.h"

#include <qtest.h>
#include <QDebug>

#include <kplugininfo.h>
//#include <kplugintrader.h>

#include <plasma/pluginloader.h>

QTEST_MAIN(PluginTraderTest)

// Switch to true in order to let tests pass, this test usually will only
// work with plugins installed, but there aren't any in plasma-framework
bool buildonly = true;

void PluginTraderTest::listPackages()
{
    KPluginInfo::List plugins = Plasma::PluginLoader::listEngineInfo();
    foreach (const KPluginInfo info, plugins) {
        //qDebug() << " Found DataEngine: " << info.pluginName() << info.name();
    }
    qDebug() << " Found " << plugins.count() << " DataEngines";
    QVERIFY(plugins.count() > 0 || buildonly);
}

