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

#include "datacontainer.h"
#include "datacontainer_p.h"

#include <QVariant>

#include <KDebug>

namespace Plasma
{

DataContainer::DataContainer(QObject* parent)
    : QObject(parent),
      d(new Private())
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

void DataContainer::setData(const QString& key, const QVariant& value)
{
    if (value.isNull() || !value.isValid()) {
        if (!d->data.contains(key)) {
            return;
        }

        d->data.remove(key);
    } else {
        d->data[key] = value;
    }

    d->dirty = true;
}

void DataContainer::clearData()
{
    if (d->data.count() < 1) {
        // avoid an update if we don't have any data anyways
        return;
    }

    d->data.clear();
    d->dirty = true;
}

void DataContainer::checkForUpdate()
{
    if (d->dirty) {
        emit updated(objectName(), d->data);
        d->dirty = false;
    }
}

QObject* DataContainer::signalRelay(QObject *visualization, uint updateInterval) const
{
    return d->signalRelay(this, visualization, updateInterval);
}

void DataContainer::checkUsage()
{
    if (d->relays.count() < 1 &&
        receivers(SIGNAL(updated(QString, Plasma::DataEngine::Data))) < 1) {
        // DO NOT CALL ANYTHING AFTER THIS LINE AS IT MAY GET DELETED!
        emit unused(objectName());
    }
}

void DataContainer::connectVisualization(QObject* visualization, uint updateInterval)
{
//    kDebug() << "connecting visualization" << (void*)visualization << "at interval of" << updateInterval;
    QMap<QObject *, SignalRelay *>::iterator objIt = d->relayObjects.find(visualization);
    bool connected = objIt != d->relayObjects.end();
    if (connected) {
        // this visualization is already connected. just adjust the update
        // frequency if necessary
        SignalRelay *relay = objIt.value();
        if (relay) {
            // connected to a relay
            disconnect(relay, SIGNAL(updated(QString,Plasma::DataEngine::Data)),
                    visualization, SLOT(updated(QString,Plasma::DataEngine::Data)));

            if (relay->isUnused()) {
                d->relays.erase(d->relays.find(relay->interval));
                delete relay;
            }
//            kDebug() << "     already connected, but to a relay";
        } else if (updateInterval < 1) {
            // the visualization was connected already, but not to a relay
            // and it still doesn't want to connect to a relay, so we have
            // nothing to do!
            kDebug() << "     already connected, nothing to do";
            return;
        } else {
            disconnect(this, SIGNAL(updated(QString,Plasma::DataEngine::Data)),
                       visualization, SLOT(updated(QString,Plasma::DataEngine::Data)));
        }
    }

    if (!connected) {
        connect(visualization, SIGNAL(destroyed(QObject*)),
                this, SLOT(disconnectVisualization(QObject*)), Qt::QueuedConnection);
    }

    d->relayObjects[visualization] = 0;

    if (updateInterval < 1) {
//        kDebug() << "    connecting directly";
        connect(this, SIGNAL(updated(QString,Plasma::DataEngine::Data)),
                visualization, SLOT(updated(QString,Plasma::DataEngine::Data)));
    } else {
//        kDebug() << "    connecting to a relay";
        connect(signalRelay(visualization, updateInterval),
                SIGNAL(updated(QString,Plasma::DataEngine::Data)),
                visualization, SLOT(updated(QString,Plasma::DataEngine::Data)));
    }
}

void DataContainer::disconnectVisualization(QObject* visualization)
{
    QMap<QObject *, SignalRelay *>::iterator objIt = d->relayObjects.find(visualization);

    if (objIt == d->relayObjects.end() || !objIt.value()) {
        // it is connected directly to the DataContainer itself
        disconnect(this, SIGNAL(updated(QString,Plasma::DataEngine::Data)),
                   visualization, SLOT(updated(QString,Plasma::DataEngine::Data)));
    } else {
        SignalRelay *relay = objIt.value();
        disconnect(relay, SIGNAL(updated(QString,Plasma::DataEngine::Data)),
                   visualization, SLOT(updated(QString,Plasma::DataEngine::Data)));

        if (relay->isUnused()) {
            d->relays.erase(d->relays.find(relay->interval));
            delete relay;
        }

    }

    d->relayObjects.erase(objIt);
    checkUsage();
}

} // Plasma namespace

#include "datacontainer.moc"
#include "datacontainer_p.moc"

