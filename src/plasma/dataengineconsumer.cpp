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
#include <QUrl>

#include <QDebug>

#include "private/dataenginemanager_p.h"
#include "servicejob.h"

namespace Plasma
{

void DataEngineConsumerPrivate::slotJobFinished(Plasma::ServiceJob *job)
{
#ifndef NDEBUG
    // qDebug() << "engine ready!";
#endif
    QString engineName = job->parameters()["EngineName"].toString();
    QString location = job->destination();
    QPair<QString, QString> pair(location, engineName);
#ifndef NDEBUG
    // qDebug() << "pair = " << pair;
#endif
}

void DataEngineConsumerPrivate::slotServiceReady(Plasma::Service *plasmoidService)
{
#ifndef NDEBUG
    // qDebug() << "service ready!";
#endif
    if (!engineNameForService.contains(plasmoidService)) {
#ifndef NDEBUG
        // qDebug() << "no engine name for service!";
#endif
#ifndef NDEBUG
        // qDebug() << "amount of services in map: " << engineNameForService.count();
#endif
    } else {
#ifndef NDEBUG
        // qDebug() << "value = " << engineNameForService.value(plasmoidService);
#endif
    }

#ifndef NDEBUG
    // qDebug() << "requesting dataengine!";
#endif
    QVariantMap op = plasmoidService->operationDescription("DataEngine");
    op["EngineName"] = engineNameForService.value(plasmoidService);
    plasmoidService->startOperationCall(op);
    connect(plasmoidService, SIGNAL(finished(Plasma::ServiceJob*)),
            this, SLOT(slotJobFinished(Plasma::ServiceJob*)));
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

DataEngine *DataEngineConsumer::dataEngine(const QString &name)
{
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


