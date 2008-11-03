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

#include "wirednetworkinterface.h"
#include "wirednetworkinterface_p.h"

#include "soliddefs_p.h"
#include "ifaces/wirednetworkinterface.h"

Solid::Control::WiredNetworkInterface::WiredNetworkInterface(QObject *backendObject)
    : NetworkInterface(*new WiredNetworkInterfacePrivate(this), backendObject)
{
    Q_D(WiredNetworkInterface);
    d->setBackendObject(backendObject);
}

Solid::Control::WiredNetworkInterface::WiredNetworkInterface(const WiredNetworkInterface &networkinterface)
    : NetworkInterface(*new WiredNetworkInterfacePrivate(this), networkinterface)
{
    Q_D(WiredNetworkInterface);
    d->setBackendObject(networkinterface.d_ptr->backendObject());
}

Solid::Control::WiredNetworkInterface::WiredNetworkInterface(WiredNetworkInterfacePrivate &dd, QObject *backendObject)
    : NetworkInterface(dd, backendObject)
{
}

Solid::Control::WiredNetworkInterface::WiredNetworkInterface(WiredNetworkInterfacePrivate &dd, const WiredNetworkInterface &network)
    : NetworkInterface(dd, network.d_ptr->backendObject())
{
}

Solid::Control::WiredNetworkInterface::~WiredNetworkInterface()
{

}

Solid::Control::NetworkInterface::Type Solid::Control::WiredNetworkInterface::type() const
{
    return Ieee8023;
}

QString Solid::Control::WiredNetworkInterface::hardwareAddress() const
{
    Q_D(const WiredNetworkInterface);
    return_SOLID_CALL(Ifaces::WiredNetworkInterface *, d->backendObject(), QString(), hardwareAddress());
}

int Solid::Control::WiredNetworkInterface::bitRate() const
{
    Q_D(const WiredNetworkInterface);
    return_SOLID_CALL(Ifaces::WiredNetworkInterface *, d->backendObject(), 0, bitRate());
}

bool Solid::Control::WiredNetworkInterface::carrier() const
{
    Q_D(const WiredNetworkInterface);
    return_SOLID_CALL(Ifaces::WiredNetworkInterface *, d->backendObject(), false, carrier());
}

void Solid::Control::WiredNetworkInterfacePrivate::setBackendObject(QObject *object)
{
    NetworkInterfacePrivate::setBackendObject(object);

    if (object) {
        QObject::connect(object, SIGNAL(bitRateChanged(int)),
                         parent(), SIGNAL(bitRateChanged(int)));
        QObject::connect(object, SIGNAL(carrierChanged(bool)),
                         parent(), SIGNAL(carrierChanged(bool)));
    }
}

void Solid::Control::WiredNetworkInterface::_k_destroyed(QObject *object)
{
    Q_UNUSED(object);
    // nothing to do yet
}
#include "wirednetworkinterface.moc"
