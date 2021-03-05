/*
    SPDX-FileCopyrightText: 2006-2007 Aaron Seigo <aseigo@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "datacontainer_p.h" //krazy:exclude=includes

namespace Plasma
{
SignalRelay *DataContainerPrivate::signalRelay(const DataContainer *dc,
                                               QObject *visualization,
                                               uint pollingInterval,
                                               Plasma::Types::IntervalAlignment align,
                                               bool immediateUpdate)
{
    QMap<uint, SignalRelay *>::const_iterator relayIt = relays.constFind(pollingInterval);
    SignalRelay *relay = nullptr;

    // FIXME what if we have two applets with the same interval and different alignment?
    if (relayIt == relays.constEnd()) {
        relay = new SignalRelay(const_cast<DataContainer *>(dc), this, pollingInterval, align, immediateUpdate);
        relays[pollingInterval] = relay;
    } else {
        relay = relayIt.value();
    }

    relayObjects[visualization] = relay;
    return relay;
}

bool DataContainerPrivate::hasUpdates()
{
    if (cached) {
        // SignalRelay needs us to pretend we did an update
        cached = false;
        return true;
    }

    return dirty;
}

SignalRelay::SignalRelay(DataContainer *parent, DataContainerPrivate *data, uint ival, Plasma::Types::IntervalAlignment align, bool immediateUpdate)
    : QObject(parent)
    , dc(parent)
    , d(data)
    , m_interval(ival)
    , m_align(align)
    , m_resetTimer(true)
    , m_queued(true)
{
    // qCDebug(LOG_PLASMA) << "signal relay with time of" << m_timerId << "being set up";
    m_timerId = startTimer(immediateUpdate ? 0 : m_interval);
    if (m_align != Plasma::Types::NoAlignment) {
        checkAlignment();
    }
}

int SignalRelay::receiverCount() const
{
    return receivers(SIGNAL(dataUpdated(QString, Plasma::DataEngine::Data)));
}

bool SignalRelay::isUnused() const
{
    return receivers(SIGNAL(dataUpdated(QString, Plasma::DataEngine::Data))) < 1;
}

void SignalRelay::checkAlignment()
{
    int newTime = 0;

    QTime t = QTime::currentTime();
    if (m_align == Plasma::Types::AlignToMinute) {
        int seconds = t.second();
        newTime = ((60 - seconds) * 1000) + 500;
    } else if (m_align == Plasma::Types::AlignToHour) {
        int minutes = t.minute();
        int seconds = t.second();
        if (minutes > 1 || seconds > 10) {
            newTime = ((60 - minutes) * 1000 * 60) + ((60 - seconds) * 1000) + 500;
        }
    }

    if (newTime) {
        killTimer(m_timerId);
        m_timerId = startTimer(newTime);
        m_resetTimer = true;
    }
}

void SignalRelay::checkQueueing()
{
    // qCDebug(LOG_PLASMA) << m_queued;
    if (m_queued) {
        Q_EMIT dataUpdated(dc->objectName(), d->data);
        m_queued = false;
        // TODO: should we re-align our timer at this point, to avoid
        //      constant queueing due to more-or-less constant time
        //      async update time? this might make sense for
        //      staggered accesses to the same source by multiple
        //      visualizations causing a minimumPollingInterval violation.
        //      it may not make sense for purely async-and-takes-a-while
        //      type operations (e.g. network fetching).
        //      we need more real world data before making such a change
        //      change
        //
        // killTimer(m_timerId);
        // m_timerId = startTime(m_interval);
    }
}

void SignalRelay::forceImmediateUpdate()
{
    Q_EMIT dataUpdated(dc->objectName(), d->data);
}

void SignalRelay::timerEvent(QTimerEvent *event)
{
    if (event->timerId() != m_timerId) {
        QObject::timerEvent(event);
        return;
    }

    if (m_resetTimer) {
        killTimer(m_timerId);
        m_timerId = startTimer(m_interval);
        m_resetTimer = false;
    }

    if (m_align != Plasma::Types::NoAlignment) {
        checkAlignment();
    }

    Q_EMIT dc->updateRequested(dc);
    if (d->hasUpdates()) {
        // qCDebug(LOG_PLASMA) << "emitting data updated directly" << d->data;
        Q_EMIT dataUpdated(dc->objectName(), d->data);
        m_queued = false;
    } else {
        // the source wasn't actually updated; so let's put ourselves in the queue
        // so we get a dataUpdated() call when the data does arrive
        // qCDebug(LOG_PLASMA) << "queued";
        m_queued = true;
    }
}

} // Plasma namespace

#include "moc_datacontainer_p.cpp"
