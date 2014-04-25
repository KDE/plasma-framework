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

#include "plugintest.h"

#include <kpluginfactory.h>

#include <QDebug>
#include <kservice.h>
#include <kservicetypetrader.h>
#include <kshell.h>
#include <ksycoca.h>
#include <klocalizedstring.h>

#include <plasma/applet.h>
#include <plasma/packagestructure.h>
#include <plasma/package.h>
#include <plasma/pluginloader.h>
#include <plasma/dataengine.h>
#include <kjob.h>

#include <qcommandlineparser.h>
#include <QDir>
#include <QDBusInterface>
#include <QFileInfo>
#include <QMap>
#include <QStandardPaths>
#include <QStringList>
#include <QTimer>
#include <QJsonObject>

#include <iostream>
#include <iomanip>

namespace Plasma
{
class PluginTestPrivate
{
public:
    QString pluginName;
    QCommandLineParser *parser;
};

PluginTest::PluginTest(int &argc, char **argv, QCommandLineParser *parser) :
    QApplication(argc, argv)
{
    d = new PluginTestPrivate;
    d->parser = parser;
    QTimer::singleShot(0, this, SLOT(runMain()));
}

PluginTest::~PluginTest()
{
    delete d;
}

void PluginTest::runMain()
{
    qDebug() << "plugin test runs: ";
    //loadDataEngine();
//     qDebug() << " - - - -- - - - - ------------------------------------\n";
//     qDebug() << " libs are in: " << QCoreApplication::libraryPaths();
//     //loadKQPlugin();
    qDebug() << "::: loadKPlugin() == " << loadKPlugin();
//     qDebug() << " - - - -- - - - - ------------------------------------\n";
//     qDebug() << "::: loadKService() == " << loadKService();
//
    qDebug() << " - - - -- - - - - ------------------------------------\n";
    qDebug() << "::: loadKFromPlasma() == " << loadFromPlasma();
    exit(0);
    return;
}

bool PluginTest::loadKPlugin()
{
    bool ok = false;
    qDebug() << "Load KPlugin";
    QString pluginPath = "/home/sebas/kf5/install/lib/x86_64-linux-gnu/kplugins/";
    QCoreApplication::addLibraryPath(pluginPath);
    //QPluginLoader loader("/home/sebas/kf5/install/lib/x86_64-linux-gnu/kplugins/libkqpluginfactory.so", this);
    QPluginLoader loader("/home/sebas/kf5/install/lib/x86_64-linux-gnu/plugins/kf5/kplugins/libplasma_engine_time.so", this);
    KPluginFactory *factory = qobject_cast<KPluginFactory *>(loader.instance());
    //QObject *factory = loader.instance();
    if (factory) {
        qDebug() << "loaded successfully and cast";
        qDebug() << "metadata: " << loader.metaData();
        //QObject *o = factory->createPlugin("time");
        //qDebug() << " objec name:" << o->objectName();
        //Plasma::DataEngine *time_engine = qobject_cast<Plasma::DataEngine*>(factory->create(this, QVariantList()));
        Plasma::DataEngine *time_engine = 0;
//         Plasma::DataEngine *time_engine = factory->create(this, QVariantList());
        time_engine = factory->create<Plasma::DataEngine>(this, QVariantList());

        if (time_engine) {
            qDebug() << "Successfully loaded timeengine";
            time_engine->connectSource("Europe/Amsterdam", this);
            qDebug() << "SOURCE: " << time_engine->sources();
            ok = true;
        } else {
            qDebug() << "Timeengine failed to load. :(";

        }

    } else {
        qDebug() << "loading failed somehow";
    }
    //KQPluginFactory* factory = new KQPluginFactory(KPluginInfo(), this);
    return ok;

}

bool PluginTest::loadFromKService(const QString &name)
{
    DataEngine *engine = 0;

    // load the engine, add it to the engines
    QString constraint = QString("[X-KDE-PluginInfo-Name] == '%1'").arg(name);
    KService::List offers = KServiceTypeTrader::self()->query("Plasma/DataEngine",
                            constraint);
    QString error;

    if (offers.isEmpty()) {
        qDebug() << "offers are empty for " << name << " with constraint " << constraint;
    } else {
        QVariantList allArgs;
        allArgs << offers.first()->storageId();
        QString api = offers.first()->property("X-Plasma-API").toString();
        if (api.isEmpty()) {
            if (offers.first()) {
                KPluginLoader plugin(*offers.first());
                if (Plasma::isPluginVersionCompatible(plugin.pluginVersion())) {
                    engine = offers.first()->createInstance<Plasma::DataEngine>(0, allArgs, &error);
                }
            }
        } else {
            engine = new DataEngine(KPluginInfo(offers.first()), 0);
        }
    }

    return engine != 0;
}

bool PluginTest::loadFromPlasma()
{
    bool ok = false;
    const QStringList allEngines = Plasma::PluginLoader::self()->listAllEngines();
    qDebug() << "All engines: " << allEngines;
    foreach (const QString &e, allEngines) {
        Plasma::DataEngine *engine = Plasma::PluginLoader::self()->loadDataEngine(e);
        if (engine) {
            engine->connectSource("Europe/Amsterdam", this);
            engine->connectSource("Battery", this);
            engine->connectAllSources(this);
            qDebug() << "SOURCE: " << engine->sources();
            ok = true;
        }

    }
    return ok;
}

void PluginTest::loadKQPlugin()
{
    qDebug() << "Load KQPlugin";
#if 0
    QString pluginPath = "/home/sebas/kf5/install/lib/x86_64-linux-gnu/kplugins/";
    QCoreApplication::addLibraryPath(pluginPath);
    //QPluginLoader loader("/home/sebas/kf5/install/lib/x86_64-linux-gnu/kplugins/libkqpluginfactory.so", this);
    QPluginLoader loader("/home/sebas/kf5/install/lib/x86_64-linux-gnu/plugins/kf5/kplugins/libplasma_engine_time.so", this);
    KPluginFactory *factory = qobject_cast<KPluginFactory *>(loader.instance());
    //QObject *factory = loader.instance();
    if (factory) {
        qDebug() << "loaded successfully and cast";
        qDebug() << "metadata: " << loader.metaData();
        //QObject *o = factory->createPlugin("time");
        //qDebug() << " objec name:" << o->objectName();
        //Plasma::DataEngine *time_engine = qobject_cast<Plasma::DataEngine*>(factory->create(this, QVariantList()));
        //Plasma::DataEngine *time_engine = factory->create(this, QVariantList());
        Plasma::DataEngine *time_engine = factory->createInstance<Plasma::DataEngine>(0, this, QVariantList());

        if (time_engine) {
            qDebug() << "Successfully loaded timeengine";
            time_engine->connectSource("Europe/Amsterdam", this);
            qDebug() << "SOURCE: " << time_engine->sources();
        } else {
            qDebug() << "Timeengine failed to load. :(";

        }

    } else {
        qDebug() << "loading failed somehow";
    }
    //KQPluginFactory* factory = new KQPluginFactory(KPluginInfo(), this);
#endif
}

bool PluginTest::loadKService(const QString &name)
{
//     DataEngine *engine = d->isDefaultLoader ? 0 : internalLoadDataEngine(name);
//     if (engine) {
//         return engine;
//     }
    qDebug() << "Load KService";
    DataEngine *engine = 0;
    // load the engine, add it to the engines
    QString constraint = QString("[X-KDE-PluginInfo-Name] == '%1'").arg(name);
    constraint = QString();
    KService::List offers = KServiceTypeTrader::self()->query("Plasma/DataEngine",
                            constraint);
    QString error;

    if (offers.isEmpty()) {
        qDebug() << "offers are empty for " << name << " with constraint " << constraint;
    } else {
        qDebug() << "Found a bunch of stuff";

        QVariantList allArgs;
        allArgs << offers.first()->storageId();
        QString api = offers.first()->property("X-Plasma-API").toString();
        if (api.isEmpty()) {
            if (offers.first()) {
                KPluginLoader plugin(*offers.first());
                if (Plasma::isPluginVersionCompatible(plugin.pluginVersion())) {
                    //KPluginInfo::List infos = KPluginInfo::fromServices(offers.first());
                    //qDebug() << " plugininfo:" << info.name();
                    engine = offers.first()->createInstance<Plasma::DataEngine>(0, allArgs, &error);
                    qDebug() << "DE";
                    if (engine) {
                        engine->connectSource("Europe/Amsterdam", this);
                        qDebug() << "SOURCE: " << engine->sources();
                        //qDebug() << "DataEngine ID: " << engine->pluginInfo().name();
                    } else {
                        qDebug() << "Engine invalid";
                    }
                } else {
                    qDebug() << "Plugin version incompatible" << plugin.pluginVersion();
                }
            }
        } else {
            engine = new DataEngine(KPluginInfo(offers.first()), 0);
            qDebug() << "DataEngine ID (from KPluginINfo): " << engine << engine->pluginInfo().icon() << engine->pluginInfo().name();
        }
        QStringList result;
        foreach (const KService::Ptr &service, offers) {
            const QString _plugin = service->property("X-KDE-PluginInfo-Name", QVariant::String).toString();
            qDebug() << "Found plugin: " << _plugin;
            if (!result.contains(_plugin)) {
                result << _plugin;
                //engine = PluginLoader::self()->loadDataEngine(_plugin);
                //engine = PluginLoader::self()->loadDataEngine(name);
                //qDebug() << "SOURCE: " << engine->sources();

                //engine->setPluginInfo(service->pluginInfo());
                //qDebug() << engine->pluginInfo().name();
            }
        }

    }

    if (!engine) {
        qDebug() << "Couldn't load engine \"" << name << "\". Error given: " << error;
    }

    return engine != 0;
}

void PluginTest::dataUpdated(QString s, Plasma::DataEngine::Data d)
{
//     qDebug() << "new data for source:  " << s << d;
}

} // namespace Plasma

#include "moc_plugintest.cpp"

