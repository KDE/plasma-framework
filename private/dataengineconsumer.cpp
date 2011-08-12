/*
 *   Copyright 2005 by Aaron Seigo <aseigo@kde.org>
 *   Copyright 2007 by Riccardo Iaconelli <riccardo@kde.org>
 *   Copyright 2008 by Ménard Alexis <darktears31@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
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

#include "dataengineconsumer_p.h"

#include <QtCore/QSet>

#include <kdebug.h>

#include "plasma/dataenginemanager.h"
#include "plasma/private/remotedataengine_p.h"
#include <servicejob.h>

namespace Plasma
{

ServiceMonitor::ServiceMonitor(DataEngineConsumer *consumer)
    : m_consumer(consumer)
{
}

ServiceMonitor::~ServiceMonitor()
{
}

void ServiceMonitor::slotJobFinished(Plasma::ServiceJob *job)
{
#ifndef NDEBUG
    kDebug() << "engine ready!";
#endif
    QString engineName = job->parameters()["EngineName"].toString();
    QString location = job->destination();
    QPair<QString, QString> pair(location, engineName);
#ifndef NDEBUG
    kDebug() << "pair = " << pair;
#endif
    if (!m_consumer->m_remoteEngines.contains(pair)) {
#ifndef NDEBUG
        kDebug() << "engine does not exist yet!";
#endif
    } else {
        KUrl engineLocation(location);
        engineLocation.setFileName(job->result().toString());
#ifndef NDEBUG
        kDebug() << "setting location : " << engineLocation.prettyUrl();
#endif
        m_consumer->m_remoteEngines[pair]->setLocation(engineLocation);
    }
}

void ServiceMonitor::slotServiceReady(Plasma::Service *plasmoidService)
{
#ifndef NDEBUG
    kDebug() << "service ready!";
#endif
    if (!m_consumer->m_engineNameForService.contains(plasmoidService)) {
#ifndef NDEBUG
        kDebug() << "no engine name for service!";
#endif
#ifndef NDEBUG
        kDebug() << "amount of services in map: " << m_consumer->m_engineNameForService.count();
#endif
    } else {
#ifndef NDEBUG
        kDebug() << "value = " << m_consumer->m_engineNameForService.value(plasmoidService);
#endif
    }

#ifndef NDEBUG
    kDebug() << "requesting dataengine!";
#endif
    KConfigGroup op = plasmoidService->operationDescription("DataEngine");
    op.writeEntry("EngineName", m_consumer->m_engineNameForService.value(plasmoidService));
    plasmoidService->startOperationCall(op);
    connect(plasmoidService, SIGNAL(finished(Plasma::ServiceJob*)),
            this, SLOT(slotJobFinished(Plasma::ServiceJob*)));
}

DataEngineConsumer::DataEngineConsumer()
    : m_monitor(new ServiceMonitor(this))
{
}

DataEngineConsumer::~DataEngineConsumer()
{
    foreach (const QString &engine, m_loadedEngines) {
        DataEngineManager::self()->unloadEngine(engine);
    }

    delete m_monitor;
}

DataEngine *DataEngineConsumer::dataEngine(const QString &name)
{
    if (m_loadedEngines.contains(name)) {
        DataEngine *engine = DataEngineManager::self()->engine(name);
        if (engine->isValid()) {
            return engine;
        } else {
            m_loadedEngines.remove(name);
        }
    }

    DataEngine *engine = DataEngineManager::self()->loadEngine(name);
    if (engine->isValid()) {
        m_loadedEngines.insert(name);
    }

    return engine;
}

DataEngine *DataEngineConsumer::remoteDataEngine(const KUrl &location, const QString &name)
{
    QPair<QString, QString> pair(location.prettyUrl(), name);
#ifndef NDEBUG
    kDebug() << "pair = " << pair;
#endif
    if (m_remoteEngines.contains(pair)) {
#ifndef NDEBUG
        kDebug() << "existing remote dataengine at " << location;
#endif
        return m_remoteEngines[pair];
    }

#ifndef NDEBUG
    kDebug() << "new remote dataengine at " << location;
#endif
    RemoteDataEngine *engine = new RemoteDataEngine(KUrl());
    m_remoteEngines[pair] = engine;
    Service *plasmoidService = Service::access(location);
    plasmoidService->setDestination(location.prettyUrl());
    m_engineNameForService[plasmoidService] = name;
#ifndef NDEBUG
    kDebug() << "name = " << name;
#endif
    QObject::connect(plasmoidService, SIGNAL(serviceReady(Plasma::Service*)),
                     m_monitor, SLOT(slotServiceReady(Plasma::Service*)));
    return engine;
}


} // namespace Plasma

#include "dataengineconsumer_p.moc"


