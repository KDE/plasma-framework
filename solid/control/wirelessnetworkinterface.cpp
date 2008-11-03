/*  This file is part of the KDE project
    Copyright (C) 2006 Will Stephenson <wstephenson@kde.org>
    Copyright (C) 2007 Kevin Ottens <ervin@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.

*/

#include "wirelessnetworkinterface.h"
#include "wirelessnetworkinterface_p.h"

#include "soliddefs_p.h"
#include "wirelessaccesspoint.h"
#include "ifaces/wirelessaccesspoint.h"
#include "ifaces/wirelessnetworkinterface.h"

Solid::Control::WirelessNetworkInterface::WirelessNetworkInterface(QObject *backendObject)
    : NetworkInterface(*new WirelessNetworkInterfacePrivate(this), backendObject)
{
    Q_D(WirelessNetworkInterface);
    d->setBackendObject(backendObject);
    makeConnections( backendObject );
    d->readAccessPoints();
}

Solid::Control::WirelessNetworkInterface::WirelessNetworkInterface(const WirelessNetworkInterface &networkinterface)
    : NetworkInterface(*new WirelessNetworkInterfacePrivate(this), networkinterface)
{
    Q_D(WirelessNetworkInterface);
    d->setBackendObject(networkinterface.d_ptr->backendObject());
    makeConnections( networkinterface.d_ptr->backendObject() );
    d->readAccessPoints();
}

Solid::Control::WirelessNetworkInterface::WirelessNetworkInterface(WirelessNetworkInterfacePrivate &dd, QObject *backendObject)
    : NetworkInterface(dd, backendObject)
{
    Q_D(WirelessNetworkInterface);
    makeConnections( backendObject );
    d->readAccessPoints();
}

Solid::Control::WirelessNetworkInterface::WirelessNetworkInterface(WirelessNetworkInterfacePrivate &dd, const WirelessNetworkInterface &networkinterface)
    : NetworkInterface(dd, networkinterface.d_ptr->backendObject())
{
    Q_D(WirelessNetworkInterface);
    makeConnections( networkinterface.d_ptr->backendObject() );
    d->readAccessPoints();
}

Solid::Control::WirelessNetworkInterface::~WirelessNetworkInterface()
{

}

Solid::Control::NetworkInterface::Type Solid::Control::WirelessNetworkInterface::type() const
{
    return Ieee80211;
}

void Solid::Control::WirelessNetworkInterface::makeConnections(QObject * source)
{
    connect(source, SIGNAL(accessPointAppeared(const QString &)),
            this, SLOT(_k_accessPointAdded(const QString &)));
    connect(source, SIGNAL(accessPointDisappeared(const QString &)),
            this, SLOT(_k_accessPointRemoved(const QString &)));
    connect(source, SIGNAL(bitRateChanged(int)),
            this, SIGNAL(bitRateChanged(int)));
    connect(source, SIGNAL(activeAccessPointChanged(const QString&)),
            this, SIGNAL(activeAccessPointChanged(const QString&)));
    connect(source, SIGNAL(modeChanged(Solid::Control::WirelessNetworkInterface::OperationMode)),
            this, SIGNAL(modeChanged(Solid::Control::WirelessNetworkInterface::OperationMode)));
}

QString Solid::Control::WirelessNetworkInterface::hardwareAddress() const
{
    Q_D(const WirelessNetworkInterface);
    return_SOLID_CALL(Ifaces::WirelessNetworkInterface *, d->backendObject(), QString(), hardwareAddress());
}

QString Solid::Control::WirelessNetworkInterface::activeAccessPoint() const
{
    Q_D(const WirelessNetworkInterface);
    return_SOLID_CALL(Ifaces::WirelessNetworkInterface *, d->backendObject(), QString(), activeAccessPoint());
}

int Solid::Control::WirelessNetworkInterface::bitRate() const
{
    Q_D(const WirelessNetworkInterface);
    return_SOLID_CALL(Ifaces::WirelessNetworkInterface *, d->backendObject(), 0, bitRate());
}

Solid::Control::WirelessNetworkInterface::OperationMode Solid::Control::WirelessNetworkInterface::mode() const
{
    Q_D(const WirelessNetworkInterface);
    return_SOLID_CALL(Ifaces::WirelessNetworkInterface *, d->backendObject(), (Solid::Control::WirelessNetworkInterface::OperationMode)0, mode());
}

Solid::Control::WirelessNetworkInterface::Capabilities Solid::Control::WirelessNetworkInterface::wirelessCapabilities() const
{
    Q_D(const WirelessNetworkInterface);
    return_SOLID_CALL(Ifaces::WirelessNetworkInterface *, d->backendObject(), (Solid::Control::WirelessNetworkInterface::Capabilities)0, wirelessCapabilities());
}

Solid::Control::AccessPoint * Solid::Control::WirelessNetworkInterface::findAccessPoint(const QString  & uni) const
{
    const AccessPointPair pair = findRegisteredAccessPoint(uni);
    return pair.first;
}

void Solid::Control::WirelessNetworkInterfacePrivate::setBackendObject(QObject *object)
{
    NetworkInterfacePrivate::setBackendObject(object);

    if (object) {
    }
}

Solid::Control::AccessPointList Solid::Control::WirelessNetworkInterface::accessPoints() const
{
    Q_D(const WirelessNetworkInterface);
    return d->apMap.keys();
}

void Solid::Control::WirelessNetworkInterface::_k_accessPointAdded(const QString & uni)
{
    Q_D(WirelessNetworkInterface);
    AccessPointMap::Iterator mapIt = d->apMap.find(uni);

    if (mapIt != d->apMap.end())
    {
        // Oops, I'm not sure it should happen...
        // But for an unknown reason it does
        // We were deleting the existing items here, which caused problems later
        // when the client calls find AccessPoint and gets a frontend object with a
        // null private object - dangling pointer
    } else {
        mapIt = d->apMap.insert(uni, AccessPointPair(0, 0));
    }

    emit accessPointAppeared(uni);
}

void Solid::Control::WirelessNetworkInterface::_k_accessPointRemoved(const QString & uni)
{
    Q_D(WirelessNetworkInterface);
    AccessPointPair pair = d->apMap.take(uni);

    if (pair.first!= 0)
    {
        delete pair.first;
        delete pair.second;
    }

    emit accessPointDisappeared(uni);
}

void Solid::Control::WirelessNetworkInterface::_k_destroyed(QObject *object)
{
    Q_D(WirelessNetworkInterface);
    Ifaces::AccessPoint *ap = qobject_cast<Ifaces::AccessPoint *>(object);

    if (ap!=0)
    {
        QString uni = ap->uni();
        AccessPointPair pair = d->apMap.take(uni);
        delete pair.first;
    }
}

Solid::Control::AccessPointPair
Solid::Control::WirelessNetworkInterface::findRegisteredAccessPoint(const QString &uni) const
{
    Q_D(const WirelessNetworkInterface);

    AccessPointMap::ConstIterator mapIt = d->apMap.find(uni);
    if (mapIt != d->apMap.end() && mapIt.value().second) {
        return mapIt.value();
    } else {
        AccessPointPair pair = d->createAP(uni);
        if (pair.first && pair.second) {
            d->apMap[uni] = pair;
        }
        return pair;
    }

    return AccessPointPair(0, 0);
}

void Solid::Control::WirelessNetworkInterfacePrivate::readAccessPoints()
{
    Ifaces::WirelessNetworkInterface * t = qobject_cast<Ifaces::WirelessNetworkInterface *>(backendObject());
    if (t != 0)
    {
        const MacAddressList unis = t->accessPoints();
        Q_FOREACH (const QString & uni, unis) {
            apMap[uni] = AccessPointPair(0, 0);
        }
    }
}

Solid::Control::AccessPointPair
Solid::Control::WirelessNetworkInterfacePrivate::createAP(const QString &uni) const
{
    Ifaces::WirelessNetworkInterface *device = qobject_cast<Ifaces::WirelessNetworkInterface *>(backendObject());
    AccessPoint *ap = 0;

    if (device!=0) {
        Ifaces::AccessPoint *iface = qobject_cast<Ifaces::AccessPoint *>(device->createAccessPoint(uni));

        if (qobject_cast<Ifaces::AccessPoint *>(iface)!=0) {
            ap = new AccessPoint(iface);
        }

        if (ap != 0) {
            AccessPointPair pair(ap, iface);
            QObject::connect(iface, SIGNAL(destroyed(QObject *)),
                             parent(), SLOT(_k_destroyed(QObject *)));

            return pair;
        }
    }
    return AccessPointPair(0, 0);
}

#include "wirelessnetworkinterface.moc"
