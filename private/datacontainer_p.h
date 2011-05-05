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

#ifndef PLASMA_DATACONTAINER_P_H
#define PLASMA_DATACONTAINER_P_H

#include <QtCore/QTimerEvent>
#include <QtCore/QTime>
#include "servicejob.h"
#include "storage_p.h"

class QTimer;

namespace Plasma
{
class ServiceJob;
class SignalRelay;

class DataContainerPrivate
{
public:
    DataContainerPrivate(DataContainer *container)
        : q(container),
          storage(NULL),
          dirty(false),
          cached(false),
          enableStorage(false),
          isStored(true),
          storageCount(0)
    {
    }

    SignalRelay *signalRelay(const DataContainer *dc, QObject *visualization,
                             uint pollingInterval, Plasma::IntervalAlignment align,
                             bool immediateUpdate);

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
    QTimer *storageTimer;
    QTime updateTs;
    Storage* storage;
    bool dirty : 1;
    bool cached : 1;
    bool enableStorage : 1;
    bool isStored : 1;
    int  storageCount;
};

class SignalRelay : public QObject
{
    Q_OBJECT

public:
    SignalRelay(DataContainer *parent, DataContainerPrivate *data,
                uint ival, Plasma::IntervalAlignment align, bool immediateUpdate);

    int receiverCount() const;
    bool isUnused() const;

    void checkAlignment();
    void checkQueueing();
    void forceImmediateUpdate();

    DataContainer *dc;
    DataContainerPrivate *d;
    uint m_interval;
    Plasma::IntervalAlignment m_align;
    int m_timerId;
    bool m_resetTimer;
    bool m_queued;

signals:
    void dataUpdated(const QString &, const Plasma::DataEngine::Data &);

protected:
    void timerEvent(QTimerEvent *event);
};

} // Plasma namespace

#endif // multiple inclusion guard

