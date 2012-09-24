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

#include "dataengineconsumer.h"
#include "private/dataengineconsumer_p.h"

#include <QSet>
#include <qurlpathinfo.h>

#include <kdebug.h>

#include "dataenginemanager.h"
#include "private/remotedataengine_p.h"
#include "servicejob.h"

namespace Plasma
{

void DataEngineConsumerPrivate::slotJobFinished(Plasma::ServiceJob *job)
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
    if (!remoteEngines.contains(pair)) {
#ifndef NDEBUG
        kDebug() << "engine does not exist yet!";
#endif
    } else {
        QUrlPathInfo engineLocation = QUrlPathInfo(QUrl(location));
        engineLocation.setFileName(job->result().toString());
#ifndef NDEBUG
        kDebug() << "setting location : " << engineLocation.url();
#endif
        remoteEngines[pair]->setLocation(engineLocation.url());
    }
}

void DataEngineConsumerPrivate::slotServiceReady(Plasma::Service *plasmoidService)
{
#ifndef NDEBUG
    kDebug() << "service ready!";
#endif
    if (!engineNameForService.contains(plasmoidService)) {
#ifndef NDEBUG
        kDebug() << "no engine name for service!";
#endif
#ifndef NDEBUG
        kDebug() << "amount of services in map: " << engineNameForService.count();
#endif
    } else {
#ifndef NDEBUG
        kDebug() << "value = " << engineNameForService.value(plasmoidService);
#endif
    }

#ifndef NDEBUG
    kDebug() << "requesting dataengine!";
#endif
    KConfigGroup op = plasmoidService->operationDescription("DataEngine");
    op.writeEntry("EngineName", engineNameForService.value(plasmoidService));
    plasmoidService->startOperationCall(op);
    connect(plasmoidService, SIGNAL(finished(Plasma::ServiceJob*)),
            this, SLOT(slotJobFinished(Plasma::ServiceJob*)));
}

DataEngine *DataEngineConsumerPrivate::remoteDataEngine(const QString &name, const QUrl &location)
{
    QPair<QString, QString> pair(location.toString(), name);
#ifndef NDEBUG
    kDebug() << "pair = " << pair;
#endif
    if (remoteEngines.contains(pair)) {
#ifndef NDEBUG
        kDebug() << "existing remote dataengine at " << location;
#endif
        return remoteEngines[pair];
    }

#ifndef NDEBUG
    kDebug() << "new remote dataengine at " << location;
#endif
    RemoteDataEngine *engine = new RemoteDataEngine(QUrl());
    remoteEngines[pair] = engine;
    Service *plasmoidService = Service::access(location);
    plasmoidService->setDestination(location.toString());
    engineNameForService[plasmoidService] = name;
#ifndef NDEBUG
    kDebug() << "name = " << name;
#endif

    connect(plasmoidService, SIGNAL(serviceReady(Plasma::Service*)),
            this, SLOT(slotServiceReady(Plasma::Service*)));
    return engine;
}

DataEngineConsumer::DataEngineConsumer()
    : d(new DataEngineConsumerPrivate)
{
}

DataEngineConsumer::~DataEngineConsumer()
{
    foreach (const QString &engine, d->loadedEngines) {
        DataEngineManager::self()->unloadEngine(engine);
    }

    delete d;
}

DataEngine *DataEngineConsumer::dataEngine(const QString &name, const QUrl &location)
{
    if (!location.isEmpty()) {
        return d->remoteDataEngine(name, location);
    }

    if (d->loadedEngines.contains(name)) {
        DataEngine *engine = DataEngineManager::self()->engine(name);
        if (engine->isValid()) {
            return engine;
        } else {
            d->loadedEngines.remove(name);
        }
    }

    DataEngine *engine = DataEngineManager::self()->loadEngine(name);
    if (engine->isValid()) {
        d->loadedEngines.insert(name);
    }

    return engine;
}

} // namespace Plasma

#include "private/moc_dataengineconsumer_p.cpp"


