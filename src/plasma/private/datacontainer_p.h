/*
    SPDX-FileCopyrightText: 2006-2007 Aaron Seigo <aseigo@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef PLASMA_DATACONTAINER_P_H
#define PLASMA_DATACONTAINER_P_H

#include "datacontainer.h"
#include "servicejob.h"
#include "storage_p.h"

#include <QBasicTimer>
#include <QElapsedTimer>
#include <QTimerEvent>

#include <QAbstractItemModel>

class QTimer;

namespace Plasma
{
class ServiceJob;
class SignalRelay;

class DataContainerPrivate
{
public:
    DataContainerPrivate(DataContainer *container)
        : q(container)
        , storage(nullptr)
        , storageCount(0)
        , dirty(false)
        , cached(false)
        , enableStorage(false)
        , isStored(true)
    {
    }

    /**
     * Check if the DataContainer is still in use.
     *
     * If not the signal "becameUnused" will be emitted.
     *
     * Warning: The DataContainer may be invalid after calling this function, because a listener
     * to becameUnused() may have deleted it.
     **/
    void checkUsage();

    SignalRelay *
    signalRelay(const DataContainer *dc, QObject *visualization, uint pollingInterval, Plasma::Types::IntervalAlignment align, bool immediateUpdate);

    bool hasUpdates();

    /**
     * Deletes the store member of DataContainerPrivate if
     * there are no more references to it.
     */
    void storeJobFinished(KJob *job);

    /**
     * Does the work of putting the data from disk into the DataContainer
     * after retrieve() sets it up.
     */
    void populateFromStoredData(KJob *job);

    void store();
    void retrieve();

    DataContainer *q;
    DataEngine::Data data;
    QMap<QObject *, SignalRelay *> relayObjects;
    QMap<uint, SignalRelay *> relays;
    QElapsedTimer updateTimer;
    Storage *storage;
    QBasicTimer storageTimer;
    QBasicTimer checkUsageTimer;
    QPointer<QAbstractItemModel> model;
    int storageCount;
    bool dirty : 1;
    bool cached : 1;
    bool enableStorage : 1;
    bool isStored : 1;
};

class SignalRelay : public QObject
{
    Q_OBJECT

public:
    SignalRelay(DataContainer *parent, DataContainerPrivate *data, uint ival, Plasma::Types::IntervalAlignment align, bool immediateUpdate);

    int receiverCount() const;
    bool isUnused() const;

    void checkAlignment();
    void checkQueueing();
    void forceImmediateUpdate();

    DataContainer *dc;
    DataContainerPrivate *d;
    uint m_interval;
    Plasma::Types::IntervalAlignment m_align;
    int m_timerId;
    bool m_resetTimer;
    bool m_queued;

Q_SIGNALS:
    void dataUpdated(const QString &, const Plasma::DataEngine::Data &);

protected:
    void timerEvent(QTimerEvent *event) override;
};

} // Plasma namespace

#endif // multiple inclusion guard
