/*
    SPDX-FileCopyrightText: 2005 Aaron Seigo <aseigo@kde.org>
    SPDX-FileCopyrightText: 2007 Riccardo Iaconelli <riccardo@kde.org>
    SPDX-FileCopyrightText: 2008 Ménard Alexis <darktears31@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "dataengineconsumer.h"
#include "private/dataengineconsumer_p.h"

#include <QSet>

#include <QDebug>

#include "debug_p.h"
#include "private/dataenginemanager_p.h"
#include "servicejob.h"

namespace Plasma
{
void DataEngineConsumerPrivate::slotJobFinished(Plasma::ServiceJob *job)
{
#ifndef NDEBUG
    // qCDebug(LOG_PLASMA) << "engine ready!";
    QString engineName = job->parameters().value(QStringLiteral("EngineName")).toString();
    QString location = job->destination();
    QPair<QString, QString> pair(location, engineName);
    // qCDebug(LOG_PLASMA) << "pair = " << pair;
#endif
}

void DataEngineConsumerPrivate::slotServiceReady(Plasma::Service *plasmoidService)
{
    QVariantMap op = plasmoidService->operationDescription(QStringLiteral("DataEngine"));
    op[QStringLiteral("EngineName")] = engineNameForService.value(plasmoidService);
    plasmoidService->startOperationCall(op);
    connect(plasmoidService, SIGNAL(finished(Plasma::ServiceJob *)), this, SLOT(slotJobFinished(Plasma::ServiceJob *)));
}

DataEngineConsumer::DataEngineConsumer()
    : d(new DataEngineConsumerPrivate)
{
}

DataEngineConsumer::~DataEngineConsumer()
{
    for (const QString &engine : std::as_const(d->loadedEngines)) {
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
        }
    }

    DataEngine *engine = DataEngineManager::self()->loadEngine(name);
    d->loadedEngines.insert(name);
    return engine;
}

} // namespace Plasma

#include "private/moc_dataengineconsumer_p.cpp"
