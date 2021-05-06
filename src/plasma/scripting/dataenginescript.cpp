/*
    SPDX-FileCopyrightText: 2007 Aaron Seigo <aseigo@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "dataenginescript.h"

#if PLASMA_BUILD_DEPRECATED_SINCE(5, 83)
#include "package.h"
#endif
#include "private/dataengine_p.h"
#include "private/service_p.h"

namespace Plasma
{
class DataEngineScriptPrivate
{
public:
    DataEngine *dataEngine;
};

DataEngineScript::DataEngineScript(QObject *parent)
    : ScriptEngine(parent)
    , d(new DataEngineScriptPrivate)
{
}

DataEngineScript::~DataEngineScript()
{
    delete d;
}

void DataEngineScript::setDataEngine(DataEngine *dataEngine)
{
    d->dataEngine = dataEngine;
}

DataEngine *DataEngineScript::dataEngine() const
{
    return d->dataEngine;
}

QStringList DataEngineScript::sources() const
{
    return d->dataEngine->containerDict().keys();
}

bool DataEngineScript::sourceRequestEvent(const QString &name)
{
    Q_UNUSED(name);
    return false;
}

bool DataEngineScript::updateSourceEvent(const QString &source)
{
    Q_UNUSED(source);
    return false;
}

Service *DataEngineScript::serviceForSource(const QString &source)
{
    Q_ASSERT(d->dataEngine);
    return new NullService(source, d->dataEngine);
}

#if PLASMA_BUILD_DEPRECATED_SINCE(5, 83)
QString DataEngineScript::mainScript() const
{
    Q_ASSERT(d->dataEngine);
    return d->dataEngine->package().filePath("mainscript");
}

Package DataEngineScript::package() const
{
    Q_ASSERT(d->dataEngine);
    return d->dataEngine->package();
}

KPluginInfo DataEngineScript::description() const
{
    return KPluginInfo(metadata());
}
#endif

KPluginMetaData DataEngineScript::metadata() const
{
    Q_ASSERT(d->dataEngine);
    return d->dataEngine->metadata();
}

void DataEngineScript::setData(const QString &source, const QString &key, const QVariant &value)
{
    if (d->dataEngine) {
        d->dataEngine->setData(source, key, value);
    }
}

void DataEngineScript::setData(const QString &source, const QVariant &value)
{
    if (d->dataEngine) {
        d->dataEngine->setData(source, value);
    }
}

void DataEngineScript::setData(const QString &source, const DataEngine::Data &values)
{
    if (d->dataEngine) {
        d->dataEngine->setData(source, values);
    }
}

void DataEngineScript::removeAllData(const QString &source)
{
    if (d->dataEngine) {
        d->dataEngine->removeAllData(source);
    }
}

void DataEngineScript::removeData(const QString &source, const QString &key)
{
    if (d->dataEngine) {
        d->dataEngine->removeData(source, key);
    }
}

void DataEngineScript::setMinimumPollingInterval(int minimumMs)
{
    if (d->dataEngine) {
        d->dataEngine->setMinimumPollingInterval(minimumMs);
    }
}

int DataEngineScript::minimumPollingInterval() const
{
    if (d->dataEngine) {
        return d->dataEngine->minimumPollingInterval();
    }
    return 0;
}

void DataEngineScript::setPollingInterval(uint frequency)
{
    if (d->dataEngine) {
        d->dataEngine->setPollingInterval(frequency);
    }
}

void DataEngineScript::removeAllSources()
{
    if (d->dataEngine) {
        d->dataEngine->removeAllSources();
    }
}

void DataEngineScript::addSource(DataContainer *source)
{
    if (d->dataEngine) {
        d->dataEngine->addSource(source);
    }
}

DataEngine::SourceDict DataEngineScript::containerDict() const
{
    if (d->dataEngine) {
        return d->dataEngine->containerDict();
    }
    return DataEngine::SourceDict();
}

void DataEngineScript::removeSource(const QString &source)
{
    if (d->dataEngine) {
        d->dataEngine->removeSource(source);
    }
}

void DataEngineScript::updateAllSources()
{
    if (d->dataEngine) {
        d->dataEngine->updateAllSources();
    }
}

void DataEngineScript::forceImmediateUpdateOfAllVisualizations()
{
    if (d->dataEngine) {
        d->dataEngine->forceImmediateUpdateOfAllVisualizations();
    }
}

} // Plasma namespace

#include "moc_dataenginescript.cpp"
