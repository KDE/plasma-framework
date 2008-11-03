/* Copyright 2008 Will Stephenson <wstephenson@kde.org>

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

#include "networkcdmainterface.h"
#include "networkcdmainterface_p.h"

#include "frontendobject_p.h"
#include "soliddefs_p.h"
#include "ifaces/wirelessaccesspoint.h"

Solid::Control::CdmaNetworkInterface::CdmaNetworkInterface(QObject *backendObject)
    : SerialNetworkInterface(*new CdmaNetworkInterfacePrivate(this), backendObject)
{
    Q_D(CdmaNetworkInterface);
    d->setBackendObject(backendObject);
    makeConnections( backendObject );
}

Solid::Control::CdmaNetworkInterface::CdmaNetworkInterface(const CdmaNetworkInterface &networkinterface)
    : SerialNetworkInterface(*new CdmaNetworkInterfacePrivate(this), networkinterface)
{
    Q_D(CdmaNetworkInterface);
    d->setBackendObject(networkinterface.d_ptr->backendObject());
    makeConnections( networkinterface.d_ptr->backendObject() );
}

Solid::Control::CdmaNetworkInterface::CdmaNetworkInterface(CdmaNetworkInterfacePrivate &dd, QObject *backendObject)
    : SerialNetworkInterface(dd, backendObject)
{
    makeConnections( backendObject );
}

Solid::Control::CdmaNetworkInterface::CdmaNetworkInterface(CdmaNetworkInterfacePrivate &dd, const CdmaNetworkInterface &networkinterface)
    : SerialNetworkInterface(dd, networkinterface.d_ptr->backendObject())
{
    makeConnections( networkinterface.d_ptr->backendObject() );
}

Solid::Control::CdmaNetworkInterface::~CdmaNetworkInterface()
{
}

Solid::Control::NetworkInterface::Type Solid::Control::CdmaNetworkInterface::type() const
{
    return Cdma;
}

void Solid::Control::CdmaNetworkInterface::makeConnections(QObject * source)
{
}

void Solid::Control::CdmaNetworkInterfacePrivate::setBackendObject(QObject *object)
{
    SerialNetworkInterfacePrivate::setBackendObject(object);
}

void Solid::Control::CdmaNetworkInterface::_k_destroyed(QObject *object)
{
    Q_UNUSED(object);
}
// vim: sw=4 sts=4 et tw=100
