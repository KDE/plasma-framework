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

#include <QTest>
#include <QDebug>
#include <QDirIterator>
#include <QStandardPaths>

#include <kplugintrader.h>

#include <plasma/pluginloader.h>


QTEST_MAIN(PluginTraderTest)

/* Preliminary performance timers:
 *
 * recursive into subdirs 37 ms per type
 * non recursive          ?? ms
 *
 */

// Switch to true in order to let tests pass, this test usually will only
// work with plugins installed, but there aren't any in plasma-framework
bool buildonly = false;

void PluginTraderTest::initTestCase()
{
    m_pluginDirs[QStringLiteral("Plasma/Applet")] = QStringLiteral("plasmoids");
    m_pluginDirs[QStringLiteral("Plasma/LayoutTemplate")] = QStringLiteral("layout-templates");
    m_pluginDirs[QStringLiteral("Plasma/LookAndFeel")] = QStringLiteral("look-and-feel");
    m_pluginDirs[QStringLiteral("Plasma/Generic")] = QStringLiteral("packages");
    m_pluginDirs[QStringLiteral("Plasma/Shell")] = QStringLiteral("shells");
    m_pluginDirs[QStringLiteral("Plasma/Wallpaper")] = QStringLiteral("wallpapers");

    m_recursive = false;
}

void PluginTraderTest::cleanupTestCase()
{

}

void PluginTraderTest::listPackages()
{
    m_recursive = true;

    QElapsedTimer timer;
    QElapsedTimer innertimer;

    timer.start();


    foreach (const QString &servicetype, m_pluginDirs.keys()) {
        innertimer.start();

        KPluginInfo::List lst;
        const QString packageLocation = QStringLiteral("plasma/") + m_pluginDirs[servicetype];

        const QStringList paths = QStandardPaths::locateAll(QStandardPaths::GenericDataLocation, packageLocation, QStandardPaths::LocateDirectory);

        Q_FOREACH (const QString &plugindir, paths) {
            if (QDir(plugindir).exists()) {
                lst << queryPackages(plugindir, servicetype);
            }
        }

        //qDebug() << " Found " << servicetype << " : " << lst.count() << " Packages in " << innertimer.elapsed() << "milliseconds";;
        //QVERIFY(lst.count() > 0 || buildonly);

    }
    qDebug() << "Directory lookups took" << timer.elapsed() << "milliseconds";

}

void PluginTraderTest::listPackagesRecursive()
{
    m_recursive = true;

    QElapsedTimer timer;
    timer.start();

    foreach (const QString &servicetype, m_pluginDirs.keys()) {
        QElapsedTimer innertimer;
        innertimer.start();
        KPluginInfo::List lst = listPlugins(servicetype);

        //qDebug() << " Found " << servicetype << " : " << lst.count() << " Packages in " << innertimer.elapsed() << "milliseconds";;
        //QVERIFY(lst.count() > 0 || buildonly);

    }
    qDebug() << "Recursive querying took" << timer.elapsed() << "milliseconds";
}

KPluginInfo::List PluginTraderTest::listPlugins(const QString& servicetype)
{
    const QStringList paths = QStandardPaths::locateAll(QStandardPaths::GenericDataLocation, "plasma", QStandardPaths::LocateDirectory);
    KPluginInfo::List lst;

    Q_FOREACH (const QString &plugindir, paths) {
        lst << queryPackages(plugindir, servicetype);
    }
    return lst;
}

KPluginInfo::List PluginTraderTest::queryPackages(const QString& plugindir, const QString& servicetype, const QString &constraint)
{
    const QDirIterator::IteratorFlags flags = (m_recursive ? QDirIterator::Subdirectories : QDirIterator::NoIteratorFlags);
    const QStringList nameFilters = QStringList(QStringLiteral("metadata.desktop"));

    KPluginInfo::List lst;
    QDirIterator it(plugindir,
                    nameFilters,
                    QDir::Files,
                    flags);
    while (it.hasNext()) {
        it.next();
        const QString _f = it.fileInfo().absoluteFilePath();
        //loader.setFileName(_f);
        //const QVariantList argsWithMetaData = QVariantList() << loader.metaData().toVariantMap();
//         qDebug() << "found metadata: " << it.fileInfo().absoluteFilePath().remove(plugindir);
        KPluginInfo info(it.fileInfo().absoluteFilePath());
        if (!info.isValid()) {
            continue;
        }
        if (servicetype.isEmpty() || info.serviceTypes().contains(servicetype)) {
            lst << info;
        }
    }
    KPluginTrader::applyConstraints(lst, constraint);
    return lst;

}

void PluginTraderTest::listPackagesFromTestData()
{
    const QString testdatadir = QFINDTESTDATA("data/plasma");
    qDebug() << "Test data: " << testdatadir;
    QElapsedTimer timer;
    QElapsedTimer innertimer;
    timer.start();
    foreach (const QString &servicetype, m_pluginDirs.keys()) {
        innertimer.start();
        KPluginInfo::List lst = queryPackages(testdatadir, servicetype);

        int ms = innertimer.elapsed();
        qDebug() << " Found " << servicetype << " : " << lst.count() << " Packages in " << ms << "milliseconds";
        //QVERIFY(lst.count() > 0 || buildonly);
    }
    qDebug() << "Querying testdata takes" << (int)(timer.elapsed()/m_pluginDirs.count()) << "msec";
}

//#include "plugintradertest.moc"