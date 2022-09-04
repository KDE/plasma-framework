/*
    SPDX-FileCopyrightText: 2006-2007 Aaron Seigo <aseigo@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "dataenginemanager_p.h"

#include <QFile>
#include <QTextStream>

#include <QDebug>

#include <QStandardPaths>

#include "datacontainer.h"
#include "debug_p.h"
#include "pluginloader.h"
#include "private/datacontainer_p.h"
#include "private/dataengine_p.h"
#include "scripting/scriptengine.h"

namespace Plasma
{
class NullEngine : public DataEngine
{
public:
    explicit NullEngine(QObject *parent = nullptr)
        : DataEngine(KPluginMetaData(), parent)
    {
        setValid(false);

        // ref() ourselves to ensure we never get deleted
        d->ref();
    }
};

class DataEngineManagerPrivate
{
public:
    DataEngineManagerPrivate()
        : nullEng(nullptr)
    {
    }

    ~DataEngineManagerPrivate()
    {
        for (Plasma::DataEngine *engine : std::as_const(engines)) {
            delete engine;
        }
        engines.clear();
        delete nullEng;
    }

    DataEngine *nullEngine()
    {
        if (!nullEng) {
            nullEng = new NullEngine;
        }

        return nullEng;
    }

    DataEngine::Dict engines;
    DataEngine *nullEng;
};

class DataEngineManagerSingleton
{
public:
    DataEngineManager self;
};

Q_GLOBAL_STATIC(DataEngineManagerSingleton, privateDataEngineManagerSelf)

DataEngineManager *DataEngineManager::self()
{
    return &privateDataEngineManagerSelf()->self;
}

DataEngineManager::DataEngineManager()
    : d(new DataEngineManagerPrivate)
{
    // startTimer(30000);
}

DataEngineManager::~DataEngineManager()
{
    delete d;
}

Plasma::DataEngine *DataEngineManager::engine(const QString &name) const
{
    if (name.isEmpty()) {
        return d->nullEngine();
    }

    Plasma::DataEngine::Dict::const_iterator it = d->engines.constFind(name);
    if (it != d->engines.constEnd()) {
        return *it;
    }

    return d->nullEngine();
}

Plasma::DataEngine *DataEngineManager::loadEngine(const QString &name)
{
    if (name.isEmpty()) {
        qCDebug(LOG_PLASMA) << "Asked an engine with empty name";
        return d->nullEngine();
    }
    Plasma::DataEngine::Dict::const_iterator it = d->engines.constFind(name);

    if (it != d->engines.constEnd()) {
        DataEngine *engine = *it;
        engine->d->ref();
        return engine;
    }

#if PLASMA_BUILD_DEPRECATED_SINCE(5, 94)
    DataEngine *engine = PluginLoader::self()->loadDataEngine(name);
#else
    // Expect the plugin id to be the same as the file name or not explicitly set
    const KPluginMetaData data(QLatin1String("plasma/dataengine/plasma_engine_") + name);
    DataEngine *engine = KPluginFactory::instantiatePlugin<Plasma::DataEngine>(data).plugin;
#endif
    if (!engine) {
        qCDebug(LOG_PLASMA) << "Can't find a dataengine named" << name;
        return d->nullEngine();
    }

    d->engines[name] = engine;
    return engine;
}

void DataEngineManager::unloadEngine(const QString &name)
{
    Plasma::DataEngine::Dict::iterator it = d->engines.find(name);

    if (it != d->engines.end()) {
        Plasma::DataEngine *engine = *it;
        engine->d->deref();

        if (!engine->d->isUsed()) {
            d->engines.erase(it);
            delete engine;
        }
    }
}

void DataEngineManager::timerEvent(QTimerEvent *)
{
#ifndef NDEBUG
    QString path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + QStringLiteral("/plasma_dataenginemanager_log");
    QFile f(path);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        // qCDebug(LOG_PLASMA) << "failed to open" << path;
        return;
    }

    QTextStream out(&f);

    QHashIterator<QString, DataEngine *> it(d->engines);
    out << "================================== " << QLocale().toString(QDateTime::currentDateTime()) << '\n';
    while (it.hasNext()) {
        it.next();
        DataEngine *engine = it.value();
        out << "DataEngine: " << it.key() << ' ' << engine << '\n';
        out << "            Claimed # of sources: " << engine->sources().count() << '\n';
        out << "            Actual # of sources: " << engine->containerDict().count() << '\n';
        out << "\n            Source Details" << '\n';

        const auto lst = engine->containerDict();
        for (DataContainer *dc : lst) {
            out << "                * " << dc->objectName() << '\n';
            out << "                       Data count: " << dc->d->data.count() << '\n';
            out << "                       Stored: " << dc->isStorageEnabled() << " \n";
            const int directs = dc->receivers(SIGNAL(dataUpdated(QString, Plasma::DataEngine::Data)));
            if (directs > 0) {
                out << "                       Direction Connections: " << directs << " \n";
            }

            const int relays = dc->d->relays.count();
            if (relays > 0) {
                out << "                       Relays: " << dc->d->relays.count() << '\n';
                QString times;
                for (SignalRelay *relay : std::as_const(dc->d->relays)) {
                    times.append(QLatin1Char(' ') + QString::number(relay->m_interval));
                }
                out << "                       Relay Timeouts: " << times << " \n";
            }
        }

        out << "\n-----\n";
    }
    out << "\n\n";
#endif
    //    killTimer(event->timerId());
}

} // namespace Plasma

#include "moc_dataenginemanager_p.cpp"
