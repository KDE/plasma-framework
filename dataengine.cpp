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

#include <QQueue>
#include <QTimer>
#include <QTime>
#include <QTimerEvent>
#include <QVariant>

#include <KDebug>
#include <KPluginInfo>
#include <KService>

#include "datacontainer.h"
#include "scripting/dataenginescript.h"

namespace Plasma
{

class DataEngine::Private
{
    public:
        Private(DataEngine* e, KService::Ptr service)
            : engine(e),
              ref(-1), // first ref
              updateTimerId(0),
              minUpdateInterval(-1),
              limit(0),
              valid(true),
              script(0)
        {
            updateTimer = new QTimer(engine);
            updateTimer->setSingleShot(true);
            updateTimestamp.start();

            if (!service) {
                return;
            }

            engineName = service->property("X-Plasma-EngineName").toString();
            e->setObjectName(engineName);
            icon = service->icon();

            KPluginInfo dataEngineDescription(service);
            if (dataEngineDescription.isValid()) {
                QString language = dataEngineDescription.property("X-Plasma-Language").toString();

                if (!language.isEmpty()) {
                    script = Plasma::loadScriptEngine(language, engine);
                    if (!script) {
                        kDebug() << "Could not create a" << language << "ScriptEngine for the"
                                << dataEngineDescription.name() << "DataEngine.";
                    }
                }
            }
        }

        DataContainer* source(const QString& sourceName, bool createWhenMissing = true)
        {
            DataEngine::SourceDict::const_iterator it = sources.find(sourceName);
            if (it != sources.constEnd()) {
                DataContainer* s = it.value();
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

            /*kDebug() << "DataEngine " << engine->objectName()
                     << ": could not find DataContainer " << sourceName
                     << ", creating" << endl;*/
            DataContainer* s = new DataContainer(engine);
            s->setObjectName(sourceName);
            sources.insert(sourceName, s);
            connect(s, SIGNAL(requestUpdate(DataContainer*)), engine, SLOT(internalUpdateSource(DataContainer*)));

            if (limit > 0) {
                trimQueue();
                sourceQueue.enqueue(s);
            }
            emit engine->newSource(sourceName);
            return s;
        }

        void connectSource(DataContainer* s, QObject* visualization, uint updateInterval,
                           Plasma::IntervalAlignment align, bool immediateCall = true)
        {
            //kDebug() << "connect source called with interval" << updateInterval;
            if (updateInterval > 0) {
                // never more frequently than allowed, never more than 20 times per second
                uint min = qMax(50, minUpdateInterval); // for qMin below
                updateInterval = qMax(min, updateInterval);

                // align on the 50ms
                updateInterval = updateInterval - (updateInterval % 50);
            }

            s->connectVisualization(visualization, updateInterval, align);

            if (immediateCall) {
                QMetaObject::invokeMethod(visualization, "dataUpdated",
                                          Q_ARG(QString, s->objectName()),
                                          Q_ARG(Plasma::DataEngine::Data, s->data()));
            }
        }

        DataContainer* requestSource(const QString& sourceName, bool* newSource = 0)
        {
            if (newSource) {
                *newSource = false;
            }

            //kDebug() << "requesting source " << sourceName;
            DataContainer* s = source(sourceName, false);

            if (!s) {
                // we didn't find a data source, so give the engine an opportunity to make one
                /*kDebug() << "DataEngine " << engine->objectName()
                    << ": could not find DataContainer " << sourceName
                    << " will create on request" << endl;*/
                if (engine->sourceRequested(sourceName)) {
                    s = source(sourceName, false);
                    if (s) {
                        // now we have a source; since it was created on demand, assume
                        // it should be removed when not used
                        if (newSource) {
                            *newSource = true;
                        }
                        connect(s, SIGNAL(unused(QString)), engine, SLOT(removeSource(QString)));
                    }
                }
            }

            return s;
        }

        void trimQueue()
        {
            uint queueCount = sourceQueue.count();
            while (queueCount >= limit) {
                DataContainer* punted = sourceQueue.dequeue();
                engine->removeSource(punted->objectName());
            }
        }

        void queueUpdate()
        {
            if (updateTimer->isActive()) {
                return;
            }
            updateTimer->start(0);
        }

        DataEngine* engine;
        int ref;
        int updateTimerId;
        int minUpdateInterval;
        QTime updateTimestamp;
        DataEngine::SourceDict sources;
        QQueue<DataContainer*> sourceQueue;
        QTimer* updateTimer;
        QString icon;
        uint limit;
        bool valid;
        DataEngineScript* script;
        QString engineName;
};


DataEngine::DataEngine(QObject* parent, KService::Ptr service)
    : QObject(parent),
      d(new Private(this, service))
{
    connect(d->updateTimer, SIGNAL(timeout()), this, SLOT(checkForUpdates()));
    init();
}

DataEngine::DataEngine(QObject* parent, const QVariantList& args)
    : QObject(parent),
      d(new Private(this, KService::serviceByStorageId(args.count() > 0 ? args[0].toString() : QString())))
{
    connect(d->updateTimer, SIGNAL(timeout()), this, SLOT(checkForUpdates()));
    init();
}

DataEngine::~DataEngine()
{
    //kDebug() << objectName() << ": bye bye birdy! ";
    delete d;
}

QStringList DataEngine::sources() const
{
    return d->sources.keys();
}

void DataEngine::connectSource(const QString& source, QObject* visualization,
                               uint updateInterval, Plasma::IntervalAlignment intervalAlignment) const
{
    //kDebug() << "connectSource" << source;
    bool newSource;
    DataContainer* s = d->requestSource(source, &newSource);

    if (s) {
        // we suppress the immediate invocation of dataUpdated here if the source was prexisting and they
        // don't request delayed updates (we want to do an immediate update in that case so they
        // don't have to wait for the first time out)
        d->connectSource(s, visualization, updateInterval, intervalAlignment, !newSource || updateInterval > 0);
        //kDebug() << " ==> source connected";
    }
}

void DataEngine::connectAllSources(QObject* visualization, uint updateInterval,
                                   Plasma::IntervalAlignment intervalAlignment) const
{
    foreach (DataContainer* s, d->sources) {
        d->connectSource(s, visualization, updateInterval, intervalAlignment);
    }
}

void DataEngine::disconnectSource(const QString& source, QObject* visualization) const
{
    DataContainer* s = d->source(source, false);

    if (s) {
        s->disconnectVisualization(visualization);
    }
}

DataContainer* DataEngine::containerForSource(const QString &source)
{
    return d->source(source, false);
}

DataEngine::Data DataEngine::query(const QString& source) const
{
    DataContainer* s = d->requestSource(source);

    if (!s) {
        return DataEngine::Data();
    }

    DataEngine::Data data = s->data();
    s->checkUsage();
    return data;
}

void DataEngine::internalUpdateSource(DataContainer* source)
{
    if (d->minUpdateInterval > 0 &&
        source->timeSinceLastUpdate() < d->minUpdateInterval) {
        // skip updating this source; it's been too soon
        //kDebug() << "internal update source is delaying" << source->timeSinceLastUpdate() << d->minUpdateInterval;
        //but fake an update so that the signalrelay that triggered this gets the data from the
        //recent update. this way we don't have to worry about queuing - the relay will send a
        //signal immediately and everyone else is undisturbed.
        source->setNeedsUpdate();
        return;
    }

    if (updateSource(source->objectName())) {
        d->queueUpdate();
    }
}

void DataEngine::init()
{
    if (d->script) {
        d->script->init();
    } else {
        // kDebug() << "DataEngine::init() called ";
        // default implementation does nothing. this is for engines that have to
        // start things in motion external to themselves before they can work
    }
}

bool DataEngine::sourceRequested(const QString &name)
{
    if (d->script) {
        return d->script->sourceRequested(name);
    } else {
        return false;
    }
}

bool DataEngine::updateSource(const QString& source)
{
    if (d->script) {
        return d->script->updateSource(source);
    } else {
        //kDebug() << "updateSource source" << endl;
        return false; //TODO: should this be true to trigger, even needless, updates on every tick?
    }
}

void DataEngine::setData(const QString& source, const QVariant& value)
{
    setData(source, source, value);
}

void DataEngine::setData(const QString& source, const QString& key, const QVariant& value)
{
    DataContainer* s = d->source(source);
    s->setData(key, value);
    d->queueUpdate();
}

void DataEngine::setData(const QString &source, const Data &data)
{
    DataContainer *s = d->source(source);
    Data::const_iterator it = data.constBegin();
    while (it != data.constEnd()) {
        s->setData(it.key(), it.value());
        ++it;
    }
    d->queueUpdate();
}


void DataEngine::clearData(const QString& source)
{
    DataContainer* s = d->source(source, false);
    if (s) {
        s->clearData();
        d->queueUpdate();
    }
}

void DataEngine::removeData(const QString& source, const QString& key)
{
    DataContainer* s = d->source(source, false);
    if (s) {
        s->setData(key, QVariant());
        d->queueUpdate();
    }
}

void DataEngine::addSource(DataContainer* source)
{
    SourceDict::const_iterator it = d->sources.find(source->objectName());
    if (it != d->sources.constEnd()) {
        kDebug() << "source named \"" << source->objectName() << "\" already exists.";
        return;
    }

    d->sources.insert(source->objectName(), source);
    emit newSource(source->objectName());
}

void DataEngine::setSourceLimit(uint limit)
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

void DataEngine::setMinimumUpdateInterval(int minimumMs)
{
    d->minUpdateInterval = minimumMs;
}

int DataEngine::minimumUpdateInterval() const
{
    return d->minUpdateInterval;
}

void DataEngine::setUpdateInterval(uint frequency)
{
    killTimer(d->updateTimerId);
    d->updateTimerId = 0;

    if (frequency > 0) {
        d->updateTimerId = startTimer(frequency);
    }
}

/*
NOTE: This is not implemented to prevent having to store the value internally.
      When there is a good use case for needing access to this value, we can
      add another member to the Private class and add this method.

void DataEngine::updateInterval()
{
    return d->updateInterval;
}
*/

void DataEngine::removeSource(const QString& source)
{
    //kDebug() << "removing source " << source;
    SourceDict::iterator it = d->sources.find(source);
    if (it != d->sources.end()) {
        emit sourceRemoved(it.key());
        it.value()->deleteLater();
        d->sources.erase(it);
    }
}

void DataEngine::clearSources()
{
    QMutableHashIterator<QString, Plasma::DataContainer*> it(d->sources);
    while (it.hasNext()) {
        it.next();
        emit sourceRemoved(it.key());
        delete it.value();
        it.remove();
    }
}

void DataEngine::ref()
{
    --d->ref;
}

void DataEngine::deref()
{
    ++d->ref;
}

bool DataEngine::isUsed() const
{
    return d->ref != 0;
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

DataEngine::SourceDict DataEngine::sourceDict() const
{
    return d->sources;
}

void DataEngine::timerEvent(QTimerEvent *event)
{
    if (event->timerId() != d->updateTimerId) {
        return;
    }

    event->accept();

    // if the freq update is less than 0, don't bother
    if (d->minUpdateInterval < 0) {
        return;
    }

    // minUpdateInterval
    if (d->updateTimestamp.elapsed() < d->minUpdateInterval) {
        return;
    }

    d->updateTimestamp.restart();
    QHashIterator<QString, Plasma::DataContainer*> it(d->sources);
    while (it.hasNext()) {
        it.next();
        updateSource(it.key());
    }
    checkForUpdates();
}

void DataEngine::setIcon(const QString& icon)
{
    d->icon = icon;
}

QString DataEngine::icon() const
{
    return d->icon;
}

void DataEngine::checkForUpdates()
{
    QHashIterator<QString, Plasma::DataContainer*> it(d->sources);
    while (it.hasNext()) {
        it.next();
        it.value()->checkForUpdate();
    }
}

QString DataEngine::engineName() const
{
    return d->engineName;
}

}

#include "dataengine.moc"
