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
#include <QJsonDocument>
#include <QJsonObject>
#include <QStandardPaths>

#include <kplugintrader.h>
#include <KServiceTypeTrader>

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
    //qDebug() << "QUerying " << plugindir;
    QFile cachefile(plugindir+"/plasma-packagecache.json");
    KPluginInfo::List lst;
    if (cachefile.exists()) {
//         QElapsedTimer t2;
//         //qDebug() << "Cache file: " << cachefile.fileName().remove(plugindir);
//         t2.start();
        cachefile.open(QIODevice::ReadOnly);
        QJsonDocument jdoc = QJsonDocument::fromBinaryData(cachefile.readAll());
        cachefile.close();
//         qDebug() << "Reading cache :   " << t2.elapsed() << "msec";
//         t2.start();
        QJsonObject obj = jdoc.object();
        const QVariantMap &mainVm = obj.toVariantMap();
//         qDebug() << "Version: " << mainVm["Version"].toString();
//         qDebug() << "Timestamp: " << mainVm["Timestamp"].toDouble();
        const QVariantMap &packagesVm = mainVm["Packages"].toMap();
//         qDebug() << "decoding:         " << t2.elapsed() << "msec";
//         t2.start();
        //foreach (const QString &pluginname, packagesVm.keys()) {
        for(QVariantMap::const_iterator iter = packagesVm.begin(); iter != packagesVm.end(); ++iter) {
            //qDebug() << iter.key() << iter.value();
            QVariantList pluginArgs;
            pluginArgs << iter.value().toMap();
            KPluginInfo info(pluginArgs);
            if (!info.isValid()) {
                continue;
            }
            if (servicetype.isEmpty() || info.serviceTypes().contains(servicetype)) {
                //qDebug() << "Yay! : " << info.name();
                lst << info;
            }
        }
//         qDebug() << "creating KPI:     " << t2.elapsed() << "msec";
//         t2.start();

    } else {
//         qDebug() << "Not cached";
        // If there's no cache file, fall back to listing the directory
        const QDirIterator::IteratorFlags flags = (m_recursive ? QDirIterator::Subdirectories : QDirIterator::NoIteratorFlags);
        const QStringList nameFilters;// = QStringList(QStringLiteral("metadata.desktop"));

        QDirIterator it(plugindir,
                        nameFilters,
                        QDir::Dirs,
                        flags);
        while (it.hasNext()) {
            it.next();
            const QString _f = it.fileInfo().absoluteFilePath() + QStringLiteral("/metadata.desktop");
            //loader.setFileName(_f);
            //const QVariantList argsWithMetaData = QVariantList() << loader.metaData().toVariantMap();
            //qDebug() << "found metadata: " << it.fileInfo().absoluteFilePath().remove(plugindir);
            KPluginInfo info(_f);
            if (!info.isValid()) {
                continue;
            }
            if (servicetype.isEmpty() || info.serviceTypes().contains(servicetype)) {
                lst << info;
            }
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
        KPluginInfo::List lst = queryPackages(testdatadir+"/"+m_pluginDirs[servicetype], servicetype);

        int ms = innertimer.elapsed();
        qDebug() << " Found " << servicetype << " : " << lst.count() << " Packages in " << ms << "milliseconds";
        //QVERIFY(lst.count() > 0 || buildonly);
    }
    qDebug() << "Querying testdata takes" << (int)(timer.elapsed()/m_pluginDirs.count()) << "msec";
}

void PluginTraderTest::queryPackage()
{

    QVERIFY(queryType("Plasma/Generic").count() > 0);
    QVERIFY(queryType("Plasma/Wallpaper").count() > 0);
}

KPluginInfo::List PluginTraderTest::queryType(const QString &servicetype)
{
    QElapsedTimer t;
    t.start();
    const QString testdatadir = QFINDTESTDATA("data/plasma") + "/" + m_pluginDirs[servicetype];
    KPluginInfo::List lst = queryPackages(testdatadir, servicetype);
    qDebug() << "Query for " << servicetype << " returned " << lst.count() << "hits in " << t.elapsed() << "msec";
    return lst;
}

void PluginTraderTest::queryDataEngines()
{

}

void PluginTraderTest::listEngineInfoByCategory()
{
    QStringList allEngines = Plasma::PluginLoader::self()->listAllEngines();
    qDebug() << "All engines: " << allEngines.count();
    QVERIFY(allEngines.count() > 0);

    QString constraint = "(not exist [X-KDE-ParentApp] or [X-KDE-ParentApp] == '')";
    KPluginInfo::List offers = KPluginTrader::self()->query("plasma/dataengine", "Plasma/DataEngine", constraint);
    qDebug() << "Trader found : " << offers.count() << "engines";

    KPluginInfo::List cats = Plasma::PluginLoader::self()->listEngineInfoByCategory("Date and Time"); // time engine!
    qDebug() << "in Category Date and Time: " << cats.count();

}

void PluginTraderTest::countAllEngines()
{
    QElapsedTimer t;
    t.start();

    // Look up engines in both, KServiceTypeTrader and KPluginTrader, compare the values
    QString constraint = "(not exist [X-KDE-ParentApp] or [X-KDE-ParentApp] == '')";

    QStringList pes = Plasma::PluginLoader::self()->listAllEngines();
    qDebug() << "Plasma::PluginLoader: " << pes.count() << t.elapsed() << "msec";
    t.start();

    KPluginInfo::List es = KPluginTrader::self()->query("plasma/dataengine", "Plasma/DataEngine", constraint);

    QStringList engines;
    foreach (const KPluginInfo &info, es) {
        const QString name = info.pluginName();
        if (!name.isEmpty()) {
            engines.append(name);
        }
    }

    qDebug() << "KPlugin: " << engines.count() << t.elapsed() << "msec";
    t.start();

    KService::List ss = KServiceTypeTrader::self()->query("Plasma/DataEngine", constraint);
    QStringList serviceengines;
    foreach (const KService::Ptr &service, ss) {
        QString name = service->property("X-KDE-PluginInfo-Name").toString();
        if (!name.isEmpty()) {
            if (!pes.contains(name)) {
                qWarning() << "Engine not found with Plasma::PluginLoader, but exists in KService! " << name;
            }
            if (!engines.contains(name)) {
                qWarning() << "Engine not found with KPluginTrader, but exists in KService! " << name;
                qWarning() << "This likely means that the engine's json metadata is missing,";
                qWarning() << "or that the plugin isn't installed into $PLUGINS/plasma/dataengines!";
            }
            serviceengines.append(name);
        } else {
            qDebug() << "empty" << name;
        }

    }
    qDebug() << "KService: " << serviceengines.count() << t.elapsed() << "msec";
    t.start();



    QVERIFY(pes.count() >= serviceengines.count());
    QVERIFY(engines.count() >= serviceengines.count());

}

void PluginTraderTest::listApplets()
{

    QElapsedTimer t;
    t.start();

    // list from cache or read plugin directory's metadata
    KPluginInfo::List lst;
    const QString packageLocation = QStringLiteral("plasma/plasmoids");

    const QStringList paths = QStandardPaths::locateAll(QStandardPaths::GenericDataLocation, packageLocation, QStandardPaths::LocateDirectory);

    Q_FOREACH (const QString &plugindir, paths) {
        //qDebug() << "PluginDir: " << plugindir;
        if (QDir(plugindir).exists()) {
            lst << queryPackages(plugindir, QStringLiteral("Plasma/Applet"));
        }
    }
    QString constraint = "(not exist [X-KDE-ParentApp] or [X-KDE-ParentApp] == '')";
    KPluginTrader::applyConstraints(lst, constraint);

    int tpackage = t.nsecsElapsed();
    qDebug() << "Indexed: " << lst.count() << QString::number(tpackage/1000000.0, 'f', 1) << "msec";
    t.start();

    // Plasma::PluginLoader
    lst = Plasma::PluginLoader::self()->listAppletInfo(QString());
    int tplugin = t.nsecsElapsed();
    qDebug() << "PluginLoader: " << lst.count() << QString::number(tplugin/1000000.0, 'f', 1) << "msec";
    t.start();

    qDebug() << "tplugin / tpackage " << (tpackage / (qreal)(tplugin));
    qDebug() << "Speedup is : " << QString::number(tplugin / (qreal)(tpackage)*100, 'f', 0) << "%";
    QVERIFY(tpackage < tplugin); // make sure we're always faster than KService
    QVERIFY(tpackage < 6000000); // make sure we can query in less than one frame

}


//#include "plugintradertest.moc"