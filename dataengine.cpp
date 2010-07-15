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

#include <QQueue>
#include <QTimer>
#include <QTime>
#include <QTimerEvent>
#include <QVariant>

#include <kdebug.h>
#include <kplugininfo.h>
#include <kservice.h>
#include <kstandarddirs.h>

#include "authorizationmanager.h"
#include "datacontainer.h"
#include "package.h"
#include "service.h"
#include "scripting/dataenginescript.h"

#include "private/authorizationmanager_p.h"
#include "private/dataengineservice_p.h"
#include "private/remotedataengine_p.h"
#include "private/service_p.h"
#include "private/storage_p.h"

namespace Plasma
{

DataEngine::DataEngine(QObject *parent, KService::Ptr service)
    : QObject(parent),
      d(new DataEnginePrivate(this, service))
{
}

DataEngine::DataEngine(QObject *parent, const QVariantList &args)
    : QObject(parent),
      d(new DataEnginePrivate(this, KService::serviceByStorageId(args.count() > 0 ? args[0].toString() : QString())))
{
}

DataEngine::~DataEngine()
{
    //kDebug() << objectName() << ": bye bye birdy! ";
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
        Service * s = d->script->serviceForSource(source);
        if (s) {
            return s;
        }
    }

    return new NullService(source, this);
}

void DataEngine::connectSource(const QString &source, QObject *visualization,
                               uint pollingInterval,
                               Plasma::IntervalAlignment intervalAlignment) const
{
    //kDebug() << "connectSource" << source;
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
        //kDebug() << " ==> source connected";
    }
}

void DataEngine::connectAllSources(QObject *visualization, uint pollingInterval,
                                   Plasma::IntervalAlignment intervalAlignment) const
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

DataEngine::Data DataEngine::query(const QString &source) const
{
    bool newSource;
    DataContainer *s = d->requestSource(source, &newSource);

    if (!s) {
        return DataEngine::Data();
    } else if (!newSource && d->minPollingInterval >= 0 &&
               s->timeSinceLastUpdate() >= uint(d->minPollingInterval)) {
        DataEngine *unconstThis = const_cast<DataEngine*>(this);
        if (unconstThis->updateSourceEvent(source)) {
            unconstThis->scheduleSourcesUpdated();
        }
    }

    DataEngine::Data data = s->data();
    s->checkUsage();
    return data;
}

void DataEngine::init()
{
    if (d->script) {
        d->setupScriptSupport();
        d->script->init();
    } else {
        // kDebug() << "called";
        // default implementation does nothing. this is for engines that have to
        // start things in motion external to themselves before they can work
    }
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
        //kDebug() << source;
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

    if (isNew) {
        emit sourceAdded(source);
    }

    if (s->isStorageEnabled()) {
        //FIXME: broken. one can't pass in parameters to a slot like this.
        // this probably belongs in DataContainer in any case
        QTimer::singleShot(180000, this, SLOT(storeSource(objectName())));
    }

    scheduleSourcesUpdated();
}

void DataEngine::setData(const QString &source, const Data &data)
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

    if (s->isStorageEnabled()) {
        //FIXME: broken. one can't pass in parameters to a slot like this.
        // this probably belongs in DataContainer in any case
        QTimer::singleShot(180000, this, SLOT(storeSource(objectName())));
    }

    if (isNew) {
        emit sourceAdded(source);
    }

    scheduleSourcesUpdated();
}

void DataEngine::removeAllData(const QString &source)
{
    DataContainer *s = d->source(source, false);
    if (s) {
        s->removeAllData();
        scheduleSourcesUpdated();
    }
}

void DataEngine::removeData(const QString &source, const QString &key)
{
    DataContainer *s = d->source(source, false);
    if (s) {
        s->setData(key, QVariant());
        scheduleSourcesUpdated();
    }
}

void DataEngine::addSource(DataContainer *source)
{
    if (d->sources.contains(source->objectName())) {
        kDebug() << "source named \"" << source->objectName() << "\" already exists.";
        return;
    }

    QObject::connect(source, SIGNAL(updateRequested(DataContainer*)),
                     this, SLOT(internalUpdateSource(DataContainer*)));
    QObject::connect(source, SIGNAL(destroyed(QObject*)), this, SLOT(sourceDestroyed(QObject*)));
    d->sources.insert(source->objectName(), source);
    emit sourceAdded(source->objectName());
    scheduleSourcesUpdated();
}

void DataEngine::setMaxSourceCount(uint limit)
{
    if (d->limit == limit) {
        return;
    }

    d->limit = limit;

    if (d->limit > 0) {
        d->trimQueue();
    } else {
        d->sourceQueue.clear();
    }
}

uint DataEngine::maxSourceCount() const
{
    return d->limit;
}

void DataEngine::setMinimumPollingInterval(int minimumMs)
{
    if (minimumMs < 0) {
        minimumMs = 0;
    }

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
    //kDebug() << "removing source " << source;
    SourceDict::iterator it = d->sources.find(source);
    if (it != d->sources.end()) {
        DataContainer *s = it.value();

        // remove it from the limit queue if we're keeping one
        if (d->limit > 0) {
            QQueue<DataContainer*>::iterator it = d->sourceQueue.begin();
            while (it != d->sourceQueue.end()) {
                if (*it == s) {
                    d->sourceQueue.erase(it);
                    break;
                }
                ++it;
            }
        }

        d->storeSource(s);
        s->disconnect(this);
        s->deleteLater();
        d->sources.erase(it);
        emit sourceRemoved(source);
    }
}

void DataEngine::removeAllSources()
{
    QMutableHashIterator<QString, Plasma::DataContainer*> it(d->sources);
    while (it.hasNext()) {
        it.next();
        const QString source = it.key();
        Plasma::DataContainer *s = it.value();
        it.remove();
        emit sourceRemoved(source);
        delete s;
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

DataEngine::SourceDict DataEngine::containerDict() const
{
    return d->sources;
}

void DataEngine::timerEvent(QTimerEvent *event)
{
    //kDebug();
    if (event->timerId() == d->updateTimerId) {
        // if the freq update is less than 0, don't bother
        if (d->minPollingInterval < 0) {
            //kDebug() << "uh oh.. no polling allowed!";
            return;
        }

        // minPollingInterval
        if (d->updateTimestamp.elapsed() < d->minPollingInterval) {
            //kDebug() << "hey now.. slow down!";
            return;
        }

        d->updateTimestamp.restart();
        updateAllSources();
    } else if (event->timerId() == d->checkSourcesTimerId) {
        killTimer(d->checkSourcesTimerId);
        d->checkSourcesTimerId = 0;

        QHashIterator<QString, Plasma::DataContainer*> it(d->sources);
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
    QHashIterator<QString, Plasma::DataContainer*> it(d->sources);
    while (it.hasNext()) {
        it.next();
        //kDebug() << "updating" << it.key();
        updateSourceEvent(it.key());
    }

    scheduleSourcesUpdated();
}

void DataEngine::forceImmediateUpdateOfAllVisualizations()
{
    foreach (DataContainer *source, d->sources) {
        source->forceImmediateUpdate();
    }
}

void DataEngine::setIcon(const QString &icon)
{
    d->icon = icon;
}

QString DataEngine::icon() const
{
    return d->icon;
}

QString DataEngine::pluginName() const
{
    if (!d->dataEngineDescription.isValid()) {
        return QString();
    }

    return d->dataEngineDescription.pluginName();
}

void DataEngine::setDefaultService(const QString &serviceName)
{
    d->serviceName = serviceName;
}

Service* DataEngine::createDefaultService(QObject *parent)
{
    QVariantList args;
    args << QVariant::fromValue<DataEngine*>(this);
    return Service::load(d->serviceName, args, parent);
}

void DataEnginePrivate::publish(AnnouncementMethods methods, const QString &name)
{
    if (!publishedService) {
        publishedService = new DataEngineService(q);
    }

    //QString resourceName =
    //i18nc("%1 is the name of a dataengine, %2 the name of the machine that engine is published
//on",
          //"%1 dataengine on %2", name(), AuthorizationManager::self()->d->myCredentials.name());
    kDebug() << "name: " << name;
    publishedService->d->publish(methods, name);
}

void DataEnginePrivate::unpublish(const QString &name)
{
    Q_UNUSED(name)

    if (publishedService) {
        publishedService->d->unpublish();
    }
}

bool DataEnginePrivate::isPublished() const
{
    if (publishedService) {
        return publishedService->d->isPublished();
    } else {
        return false;
    }
}

const Package *DataEngine::package() const
{
    return d->package;
}

void DataEngine::scheduleSourcesUpdated()
{
    if (d->checkSourcesTimerId) {
        return;
    }

    d->checkSourcesTimerId = startTimer(0);
}

QString DataEngine::name() const
{
    return d->engineName;
}

void DataEngine::setName(const QString &name)
{
    d->engineName = name;
    setObjectName(name);
}

void DataEngine::setStorageEnable(const QString &source, bool store)
{
    DataContainer *s = d->source(source, false);
    s->setStorageEnable(store);
}

// Private class implementations
DataEnginePrivate::DataEnginePrivate(DataEngine *e, KService::Ptr service)
    : q(e),
      dataEngineDescription(service),
      refCount(-1), // first ref
      checkSourcesTimerId(0),
      updateTimerId(0),
      minPollingInterval(-1),
      limit(0),
      valid(true),
      script(0),
      package(0),
      publishedService(0)
{
    updateTimestamp.start();

    if (!service) {
        engineName = i18n("Unnamed");
        return;
    }

    engineName = service->name();
    if (engineName.isEmpty()) {
        engineName = i18n("Unnamed");
    }
    e->setObjectName(engineName);
    icon = service->icon();

    if (dataEngineDescription.isValid()) {
        QString api = dataEngineDescription.property("X-Plasma-API").toString();

        if (!api.isEmpty()) {
            const QString path =
                KStandardDirs::locate("data",
                                      "plasma/dataengines/" + dataEngineDescription.pluginName() + '/');
            PackageStructure::Ptr structure =
                Plasma::packageStructure(api, Plasma::DataEngineComponent);
            structure->setPath(path);
            package = new Package(path, structure);

            script = Plasma::loadScriptEngine(api, q);
            if (!script) {
                kDebug() << "Could not create a" << api << "ScriptEngine for the"
                        << dataEngineDescription.name() << "DataEngine.";
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
        //kDebug() << "internal update source is delaying" << source->timeSinceLastUpdate() << minPollingInterval;
        //but fake an update so that the signalrelay that triggered this gets the data from the
        //recent update. this way we don't have to worry about queuing - the relay will send a
        //signal immediately and everyone else is undisturbed.
        source->setNeedsUpdate();
        return;
    }

    if (q->updateSourceEvent(source->objectName())) {
        //kDebug() << "queuing an update";
        q->scheduleSourcesUpdated();
    }/* else {
        kDebug() << "no update";
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
    DataEngine::SourceDict::const_iterator it = sources.constFind(sourceName);
    if (it != sources.constEnd()) {
        DataContainer *s = it.value();
        if (limit > 0) {
            QQueue<DataContainer*>::iterator it = sourceQueue.begin();
            while (it != sourceQueue.end()) {
                if (*it == s) {
                    sourceQueue.erase(it);
                    break;
                }
                ++it;
            }
            sourceQueue.enqueue(s);
        }
        return it.value();
    }

    if (!createWhenMissing) {
        return 0;
    }

    //kDebug() << "DataEngine " << q->objectName() << ": could not find DataContainer " << sourceName << ", creating";
    DataContainer *s = new DataContainer(q);
    s->setObjectName(sourceName);
    sources.insert(sourceName, s);
    QObject::connect(s, SIGNAL(destroyed(QObject *)), q, SLOT(sourceDestroyed(QObject *)));
    QObject::connect(s, SIGNAL(updateRequested(DataContainer*)),
                     q, SLOT(internalUpdateSource(DataContainer*)));

    if (limit > 0) {
        trimQueue();
        sourceQueue.enqueue(s);
    }
    return s;
}

void DataEnginePrivate::connectSource(DataContainer *s, QObject *visualization,
                                      uint pollingInterval,
                                      Plasma::IntervalAlignment align,
                                      bool immediateCall)
{
    //kDebug() << "connect source called" << s->objectName() << "with interval" << pollingInterval;

    //FIXME: at the moment a remote dataengine can only poll, a push mechanism will be needed instead
    if (pollingInterval == 0 && qobject_cast<RemoteDataEngine *>(q)) {
        pollingInterval = 5000;
    }
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
        //kDebug() << "immediate call requested, we have:" << s->visualizationIsConnected(visualization);
        immediateCall = !s->data().isEmpty() &&
                        !s->visualizationIsConnected(visualization);
    }

    retrieveStoredData(s);
    s->connectVisualization(visualization, pollingInterval, align);

    if (immediateCall) {
        QMetaObject::invokeMethod(visualization, "dataUpdated",
                                  Q_ARG(QString, s->objectName()),
                                  Q_ARG(Plasma::DataEngine::Data, s->data()));
    }
}

void DataEnginePrivate::sourceDestroyed(QObject *object)
{
    DataEngine::SourceDict::iterator it = sources.begin();
    while (it != sources.end()) {
        if (it.value() == object) {
            sources.erase(it);
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

    //kDebug() << "requesting source " << sourceName;
    DataContainer *s = source(sourceName, false);

    if (!s) {
        // we didn't find a data source, so give the engine an opportunity to make one
        /*kDebug() << "DataEngine " << q->objectName()
            << ": could not find DataContainer " << sourceName
            << " will create on request" << endl;*/
        if (q->sourceRequestEvent(sourceName)) {
            s = source(sourceName, false);
            if (s) {
                // now we have a source; since it was created on demand, assume
                // it should be removed when not used
                if (newSource) {
                    *newSource = true;
                }
                QObject::connect(s, SIGNAL(becameUnused(QString)), q, SLOT(removeSource(QString)));
            }
        }
    }

    return s;
}

void DataEnginePrivate::trimQueue()
{
    uint queueCount = sourceQueue.count();
    while (queueCount >= limit && !sourceQueue.isEmpty()) {
        DataContainer *punted = sourceQueue.dequeue();
        q->removeSource(punted->objectName());
        queueCount = sourceQueue.count();
    }
}

// put all setup routines for script here. at this point we can assume that
// package exists and that we have a script engine
void DataEnginePrivate::setupScriptSupport()
{
    if (!package) {
        return;
    }

    kDebug() << "setting up script support, package is in" << package->path()
             << "which is a" << package->structure()->type() << "package"
             << ", main script is" << package->filePath("mainscript");

    QString translationsPath = package->filePath("translations");
    if (!translationsPath.isEmpty()) {
        //FIXME: we should _probably_ use a KComponentData to segregate the applets
        //       from each other; but I want to get the basics working first :)
        KGlobal::dirs()->addResourceDir("locale", translationsPath);
        KGlobal::locale()->insertCatalog(package->metadata().pluginName());
    }
}

void DataEnginePrivate::storeAllSources()
{
    //kDebug() << "cache all sources";
    foreach (QString sourceName, q->sources()) {
        DataContainer *s = q->d->source(sourceName, false);
        storeSource(s);
    }
}

void DataEnginePrivate::storeSource(DataContainer *s) const
{
    return;
    if (!s || !s->needsToBeStored()) {
        return;
    }

    // FIXME: this should probably be moved into DataContainer, e.g. DataContainer::store()
    s->setNeedsToBeStored(false);
    Storage* store = new Storage(q->name(), 0);
    KConfigGroup op = store->operationDescription("save");
    op.writeEntry("source", s->objectName());
    DataEngine::Data dataToStore = s->data();
    DataEngine::Data::const_iterator it = dataToStore.constBegin();
    //FIXME: this shouldn't be one job per key/value pair, but one job for the entire container
    while (it != dataToStore.constEnd() && dataToStore.constEnd() == s->data().constEnd()) {
        QVariant v = it.value();
        if ((it.value().type() == QVariant::String) || (it.value().type() == QVariant::Int)) {
            op.writeEntry("key", it.key());
            op.writeEntry("data", it.value());
        } else {
            QByteArray b;
            QDataStream ds(&b, QIODevice::WriteOnly);
            ds << it.value();
            op.writeEntry("key", "base64-" + it.key());
            op.writeEntry("data", b.toBase64());
        }
        ++it;
        ServiceJob* job = store->startOperationCall(op);
        job->start();
    }

    //FIXME: this may result in the service being deleted before the jobs ... resulting in the
    //jobs potentially being terminated prematurely
    store->deleteLater();
}

void DataEnginePrivate::retrieveStoredData(DataContainer *s)
{
    if (!s || !s->isStorageEnabled()) {
        return;
    }

    //FIXME: this should probably move into DataContainer, and be triggered in setStorageEnabled
    // when it is set to true
    Storage* store = new Storage(q->name(), 0);
    KConfigGroup ret = store->operationDescription("retrieve");
    ret.writeEntry("source", s->objectName());
    ServiceJob* retrieve = store->startOperationCall(ret);
    QVariant retrievedData;
    retrieve->start();
    while (!retrieve->error()) {
        QHash<QString, QVariant> h = retrieve->result().toHash();
        QString key = h["key"].value<QString>();
        if (key.startsWith("base64-")) {
            QByteArray b = QByteArray::fromBase64(h["data"].toString().toAscii());
            QDataStream ds(&b, QIODevice::ReadOnly);
            QVariant v(ds);
            key.remove(0, 7);
            retrievedData = v;
        } else {
            retrievedData = h["data"];
        }
        s->setData(key, retrievedData);
        retrieve->start();
    }
    s->checkForUpdate();
}

}

#include "dataengine.moc"
