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

namespace Plasma
{

class SignalRelay;

class DataContainer::Private
{
public:
    Private()
        : dirty(false), cached(false)
    {}

    QObject* signalRelay(const DataContainer* dc, QObject *visualization,
                         uint updateInterval, Plasma::IntervalAlignment align);

    DataEngine::Data data;
    QMap<QObject *, SignalRelay *> relayObjects;
    QMap<uint, SignalRelay *> relays;
    QTime updateTs;
    bool dirty : 1;
    bool cached : 1;
};

class SignalRelay : public QObject
{
    Q_OBJECT

public:
    SignalRelay(DataContainer* parent, DataContainer::Private *data, uint ival, Plasma::IntervalAlignment align)
        : QObject(parent),
          dc(parent),
          d(data),
          m_interval(ival),
          m_align(align),
          m_resetTimer(true),
          m_queued(false)
    {
        //kDebug() << "signal relay with time of" << m_timerId << "being set up";
        m_timerId = startTimer(0);
        if (m_align != Plasma::NoAlignment) {
            checkAlignment();
        }
    }

    bool isUnused()
    {
        return receivers(SIGNAL(dataUpdated(QString,Plasma::DataEngine::Data))) < 1;
    }

    void checkAlignment()
    {
        int newTime = 0;

        QTime t = QTime::currentTime();
        if (m_align == Plasma::AlignToMinute) {
            int seconds = t.second();
            if (seconds > 2) {
                newTime = ((60 - seconds) * 1000) + 500;
            }
        } else if (m_align == Plasma::AlignToHour) {
            int minutes = t.minute();
            int seconds = t.second();
            if (minutes > 1 || seconds > 10) {
                newTime = ((60 - minutes) * 1000 * 60) +
                          ((60 - seconds) * 1000) + 500;
            }
        }

        if (newTime) {
            killTimer(m_timerId);
            m_timerId = startTimer(newTime);
            m_resetTimer = true;
        }
    }

    void checkQueueing() {
        if (m_queued) {
            emit dataUpdated(dc->objectName(), d->data);
            m_queued = false;
            //TODO: should we re-align our timer at this point, to avoid
            //      constant queueing due to more-or-less constant time
            //      async update time? this might make sense for
            //      staggered accesses to the same source by multiple
            //      visualizations causing a minimumUpdateInterval violation.
            //      it may not make sense for purely async-and-takes-a-while
            //      type operations (e.g. network fetching).
            //      we need more real world data before making such a change
            //      change
            //
            // killTimer(m_timerId);
            // m_timerId = startTime(m_interval);
        }
    }

    DataContainer *dc;
    DataContainer::Private *d;
    uint m_interval;
    Plasma::IntervalAlignment m_align;
    int m_timerId;
    bool m_resetTimer;
    bool m_queued;

signals:
    void dataUpdated(const QString&, const Plasma::DataEngine::Data&);

protected:
    void timerEvent(QTimerEvent *event)
    {
        if (m_resetTimer) {
            killTimer(m_timerId);
            m_timerId = startTimer(m_interval);
            m_resetTimer = false;
        }

        if (m_align != Plasma::NoAlignment) {
            checkAlignment();
        }

        emit dc->requestUpdate(dc);
        if (!dc->hasUpdates()) {
            // the source wasn't actually updated; so let's put ourselves in the queue
            // so we get an dataUpdated() when the data does arrive
            m_queued = true;
        } else {
            emit dataUpdated(dc->objectName(), d->data);
        }
        event->accept();
    }
};

QObject* DataContainer::Private::signalRelay(const DataContainer* dc, QObject *visualization, uint updateInterval, Plasma::IntervalAlignment align)
{
    QMap<uint, SignalRelay *>::const_iterator relayIt = relays.find(updateInterval);
    SignalRelay *relay = 0;

    //FIXME what if we have two applets with the same interval and different alignment?
    if (relayIt == relays.end()) {
        relay = new SignalRelay(const_cast<DataContainer*>(dc), this, updateInterval, align);
        relays[updateInterval] = relay;
    } else {
        relay = relayIt.value();
    }

    relayObjects[visualization] = relay;
    return relay;
}

}

#endif // multiple inclusion guard
