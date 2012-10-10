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
#include "datacontainer.h"
#include "private/datacontainer_p.h"
#include "private/storage_p.h"

#include <kdebug.h>

#include "plasma.h"

namespace Plasma
{

DataContainer::DataContainer(QObject *parent)
    : QObject(parent),
      d(new DataContainerPrivate(this))
{
}

DataContainer::~DataContainer()
{
    delete d;
}

const DataEngine::Data DataContainer::data() const
{
    return d->data;
}

void DataContainer::setData(const QString &key, const QVariant &value)
{
    if (!value.isValid()) {
        d->data.remove(key);
    } else {
        d->data.insert(key, value);
    }

    d->dirty = true;
    d->updateTs.start();

    //check if storage is enabled and if storage is needed.
    //If it is not set to be stored,then this is the first
    //setData() since the last time it was stored. This
    //gives us only one singleShot timer.
    if (isStorageEnabled() || !needsToBeStored()) {
        d->storageTimer.start(180000, this);
    }

    setNeedsToBeStored(true);
}

void DataContainer::removeAllData()
{
    if (d->data.isEmpty()) {
        // avoid an update if we don't have any data anyways
        return;
    }

    d->data.clear();
    d->dirty = true;
    d->updateTs.start();
}

bool DataContainer::visualizationIsConnected(QObject *visualization) const
{
    return d->relayObjects.contains(visualization);
}

void DataContainer::connectVisualization(QObject *visualization, uint pollingInterval,
                                         Plasma::IntervalAlignment alignment)
{
    //kDebug() << "connecting visualization" << visualization << "at interval of"
    //         << pollingInterval << "to" << objectName();
    QMap<QObject *, SignalRelay *>::iterator objIt = d->relayObjects.find(visualization);
    bool connected = objIt != d->relayObjects.end();
    if (connected) {
        // this visualization is already connected. just adjust the update
        // frequency if necessary
        SignalRelay *relay = objIt.value();
        if (relay) {
            // connected to a relay
            //kDebug() << "     already connected, but to a relay";
            if (relay->m_interval == pollingInterval) {
                //kDebug() << "    already connected to a relay of the same interval of"
                //          << pollingInterval << ", nothing to do";
                return;
            }

            if (relay->receiverCount() == 1) {
                //kDebug() << "    removing relay, as it is now unused";
                d->relays.remove(relay->m_interval);
                delete relay;
            } else {
                disconnect(relay, SIGNAL(dataUpdated(QString,Plasma::DataEngine::Data)),
                           visualization, SLOT(dataUpdated(QString,Plasma::DataEngine::Data)));
                //relay->isUnused();
            }
        } else if (pollingInterval < 1) {
            // the visualization was connected already, but not to a relay
            // and it still doesn't want to connect to a relay, so we have
            // nothing to do!
            //kDebug() << "     already connected, nothing to do";
            return;
        } else {
            disconnect(this, SIGNAL(dataUpdated(QString,Plasma::DataEngine::Data)),
                       visualization, SLOT(dataUpdated(QString,Plasma::DataEngine::Data)));
        }
    } else {
        connect(visualization, SIGNAL(destroyed(QObject*)),
                this, SLOT(disconnectVisualization(QObject*)));//, Qt::QueuedConnection);
    }

    if (pollingInterval < 1) {
        //kDebug() << "    connecting directly";
        d->relayObjects[visualization] = 0;
        connect(this, SIGNAL(dataUpdated(QString,Plasma::DataEngine::Data)),
                visualization, SLOT(dataUpdated(QString,Plasma::DataEngine::Data)));
    } else {
        //kDebug() << "    connecting to a relay";
        // we only want to do an imediate update if this is not the first object to connect to us
        // if it is the first visualization, then the source will already have been populated
        // engine's sourceRequested method
        bool immediateUpdate = connected || d->relayObjects.count() > 1;
        SignalRelay *relay = d->signalRelay(this, visualization, pollingInterval,
                                            alignment, immediateUpdate);
        connect(relay, SIGNAL(dataUpdated(QString,Plasma::DataEngine::Data)),
                visualization, SLOT(dataUpdated(QString,Plasma::DataEngine::Data)));
    }
}

void DataContainer::setStorageEnabled(bool store)
{
    QTime time = QTime::currentTime();
    qsrand((uint)time.msec());
    d->enableStorage = store;
    if (store) {
        QTimer::singleShot(qrand() % (2000 + 1) , this, SLOT(retrieve()));
    }
}

bool DataContainer::isStorageEnabled() const
{
    return d->enableStorage;
}

bool DataContainer::needsToBeStored() const
{
    return !d->isStored;
}

void DataContainer::setNeedsToBeStored(bool store)
{
    d->isStored = !store;
}

DataEngine* DataContainer::getDataEngine()
{
    QObject *o = this;
    DataEngine *de = NULL;
    while (de == NULL)
    {
        o = dynamic_cast<QObject *> (o->parent());
        if (o == NULL) {
            return NULL;
        }
        de = dynamic_cast<DataEngine *> (o);
    }
    return de;
}

void DataContainerPrivate::store()
{
    if (!q->needsToBeStored() || !q->isStorageEnabled()) {
        return;
    }

    DataEngine* de = q->getDataEngine();
    if (!de) {
        return;
    }

    q->setNeedsToBeStored(false);

    if (!storage) {
        storage = new Storage(q);
    }

    KConfigGroup op = storage->operationDescription("save");
    op.writeEntry("group", q->objectName());
    StorageJob *job = static_cast<StorageJob *>(storage->startOperationCall(op));
    job->setData(data);
    storageCount++;
    QObject::connect(job, SIGNAL(finished(KJob*)), q, SLOT(storeJobFinished(KJob*)));
}

void DataContainerPrivate::storeJobFinished(KJob* )
{
    --storageCount;
    if (storageCount < 1) {
        storage->deleteLater();
        storage = 0;
    }
}

void DataContainerPrivate::retrieve()
{
    DataEngine* de = q->getDataEngine();
    if (de == NULL) {
        return;
    }

    if (!storage) {
        storage = new Storage(q);
    }

    KConfigGroup retrieveGroup = storage->operationDescription("retrieve");
    retrieveGroup.writeEntry("group", q->objectName());
    ServiceJob* retrieveJob = storage->startOperationCall(retrieveGroup);
    QObject::connect(retrieveJob, SIGNAL(result(KJob*)), q,
            SLOT(populateFromStoredData(KJob*)));
}

void DataContainerPrivate::populateFromStoredData(KJob *job)
{
    if (job->error()) {
        return;
    }

    StorageJob *ret = dynamic_cast<StorageJob*>(job);
    if (!ret) {
        return;
    }

    // Only fill the source with old stored
    // data if it is not already populated with new data.
    if (data.isEmpty() && !ret->data().isEmpty()) {
        data = ret->data();
        dirty = true;
        q->forceImmediateUpdate();
    }

    KConfigGroup expireGroup = storage->operationDescription("expire");
    //expire things older than 4 days
    expireGroup.writeEntry("age", 345600);
    storage->startOperationCall(expireGroup);
}

void DataContainer::disconnectVisualization(QObject *visualization)
{
    QMap<QObject *, SignalRelay *>::iterator objIt = d->relayObjects.find(visualization);
    disconnect(visualization, SIGNAL(destroyed(QObject*)),
              this, SLOT(disconnectVisualization(QObject*)));//, Qt::QueuedConnection);

    if (objIt == d->relayObjects.end() || !objIt.value()) {
        // it is connected directly to the DataContainer itself
        disconnect(this, SIGNAL(dataUpdated(QString,Plasma::DataEngine::Data)),
                   visualization, SLOT(dataUpdated(QString,Plasma::DataEngine::Data)));
    } else {
        SignalRelay *relay = objIt.value();

        if (relay->receiverCount() == 1) {
            d->relays.remove(relay->m_interval);
            delete relay;
        } else {
            disconnect(relay, SIGNAL(dataUpdated(QString,Plasma::DataEngine::Data)),
                       visualization, SLOT(dataUpdated(QString,Plasma::DataEngine::Data)));
        }
    }

    d->relayObjects.erase(objIt);
    checkUsage();
}

void DataContainer::checkForUpdate()
{
    //kDebug() << objectName() << d->dirty;
    if (d->dirty) {
        emit dataUpdated(objectName(), d->data);

        foreach (SignalRelay *relay, d->relays) {
            relay->checkQueueing();
        }

        d->dirty = false;
    }
}

void DataContainer::forceImmediateUpdate()
{
    if (d->dirty) {
        d->dirty = false;
        emit dataUpdated(objectName(), d->data);
    }

    foreach (SignalRelay *relay, d->relays) {
        relay->forceImmediateUpdate();
    }
}

uint DataContainer::timeSinceLastUpdate() const
{
    //FIXME: we still assume it's been <24h
    //and ignore possible daylight savings changes
    return d->updateTs.elapsed();
}

void DataContainer::setNeedsUpdate(bool update)
{
    d->cached = update;
}

void DataContainer::checkUsage()
{
    if (!d->checkUsageTimer.isActive()) {
        d->checkUsageTimer.start(10, this);
    }
}

void DataContainer::timerEvent(QTimerEvent * event)
{
    if (event->timerId() == d->checkUsageTimer.timerId()) {
        if (d->relays.count() < 1 &&
            receivers(SIGNAL(dataUpdated(QString,Plasma::DataEngine::Data))) < 1) {
            // DO NOT CALL ANYTHING AFTER THIS LINE AS IT MAY GET DELETED!
            kDebug() << objectName() << "is unused";
            emit becameUnused(objectName());
        }
        d->checkUsageTimer.stop();
    } else if (event->timerId() == d->storageTimer.timerId()) {
        d->store();
        d->storageTimer.stop();
    }
}

} // Plasma namespace

#include "datacontainer.moc"

