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

class DataContainerPrivate
{
public:
    DataContainerPrivate()
        : dirty(false), cached(false)
    {}

    SignalRelay* signalRelay(const DataContainer* dc, QObject *visualization,
                             uint pollingInterval, Plasma::IntervalAlignment align);

    bool hasUpdates();

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
    SignalRelay(DataContainer* parent, DataContainerPrivate *data, uint ival, Plasma::IntervalAlignment align)
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

    int receiverCount() const
    {
        return receivers(SIGNAL(dataUpdated(QString,Plasma::DataEngine::Data)));
    }

    bool isUnused()
    {
        return receivers(SIGNAL(dataUpdated(QString,Plasma::DataEngine::Data))) < 1;
    }

    void checkAlignment();
    void checkQueueing();

    DataContainer *dc;
    DataContainerPrivate *d;
    uint m_interval;
    Plasma::IntervalAlignment m_align;
    int m_timerId;
    bool m_resetTimer;
    bool m_queued;

signals:
    void dataUpdated(const QString&, const Plasma::DataEngine::Data&);

protected:
    void timerEvent(QTimerEvent *event);
};

} // Plasma namespace

#endif // multiple inclusion guard

