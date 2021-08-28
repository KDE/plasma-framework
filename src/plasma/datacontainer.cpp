/*
    SPDX-FileCopyrightText: 2006-2007 Aaron Seigo <aseigo@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#include "datacontainer.h"
#include "private/datacontainer_p.h"
#include "private/storage_p.h"

#include <QAbstractItemModel>
#include <QDebug>
#include <QRandomGenerator>

#include "debug_p.h"
#include "plasma.h"

namespace Plasma
{
DataContainer::DataContainer(QObject *parent)
    : QObject(parent)
    , d(new DataContainerPrivate(this))
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
    d->updateTimer.start();

    // check if storage is enabled and if storage is needed.
    // If it is not set to be stored,then this is the first
    // setData() since the last time it was stored. This
    // gives us only one singleShot timer.
    if (isStorageEnabled() || !needsToBeStored()) {
        d->storageTimer.start(180000, this);
    }

    setNeedsToBeStored(true);
}

void DataContainer::setModel(QAbstractItemModel *model)
{
    if (d->model.data() == model) {
        return;
    }

    if (d->model) {
        d->model.data()->deleteLater();
    }

    d->model = model;
    model->setParent(this);
    Q_EMIT modelChanged(objectName(), model);
}

QAbstractItemModel *DataContainer::model()
{
    return d->model.data();
}

void DataContainer::removeAllData()
{
    if (d->data.isEmpty()) {
        // avoid an update if we don't have any data anyways
        return;
    }

    d->data.clear();
    d->dirty = true;
    d->updateTimer.start();
}

bool DataContainer::visualizationIsConnected(QObject *visualization) const
{
    return d->relayObjects.contains(visualization);
}

void DataContainer::connectVisualization(QObject *visualization, uint pollingInterval, Plasma::Types::IntervalAlignment alignment)
{
    // qCDebug(LOG_PLASMA) << "connecting visualization" <<this<< visualization << "at interval of"
    //         << pollingInterval << "to" << objectName();
    QMap<QObject *, SignalRelay *>::iterator objIt = d->relayObjects.find(visualization);
    bool connected = objIt != d->relayObjects.end();
    if (connected) {
        // this visualization is already connected. just adjust the update
        // frequency if necessary
        SignalRelay *relay = objIt.value();
        if (relay) {
            // connected to a relay
            // qCDebug(LOG_PLASMA) << "     already connected, but to a relay";
            if (relay->m_interval == pollingInterval && relay->m_align == alignment) {
                // qCDebug(LOG_PLASMA) << "    already connected to a relay of the same interval of"
                //          << pollingInterval << ", nothing to do";
                return;
            }

            if (relay->receiverCount() == 1) {
                // qCDebug(LOG_PLASMA) << "    removing relay, as it is now unused";
                d->relays.remove(relay->m_interval);
                delete relay;
            } else {
                if (visualization->metaObject()->indexOfSlot("dataUpdated(QString,Plasma::DataEngine::Data)") >= 0) {
                    disconnect(relay,
                               SIGNAL(dataUpdated(QString, Plasma::DataEngine::Data)),
                               visualization,
                               SLOT(dataUpdated(QString, Plasma::DataEngine::Data)));
                }
                // modelChanged is always emitted by the dataSource since there is no polling there
                if (visualization->metaObject()->indexOfSlot("modelChanged(QString,QAbstractItemModel*)") >= 0) {
                    disconnect(this, SIGNAL(modelChanged(QString, QAbstractItemModel *)), visualization, SLOT(modelChanged(QString, QAbstractItemModel *)));
                }
                // relay->isUnused();
            }
        } else if (pollingInterval < 1) {
            // the visualization was connected already, but not to a relay
            // and it still doesn't want to connect to a relay, so we have
            // nothing to do!
            // qCDebug(LOG_PLASMA) << "     already connected, nothing to do";
            return;
        } else {
            if (visualization->metaObject()->indexOfSlot("dataUpdated(QString,Plasma::DataEngine::Data)") >= 0) {
                disconnect(this, SIGNAL(dataUpdated(QString, Plasma::DataEngine::Data)), visualization, SLOT(dataUpdated(QString, Plasma::DataEngine::Data)));
            }
            if (visualization->metaObject()->indexOfSlot("modelChanged(QString,QAbstractItemModel*)") >= 0) {
                disconnect(this, SIGNAL(modelChanged(QString, QAbstractItemModel *)), visualization, SLOT(modelChanged(QString, QAbstractItemModel *)));
            }
        }
    } else {
        connect(visualization, &QObject::destroyed, this, &DataContainer::disconnectVisualization); //, Qt::QueuedConnection);
    }

    if (pollingInterval < 1) {
        // qCDebug(LOG_PLASMA) << "    connecting directly";
        d->relayObjects[visualization] = nullptr;
        if (visualization->metaObject()->indexOfSlot("dataUpdated(QString,Plasma::DataEngine::Data)") >= 0) {
            connect(this, SIGNAL(dataUpdated(QString, Plasma::DataEngine::Data)), visualization, SLOT(dataUpdated(QString, Plasma::DataEngine::Data)));
        }
        if (visualization->metaObject()->indexOfSlot("modelChanged(QString,QAbstractItemModel*)") >= 0) {
            connect(this, SIGNAL(modelChanged(QString, QAbstractItemModel *)), visualization, SLOT(modelChanged(QString, QAbstractItemModel *)));
        }
    } else {
        // qCDebug(LOG_PLASMA) << "    connecting to a relay";
        // we only want to do an immediate update if this is not the first object to connect to us
        // if it is the first visualization, then the source will already have been populated
        // engine's sourceRequested method
        bool immediateUpdate = connected || d->relayObjects.count() > 1;
        SignalRelay *relay = d->signalRelay(this, visualization, pollingInterval, alignment, immediateUpdate);
        if (visualization->metaObject()->indexOfSlot("dataUpdated(QString,Plasma::DataEngine::Data)") >= 0) {
            connect(relay, SIGNAL(dataUpdated(QString, Plasma::DataEngine::Data)), visualization, SLOT(dataUpdated(QString, Plasma::DataEngine::Data)));
        }
        // modelChanged is always emitted by the dataSource since there is no polling there
        if (visualization->metaObject()->indexOfSlot("modelChanged(QString,QAbstractItemModel*)") >= 0) {
            connect(this, SIGNAL(modelChanged(QString, QAbstractItemModel *)), visualization, SLOT(modelChanged(QString, QAbstractItemModel *)));
        }
    }
}

void DataContainer::setStorageEnabled(bool store)
{
    d->enableStorage = store;
    if (store) {
        QTimer::singleShot(QRandomGenerator::global()->bounded(2000 + 1), this, SLOT(retrieve()));
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

DataEngine *DataContainer::getDataEngine()
{
    QObject *o = this;
    DataEngine *de = nullptr;
    while (de == nullptr) {
        o = dynamic_cast<QObject *>(o->parent());
        if (o == nullptr) {
            return nullptr;
        }
        de = dynamic_cast<DataEngine *>(o);
    }
    return de;
}

void DataContainerPrivate::store()
{
    if (!q->needsToBeStored() || !q->isStorageEnabled()) {
        return;
    }

    DataEngine *de = q->getDataEngine();
    if (!de) {
        return;
    }

    q->setNeedsToBeStored(false);

    if (!storage) {
        storage = new Storage(q);
    }

    QVariantMap op = storage->operationDescription(QStringLiteral("save"));
    op[QStringLiteral("group")] = q->objectName();
    StorageJob *job = static_cast<StorageJob *>(storage->startOperationCall(op));
    job->setData(data);
    storageCount++;
    QObject::connect(job, SIGNAL(finished(KJob *)), q, SLOT(storeJobFinished(KJob *)));
}

void DataContainerPrivate::storeJobFinished(KJob *)
{
    --storageCount;
    if (storageCount < 1) {
        storage->deleteLater();
        storage = nullptr;
    }
}

void DataContainerPrivate::retrieve()
{
    DataEngine *de = q->getDataEngine();
    if (de == nullptr) {
        return;
    }

    if (!storage) {
        storage = new Storage(q);
    }

    QVariantMap retrieveGroup = storage->operationDescription(QStringLiteral("retrieve"));
    retrieveGroup[QStringLiteral("group")] = q->objectName();
    ServiceJob *retrieveJob = storage->startOperationCall(retrieveGroup);
    QObject::connect(retrieveJob, SIGNAL(result(KJob *)), q, SLOT(populateFromStoredData(KJob *)));
}

void DataContainerPrivate::populateFromStoredData(KJob *job)
{
    if (job->error()) {
        return;
    }

    StorageJob *ret = dynamic_cast<StorageJob *>(job);
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

    QVariantMap expireGroup = storage->operationDescription(QStringLiteral("expire"));
    // expire things older than 4 days
    expireGroup[QStringLiteral("age")] = 345600;
    storage->startOperationCall(expireGroup);
}

void DataContainer::disconnectVisualization(QObject *visualization)
{
    QMap<QObject *, SignalRelay *>::iterator objIt = d->relayObjects.find(visualization);
    disconnect(visualization, &QObject::destroyed, this, &DataContainer::disconnectVisualization); //, Qt::QueuedConnection);

    if (objIt == d->relayObjects.end() || !objIt.value()) {
        // it is connected directly to the DataContainer itself
        if (visualization->metaObject()->indexOfSlot("dataUpdated(QString,Plasma::DataEngine::Data)") >= 0) {
            disconnect(this, SIGNAL(dataUpdated(QString, Plasma::DataEngine::Data)), visualization, SLOT(dataUpdated(QString, Plasma::DataEngine::Data)));
        }
        if (visualization->metaObject()->indexOfSlot("modelChanged(QString,QAbstractItemModel*)") >= 0) {
            disconnect(this, SIGNAL(modelChanged(QString, QAbstractItemModel *)), visualization, SLOT(modelChanged(QString, QAbstractItemModel *)));
        }
    } else {
        SignalRelay *relay = objIt.value();

        if (relay->receiverCount() == 1) {
            d->relays.remove(relay->m_interval);
            delete relay;
        } else {
            if (visualization->metaObject()->indexOfSlot("dataUpdated(QString,Plasma::DataEngine::Data)") >= 0) {
                disconnect(relay, SIGNAL(dataUpdated(QString, Plasma::DataEngine::Data)), visualization, SLOT(dataUpdated(QString, Plasma::DataEngine::Data)));
            }
            // modelChanged is always emitted by the dataSource since there is no polling there
            if (visualization->metaObject()->indexOfSlot("modelChanged(QString,QAbstractItemModel*)") >= 0) {
                disconnect(this, SIGNAL(modelChanged(QString, QAbstractItemModel *)), visualization, SLOT(modelChanged(QString, QAbstractItemModel *)));
            }
        }
    }

    d->relayObjects.erase(objIt);
    d->checkUsage();
}

void DataContainer::checkForUpdate()
{
    // qCDebug(LOG_PLASMA) << objectName() << d->dirty;
    if (d->dirty) {
        Q_EMIT dataUpdated(objectName(), d->data);

        // copy as checkQueueing can result in deletion of the relay
        const auto relays = d->relays;
        for (SignalRelay *relay : relays) {
            relay->checkQueueing();
        }

        d->dirty = false;
    }
}

void DataContainer::forceImmediateUpdate()
{
    if (d->dirty) {
        d->dirty = false;
        Q_EMIT dataUpdated(objectName(), d->data);
    }

    for (SignalRelay *relay : std::as_const(d->relays)) {
        relay->forceImmediateUpdate();
    }
}

uint DataContainer::timeSinceLastUpdate() const
{
    return d->updateTimer.elapsed();
}

void DataContainer::setNeedsUpdate(bool update)
{
    d->cached = update;
}

bool DataContainer::isUsed() const
{
    return !d->relays.isEmpty() || receivers(SIGNAL(dataUpdated(QString, Plasma::DataEngine::Data))) > 0;
}

void DataContainerPrivate::checkUsage()
{
    if (!checkUsageTimer.isActive()) {
        checkUsageTimer.start(10, q);
    }
}

void DataContainer::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == d->checkUsageTimer.timerId()) {
        if (!isUsed()) {
            // DO NOT CALL ANYTHING AFTER THIS LINE AS IT MAY GET DELETED!
            // qCDebug(LOG_PLASMA) << objectName() << "is unused";

            // NOTE: Notifying visualization of the model destruction before actual deletion avoids crashes in some edge cases
            if (d->model) {
                d->model.clear();
                Q_EMIT modelChanged(objectName(), nullptr);
            }
            Q_EMIT becameUnused(objectName());
        }
        d->checkUsageTimer.stop();
    } else if (event->timerId() == d->storageTimer.timerId()) {
        d->store();
        d->storageTimer.stop();
    }
}

} // Plasma namespace

#include "moc_datacontainer.cpp"
