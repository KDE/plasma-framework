/*
 *   Copyright 2006-2007 Aaron Seigo <aseigo@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License version 2 as
 *   published by the Free Software Foundation
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

namespace Plasma
{

class SignalRelay;

class DataContainer::Private
{
public:
    Private()
    : dirty(false)
    {}

    QObject* signalRelay(const DataContainer* dc, QObject *visualization, uint updateInterval);

    DataEngine::Data data;
    QMap<QObject *, SignalRelay *> relayObjects;
    QMap<uint, SignalRelay *> relays;
    bool dirty : 1;
};

class SignalRelay : public QObject
{
    Q_OBJECT

public:
    SignalRelay(DataContainer* parent, DataContainer::Private *data, uint ival)
        : QObject(parent),
          dc(parent),
          d(data),
          interval(ival)
    {
        startTimer(interval);
    }

    bool isUnused()
    {
        return receivers(SIGNAL(updated(QString,Plasma::DataEngine::Data))) < 1;
    }

    DataContainer *dc;
    DataContainer::Private *d;
    uint interval;

signals:
    void updated(const QString&, const Plasma::DataEngine::Data&);

protected:
    void timerEvent(QTimerEvent *event)
    {
        emit dc->requestUpdate(dc->objectName());
        emit updated(dc->objectName(), d->data);
        event->accept();
    }
};

QObject* DataContainer::Private::signalRelay(const DataContainer* dc, QObject *visualization, uint updateInterval)
{
    QMap<uint, SignalRelay *>::const_iterator relayIt = relays.find(updateInterval);
    SignalRelay *relay = 0;

    if (relayIt == relays.end()) {
        relay = new SignalRelay(const_cast<DataContainer*>(dc), this, updateInterval);
        relays[updateInterval] = relay;
    } else {
        relay = relayIt.value();
    }

    relayObjects[visualization] = relay;
    return relay;
}

}

#endif // multiple inclusion guard
