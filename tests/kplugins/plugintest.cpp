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
    QApplication(argc, argv) // QApp needed for CursorNotificationHandler (QWidget)
{
    d = new PluginTestPrivate;
    d->parser = parser;
    QTimer::singleShot(0, this, &PluginTest::runMain);
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
//
    qDebug() << " - - - -- - - - - ------------------------------------\n";
    qDebug() << "::: loadFromPlasma() == " << loadFromPlasma();
    exit(0);
    return;
}

bool PluginTest::loadKPlugin()
{
    bool ok = false;
    qDebug() << "Load KPlugin";
    QString pluginPath = QStringLiteral("/home/sebas/kf5/install/lib/x86_64-linux-gnu/kplugins/");
    QCoreApplication::addLibraryPath(pluginPath);
    //QPluginLoader loader("/home/sebas/kf5/install/lib/x86_64-linux-gnu/kplugins/libkqpluginfactory.so", this);
    QPluginLoader loader(QStringLiteral("/home/sebas/kf5/install/lib/x86_64-linux-gnu/plugins/kf5/kplugins/libplasma_engine_time.so"), this);
    KPluginFactory *factory = qobject_cast<KPluginFactory *>(loader.instance());
    //QObject *factory = loader.instance();
    if (factory) {
        qDebug() << "loaded successfully and cast";
        qDebug() << "metadata: " << loader.metaData();
        //QObject *o = factory->createPlugin("time");
        //qDebug() << " objec name:" << o->objectName();
        //Plasma::DataEngine *time_engine = qobject_cast<Plasma::DataEngine*>(factory->create(this, QVariantList()));
        Plasma::DataEngine *time_engine = nullptr;
//         Plasma::DataEngine *time_engine = factory->create(this, QVariantList());
        time_engine = factory->create<Plasma::DataEngine>(this, QVariantList());

        if (time_engine) {
            qDebug() << "Successfully loaded timeengine";
            time_engine->connectSource(QStringLiteral("Europe/Amsterdam"), this);
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

bool PluginTest::loadFromPlasma()
{
    bool ok = false;
    const QStringList allEngines = Plasma::PluginLoader::self()->listAllEngines();
    qDebug() << "All engines: " << allEngines;
    foreach (const QString &e, allEngines) {
        Plasma::DataEngine *engine = Plasma::PluginLoader::self()->loadDataEngine(e);
        if (engine) {
            engine->connectSource(QStringLiteral("Europe/Amsterdam"), this);
            engine->connectSource(QStringLiteral("Battery"), this);
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

void PluginTest::dataUpdated(QString s, Plasma::DataEngine::Data d)
{
    qDebug() << "new data for source:" << s << d;
}

} // namespace Plasma

#include "moc_plugintest.cpp"

