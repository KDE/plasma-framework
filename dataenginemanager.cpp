/*
 *   Copyright (C) 2006 Aaron Seigo <aseigo@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License version 2 as
 *   published by the Free Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "dataenginemanager.h"

#include <KDebug>
#include <KServiceTypeTrader>
#include <KParts/ComponentFactory>

namespace Plasma
{

class NullEngine : public DataEngine
{
    public:
        NullEngine(QObject* parent = 0)
            : DataEngine(parent)
        {
            setValid(false);
        }
};

class DataEngineManager::Private
{
    public:
        Private()
            : null(0)
        {}

        ~Private()
        {
            delete null;
        }

        DataEngine* nullEngine()
        {
            if (!null) {
                null = new NullEngine;
            }

            return null;
        }

        DataEngine::Dict m_engines;
        DataEngine* null;
};

class DataEngineManagerSingleton
{
    public:
        DataEngineManager self;
};

K_GLOBAL_STATIC(DataEngineManagerSingleton, privateSelf)

DataEngineManager* DataEngineManager::self()
{
    return &privateSelf->self;
}

DataEngineManager::DataEngineManager()
    : d(new Private())
{
}

DataEngineManager::~DataEngineManager()
{
    foreach (Plasma::DataEngine* engine, d->m_engines) {
        delete engine;
    }
    d->m_engines.clear();
}

Plasma::DataEngine* DataEngineManager::dataEngine(const QString& name) const
{
    Plasma::DataEngine::Dict::const_iterator it = d->m_engines.find(name);
    if (it != d->m_engines.end()) {
        // ref and return the engine
        //Plasma::DataEngine *engine = *it;
        return *it;
    }

    return d->nullEngine();
}

Plasma::DataEngine* DataEngineManager::loadDataEngine(const QString& name)
{
    Plasma::DataEngine* engine = 0;
    Plasma::DataEngine::Dict::const_iterator it = d->m_engines.find(name);

    if (it != d->m_engines.end()) {
        engine = *it;
        engine->ref();
        return engine;
    }

    // load the engine, add it to the engines
    QString constraint = QString("[X-EngineName] == '%1'").arg(name);
    KService::List offers = KServiceTypeTrader::self()->query("Plasma/DataEngine",
                                                              constraint);

    if (offers.isEmpty()) {
        kDebug() << "offers are empty for " << name << " with constraint " << constraint << endl;
        return 0;
    }

    engine = KService::createInstance<Plasma::DataEngine>(offers.first(), 0);
    if (!engine) {
        kDebug() << "Couldn't load engine \"" << name << "\"!" << endl;
        engine = d->nullEngine();
    }

    engine->ref();
    engine->setObjectName(offers.first()->name());
    engine->setIcon(offers.first()->icon());
    d->m_engines[name] = engine;
    return engine;
}

void DataEngineManager::unloadDataEngine(const QString& name)
{
    Plasma::DataEngine* engine = dataEngine(name);

    if (engine) {
        engine->deref();

        if (!engine->isUsed()) {
            d->m_engines.remove(name);
            delete engine;
        }
    }
}

QStringList DataEngineManager::knownEngines()
{
    QStringList engines;
    KService::List offers = KServiceTypeTrader::self()->query("Plasma/DataEngine");
    foreach (KService::Ptr service, offers) {
        engines.append(service->property("X-EngineName").toString());
    }

    return engines;
}

} // namespace Plasma

