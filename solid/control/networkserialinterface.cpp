/*
Copyright 2008 Will Stephenson <wstephenson@kde.org>

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) version 3, or any
later version accepted by the membership of KDE e.V. (or its
successor approved by the membership of KDE e.V.), which shall
act as a proxy defined in Section 6 of version 3 of the license.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public 
License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "networkserialinterface.h"
#include "networkserialinterface_p.h"

#include "frontendobject_p.h"
#include "soliddefs_p.h"
#include "ifaces/wirelessaccesspoint.h"

Solid::Control::SerialNetworkInterface::SerialNetworkInterface(QObject *backendObject)
    : NetworkInterface(*new SerialNetworkInterfacePrivate(this), backendObject)
{
    Q_D(SerialNetworkInterface);
    d->setBackendObject(backendObject);
    makeConnections( backendObject );
}

Solid::Control::SerialNetworkInterface::SerialNetworkInterface(const SerialNetworkInterface &networkinterface)
    : NetworkInterface(*new SerialNetworkInterfacePrivate(this), networkinterface)
{
    Q_D(SerialNetworkInterface);
    d->setBackendObject(networkinterface.d_ptr->backendObject());
    makeConnections( networkinterface.d_ptr->backendObject() );
}

Solid::Control::SerialNetworkInterface::SerialNetworkInterface(SerialNetworkInterfacePrivate &dd, QObject *backendObject)
    : NetworkInterface(dd, backendObject)
{
    makeConnections( backendObject );
}

Solid::Control::SerialNetworkInterface::SerialNetworkInterface(SerialNetworkInterfacePrivate &dd, const SerialNetworkInterface &networkinterface)
    : NetworkInterface(dd, networkinterface.d_ptr->backendObject())
{
    makeConnections( networkinterface.d_ptr->backendObject() );
}

Solid::Control::SerialNetworkInterface::~SerialNetworkInterface()
{
}

Solid::Control::NetworkInterface::Type Solid::Control::SerialNetworkInterface::type() const
{
    return Serial;
}

void Solid::Control::SerialNetworkInterface::makeConnections(QObject * source)
{
    connect(source, SIGNAL(pppStats(uint,uint)),
            this, SIGNAL(pppStats(uint,uint)));
}

void Solid::Control::SerialNetworkInterfacePrivate::setBackendObject(QObject *object)
{
    NetworkInterfacePrivate::setBackendObject(object);
}

void Solid::Control::SerialNetworkInterface::_k_destroyed(QObject *object)
{
    Q_UNUSED(object);
}
// vim: sw=4 sts=4 et tw=100
