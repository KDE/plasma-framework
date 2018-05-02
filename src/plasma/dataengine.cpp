/*
 *   Copyright 2006-2007 Aaron Seigo <aseigo@kde.org>
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

#include "dataengine.h"
#include "private/dataengine_p.h"
#include "private/datacontainer_p.h"

#include <QAbstractItemModel>
#include <QQueue>
#include <QTimer>
#include <QTime>
#include <QTimerEvent>
#include <QVariant>

#include <QDebug>
#include <QStandardPaths>

#include <kplugininfo.h>
#include <kservice.h>
#include <klocalizedstring.h>

#include "datacontainer.h"
#include "package.h"
#include "pluginloader.h"
#include "service.h"
#include "scripting/dataenginescript.h"

#include "private/datacontainer_p.h"
#include "private/service_p.h"
#include "private/storage_p.h"
#include "config-plasma.h"

namespace Plasma
{

DataEngine::DataEngine(const KPluginInfo &plugin, QObject *parent)
    : QObject(parent),
      d(new DataEnginePrivate(this, plugin))
{
    if (d->script) {
        d->setupScriptSupport();
        d->script->init();
    } else {
        // default implementation does nothing. this is for engines that have to
        // start things in motion external to themselves before they can work
    }
}

DataEngine::DataEngine(QObject *parent, const QVariantList &args)
    : QObject(parent),
      d(new DataEnginePrivate(this, KPluginInfo(args), args))
{
    if (d->script) {
        d->setupScriptSupport();
        d->script->init();
    }
}

DataEngine::~DataEngine()
{
    //qCDebug(LOG_PLASMA) << objectName() << ": bye bye birdy! ";
    delete d;
}

QStringList DataEngine::sources() const
{
    if (d->script) {
        return d->script->sources();
    } else {
        return d->sources.keys();
    }
}

Service *DataEngine::serviceForSource(const QString &source)
{
    if (d->script) {
        Service *s = d->script->serviceForSource(source);
        if (s) {
            return s;
        }
    }

    return new NullService(source, this);
}

KPluginInfo DataEngine::pluginInfo() const
{
    return d->dataEngineDescription;
}

void DataEngine::connectSource(const QString &source, QObject *visualization,
                               uint pollingInterval,
                               Plasma::Types::IntervalAlignment intervalAlignment) const
{
    //qCDebug(LOG_PLASMA) << "connectSource" << source;
    bool newSource;
    DataContainer *s = d->requestSource(source, &newSource);

    if (s) {
        // we suppress the immediate invocation of dataUpdated here if the
        // source was prexisting and they don't request delayed updates
        // (we want to do an immediate update in that case so they don't
        // have to wait for the first time out)
        if (newSource && !s->data().isEmpty()) {
            newSource = false;
        }
        d->connectSource(s, visualization, pollingInterval, intervalAlignment,
                         !newSource || pollingInterval > 0);
        //qCDebug(LOG_PLASMA) << " ==> source connected";
    }
}

void DataEngine::connectAllSources(QObject *visualization, uint pollingInterval,
                                   Plasma::Types::IntervalAlignment intervalAlignment) const
{
    foreach (DataContainer *s, d->sources) {
        d->connectSource(s, visualization, pollingInterval, intervalAlignment);
    }
}

void DataEngine::disconnectSource(const QString &source, QObject *visualization) const
{
    DataContainer *s = d->source(source, false);

    if (s) {
        s->disconnectVisualization(visualization);
    }
}

DataContainer *DataEngine::containerForSource(const QString &source)
{
    return d->source(source, false);
}

bool DataEngine::sourceRequestEvent(const QString &name)
{
    if (d->script) {
        return d->script->sourceRequestEvent(name);
    } else {
        return false;
    }
}

bool DataEngine::updateSourceEvent(const QString &source)
{
    if (d->script) {
        return d->script->updateSourceEvent(source);
    } else {
        //qCDebug(LOG_PLASMA) << source;
        return false; //TODO: should this be true to trigger, even needless, updates on every tick?
    }
}

void DataEngine::setData(const QString &source, const QVariant &value)
{
    setData(source, source, value);
}

void DataEngine::setData(const QString &source, const QString &key, const QVariant &value)
{
    DataContainer *s = d->source(source, false);
    bool isNew = !s;

    if (isNew) {
        s = d->source(source);
    }

    s->setData(key, value);

    if (isNew && source != d->waitingSourceRequest) {
        emit sourceAdded(source);
    }

    d->scheduleSourcesUpdated();
}

void DataEngine::setData(const QString &source, const QVariantMap &data)
{
    DataContainer *s = d->source(source, false);
    bool isNew = !s;

    if (isNew) {
        s = d->source(source);
    }

    Data::const_iterator it = data.constBegin();
    while (it != data.constEnd()) {
        s->setData(it.key(), it.value());
        ++it;
    }

    if (isNew && source != d->waitingSourceRequest) {
        emit sourceAdded(source);
    }

    d->scheduleSourcesUpdated();
}

void DataEngine::removeAllData(const QString &source)
{
    DataContainer *s = d->source(source, false);
    if (s) {
        s->removeAllData();
        d->scheduleSourcesUpdated();
    }
}

void DataEngine::removeData(const QString &source, const QString &key)
{
    DataContainer *s = d->source(source, false);
    if (s) {
        s->setData(key, QVariant());
        d->scheduleSourcesUpdated();
    }
}

void DataEngine::setModel(const QString &source, QAbstractItemModel *model)
{
    if (model) {
        setData(source, QStringLiteral("HasModel"), true);
    } else {
        removeData(source, QStringLiteral("HasModel"));
    }

    Plasma::DataContainer *s = containerForSource(source);

    if (s) {
        s->setModel(model);
    }
}

QAbstractItemModel *DataEngine::modelForSource(const QString &source)
{
    Plasma::DataContainer *s = containerForSource(source);

    if (s) {
        return s->model();
    } else {
        return 0;
    }
}

void DataEngine::addSource(DataContainer *source)
{
    if (d->sources.contains(source->objectName())) {
#ifndef NDEBUG
        // qCDebug(LOG_PLASMA) << "source named \"" << source->objectName() << "\" already exists.";
#endif
        return;
    }

    QObject::connect(source, SIGNAL(updateRequested(DataContainer*)),
                     this, SLOT(internalUpdateSource(DataContainer*)));
    QObject::connect(source, SIGNAL(destroyed(QObject*)), this, SLOT(sourceDestroyed(QObject*)));
    d->sources.insert(source->objectName(), source);
    emit sourceAdded(source->objectName());
    d->scheduleSourcesUpdated();
}

void DataEngine::setMinimumPollingInterval(int minimumMs)
{
    d->minPollingInterval = minimumMs;
}

int DataEngine::minimumPollingInterval() const
{
    return d->minPollingInterval;
}

void DataEngine::setPollingInterval(uint frequency)
{
    killTimer(d->updateTimerId);
    d->updateTimerId = 0;

    if (frequency > 0) {
        d->updateTimerId = startTimer(frequency);
    }
}

void DataEngine::removeSource(const QString &source)
{
    QHash<QString, DataContainer *>::iterator it = d->sources.find(source);
    if (it != d->sources.end()) {
        DataContainer *s = it.value();
        s->d->store();
        d->sources.erase(it);
        s->disconnect(this);
        s->deleteLater();
        emit sourceRemoved(source);
    }
}

void DataEngine::removeAllSources()
{
    QMutableHashIterator<QString, Plasma::DataContainer *> it(d->sources);
    while (it.hasNext()) {
        it.next();
        Plasma::DataContainer *s = it.value();
        const QString source = it.key();
        it.remove();
        s->disconnect(this);
        s->deleteLater();
        emit sourceRemoved(source);
    }
}

bool DataEngine::isValid() const
{
    return d->valid;
}

bool DataEngine::isEmpty() const
{
    return d->sources.isEmpty();
}

void DataEngine::setValid(bool valid)
{
    d->valid = valid;
}

QHash<QString, DataContainer *> DataEngine::containerDict() const
{
    return d->sources;
}

void DataEngine::timerEvent(QTimerEvent *event)
{
    //qCDebug(LOG_PLASMA);
    if (event->timerId() == d->updateTimerId) {
        // if the freq update is less than 0, don't bother
        if (d->minPollingInterval < 0) {
            //qCDebug(LOG_PLASMA) << "uh oh.. no polling allowed!";
            return;
        }

        // minPollingInterval
        if (d->updateTimer.elapsed() < d->minPollingInterval) {
            //qCDebug(LOG_PLASMA) << "hey now.. slow down!";
            return;
        }

        d->updateTimer.start();
        updateAllSources();
    } else if (event->timerId() == d->checkSourcesTimerId) {
        killTimer(d->checkSourcesTimerId);
        d->checkSourcesTimerId = 0;

        QHashIterator<QString, Plasma::DataContainer *> it(d->sources);
        while (it.hasNext()) {
            it.next();
            it.value()->checkForUpdate();
        }
    } else {
        QObject::timerEvent(event);
    }
}

void DataEngine::updateAllSources()
{
    QHashIterator<QString, Plasma::DataContainer *> it(d->sources);
    while (it.hasNext()) {
        it.next();
        //qCDebug(LOG_PLASMA) << "updating" << it.key();
        if (it.value()->isUsed()) {
            updateSourceEvent(it.key());
        }
    }

    d->scheduleSourcesUpdated();
}

void DataEngine::forceImmediateUpdateOfAllVisualizations()
{
    foreach (DataContainer *source, d->sources) {
        if (source->isUsed()) {
            source->forceImmediateUpdate();
        }
    }
}

Package DataEngine::package() const
{
    return d->package ? *d->package : Package();
}

void DataEngine::setStorageEnabled(const QString &source, bool store)
{
    DataContainer *s = d->source(source, false);
    if (s) {
        s->setStorageEnabled(store);
    }
}

// Private class implementations
DataEnginePrivate::DataEnginePrivate(DataEngine *e, const KPluginInfo &info, const QVariantList &args)
    : q(e),
      dataEngineDescription(info),
      refCount(-1), // first ref
      checkSourcesTimerId(0),
      updateTimerId(0),
      minPollingInterval(-1),
      valid(false),
      script(0),
      package(0)
{
    updateTimer.start();

    if (dataEngineDescription.isValid()) {
        valid = true;
        e->setObjectName(dataEngineDescription.name());
    }

    if (dataEngineDescription.isValid()) {
        QString api = dataEngineDescription.property(QStringLiteral("X-Plasma-API")).toString();

        if (!api.isEmpty()) {
            const QString path =
                QStandardPaths::locate(QStandardPaths::GenericDataLocation,
                                       QStringLiteral(PLASMA_RELATIVE_DATA_INSTALL_DIR "/dataengines/") + dataEngineDescription.pluginName(),
                                       QStandardPaths::LocateDirectory);
            package = new Package(PluginLoader::self()->loadPackage(QStringLiteral("Plasma/DataEngine"), api));
            package->setPath(path);

            if (package->isValid()) {
                script = Plasma::loadScriptEngine(api, q, args);
            }

            if (!script) {
#ifndef NDEBUG
                // qCDebug(LOG_PLASMA) << "Could not create a" << api << "ScriptEngine for the"
                //        << dataEngineDescription.name() << "DataEngine.";
#endif
                delete package;
                package = 0;
            }
        }
    }
}

DataEnginePrivate::~DataEnginePrivate()
{
    delete script;
    script = 0;
    delete package;
    package = 0;
}

void DataEnginePrivate::internalUpdateSource(DataContainer *source)
{
    if (minPollingInterval > 0 &&
            source->timeSinceLastUpdate() < (uint)minPollingInterval) {
        // skip updating this source; it's been too soon
        //qCDebug(LOG_PLASMA) << "internal update source is delaying" << source->timeSinceLastUpdate() << minPollingInterval;
        //but fake an update so that the signalrelay that triggered this gets the data from the
        //recent update. this way we don't have to worry about queuing - the relay will send a
        //signal immediately and everyone else is undisturbed.
        source->setNeedsUpdate();
        return;
    }

    if (q->updateSourceEvent(source->objectName())) {
        //qCDebug(LOG_PLASMA) << "queuing an update";
        scheduleSourcesUpdated();
    }/* else {
#ifndef NDEBUG
        // qCDebug(LOG_PLASMA) << "no update";
#endif
    }*/
}

void DataEnginePrivate::ref()
{
    --refCount;
}

void DataEnginePrivate::deref()
{
    ++refCount;
}

bool DataEnginePrivate::isUsed() const
{
    return refCount != 0;
}

DataContainer *DataEnginePrivate::source(const QString &sourceName, bool createWhenMissing)
{
    QHash<QString, DataContainer *>::const_iterator it = sources.constFind(sourceName);
    if (it != sources.constEnd()) {
        DataContainer *s = it.value();
        return s;
    }

    if (!createWhenMissing) {
        return 0;
    }

    //qCDebug(LOG_PLASMA) << "DataEngine " << q->objectName() << ": could not find DataContainer " << sourceName << ", creating";
    DataContainer *s = new DataContainer(q);
    s->setObjectName(sourceName);
    sources.insert(sourceName, s);
    QObject::connect(s, SIGNAL(destroyed(QObject*)), q, SLOT(sourceDestroyed(QObject*)));
    QObject::connect(s, SIGNAL(updateRequested(DataContainer*)),
                     q, SLOT(internalUpdateSource(DataContainer*)));

    return s;
}

void DataEnginePrivate::connectSource(DataContainer *s, QObject *visualization,
                                      uint pollingInterval,
                                      Plasma::Types::IntervalAlignment align,
                                      bool immediateCall)
{
    //qCDebug(LOG_PLASMA) << "connect source called" << s->objectName() << "with interval" << pollingInterval;

    if (pollingInterval > 0) {
        // never more frequently than allowed, never more than 20 times per second
        uint min = qMax(50, minPollingInterval); // for qMax below
        pollingInterval = qMax(min, pollingInterval);

        // align on the 50ms
        pollingInterval = pollingInterval - (pollingInterval % 50);
    }

    if (immediateCall) {
        // we don't want to do an immediate call if we are simply
        // reconnecting
        //qCDebug(LOG_PLASMA) << "immediate call requested, we have:" << s->visualizationIsConnected(visualization);
        immediateCall = !s->data().isEmpty() &&
                        !s->visualizationIsConnected(visualization);
    }

    s->connectVisualization(visualization, pollingInterval, align);

    if (immediateCall) {
        QMetaObject::invokeMethod(visualization, "dataUpdated",
                                  Q_ARG(QString, s->objectName()),
                                  Q_ARG(Plasma::DataEngine::Data, s->data()));
        if (s->d->model) {
            QMetaObject::invokeMethod(visualization, "modelChanged",
                                      Q_ARG(QString, s->objectName()),
                                      Q_ARG(QAbstractItemModel*, s->d->model.data()));
        }
        s->d->dirty = false;
    }
}

void DataEnginePrivate::sourceDestroyed(QObject *object)
{
    QHash<QString, DataContainer *>::iterator it = sources.begin();
    while (it != sources.end()) {
        if (it.value() == object) {
            sources.erase(it);
            emit q->sourceRemoved(object->objectName());
            break;
        }
        ++it;
    }
}

DataContainer *DataEnginePrivate::requestSource(const QString &sourceName, bool *newSource)
{
    if (newSource) {
        *newSource = false;
    }

    //qCDebug(LOG_PLASMA) << "requesting source " << sourceName;
    DataContainer *s = source(sourceName, false);

    if (!s) {
        // we didn't find a data source, so give the engine an opportunity to make one
        /*// qCDebug(LOG_PLASMA) << "DataEngine " << q->objectName()
            << ": could not find DataContainer " << sourceName
            << " will create on request" << endl;*/
        waitingSourceRequest = sourceName;
        if (q->sourceRequestEvent(sourceName)) {
            s = source(sourceName, false);
            if (s) {
                // now we have a source; since it was created on demand, assume
                // it should be removed when not used
                if (newSource) {
                    *newSource = true;
                }
                QObject::connect(s, SIGNAL(becameUnused(QString)), q, SLOT(removeSource(QString)));
                emit q->sourceAdded(sourceName);
            }
        }
        waitingSourceRequest.clear();
    }

    return s;
}

// put all setup routines for script here. at this point we can assume that
// package exists and that we have a script engine
void DataEnginePrivate::setupScriptSupport()
{
    if (!package) {
        return;
    }

    /*
    #ifndef NDEBUG
    // qCDebug(LOG_PLASMA) << "sletting up script support, package is in" << package->path()
    #endif
             << "which is a" << package->structure()->type() << "package"
             << ", main script is" << package->filePath("mainscript");
    */

    // FIXME: Replace with ki18n functionality once semantics is clear.
    // const QString translationsPath = package->filePath("translations");
    // if (!translationsPath.isEmpty()) {
    //     KGlobal::dirs()->addResourceDir("locale", translationsPath);
    // }
}

void DataEnginePrivate::scheduleSourcesUpdated()
{
    if (checkSourcesTimerId) {
        return;
    }

    checkSourcesTimerId = q->startTimer(0);
}

}

#include "moc_dataengine.cpp"
