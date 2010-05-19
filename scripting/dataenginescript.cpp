/*
 *   Copyright 2007 by Aaron Seigo <aseigo@kde.org>
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

#include "dataenginescript.h"

#include "package.h"
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
    : ScriptEngine(parent),
      d(new DataEngineScriptPrivate)
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
    return d->dataEngine->sources();
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

QString DataEngineScript::mainScript() const
{
    Q_ASSERT(d->dataEngine);
    return d->dataEngine->package()->filePath("mainscript");
}

const Package *DataEngineScript::package() const
{
    Q_ASSERT(d->dataEngine);
    return d->dataEngine->package();
}

KPluginInfo DataEngineScript::description() const
{
    Q_ASSERT(d->dataEngine);
    return d->dataEngine->d->dataEngineDescription;
}

void DataEngineScript::setData(const QString &source, const QString &key,
                               const QVariant &value)
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

void DataEngineScript::setMaxSourceCount(uint limit)
{
    if (d->dataEngine) {
        d->dataEngine->setMaxSourceCount(limit);
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

void DataEngineScript::setName(const QString &name)
{
    if (d->dataEngine) {
        d->dataEngine->setName(name);
    }
}

void DataEngineScript::setIcon(const QString &icon)
{
    if (d->dataEngine) {
        d->dataEngine->setIcon(icon);
    }
}

void DataEngineScript::scheduleSourcesUpdated()
{
    if (d->dataEngine) {
        d->dataEngine->scheduleSourcesUpdated();
    }
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

#include "dataenginescript.moc"
