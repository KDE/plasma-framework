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

#include "networkinterface.h"
#include "networkinterface_p.h"

#include "soliddefs_p.h"
#include "ifaces/networkinterface.h"

Solid::Control::NetworkInterface::NetworkInterface(QObject *backendObject)
    : QObject(), d_ptr(new NetworkInterfacePrivate(this))
{
    Q_D(NetworkInterface);
    d->setBackendObject(backendObject);
}

Solid::Control::NetworkInterface::NetworkInterface(const NetworkInterface &other)
    : QObject(), d_ptr(new NetworkInterfacePrivate(this))
{
    Q_D(NetworkInterface);
    d->setBackendObject(other.d_ptr->backendObject());
}

Solid::Control::NetworkInterface::NetworkInterface(NetworkInterfacePrivate &dd, QObject *backendObject)
    : QObject(), d_ptr(&dd)
{
    Q_UNUSED(backendObject);
}

Solid::Control::NetworkInterface::NetworkInterface(NetworkInterfacePrivate &dd, const NetworkInterface &other)
    : d_ptr(&dd)
{
    Q_UNUSED(other);
}

Solid::Control::NetworkInterface::~NetworkInterface()
{
    delete d_ptr;
}

Solid::Control::NetworkInterface::Type Solid::Control::NetworkInterface::type() const
{
    return UnknownType;
}

QString Solid::Control::NetworkInterface::uni() const
{
    Q_D(const NetworkInterface);
    return_SOLID_CALL(Ifaces::NetworkInterface *, d->backendObject(), QString(), uni());
}

QString Solid::Control::NetworkInterface::interfaceName() const
{
    Q_D(const NetworkInterface);
    return_SOLID_CALL(Ifaces::NetworkInterface *, d->backendObject(), QString(), interfaceName());
}

QString Solid::Control::NetworkInterface::driver() const
{
    Q_D(const NetworkInterface);
    return_SOLID_CALL(Ifaces::NetworkInterface *, d->backendObject(), QString(), driver());
}

Solid::Control::IPv4Config Solid::Control::NetworkInterface::ipV4Config() const
{
    Q_D(const NetworkInterface);
    return_SOLID_CALL(Ifaces::NetworkInterface *, d->backendObject(), Solid::Control::IPv4Config(), ipV4Config() );
}

bool Solid::Control::NetworkInterface::isActive() const
{
    Q_D(const NetworkInterface);
    return_SOLID_CALL(Ifaces::NetworkInterface *, d->backendObject(), false, isActive());
}

Solid::Control::NetworkInterface::ConnectionState Solid::Control::NetworkInterface::connectionState() const
{
    Q_D(const NetworkInterface);
    return_SOLID_CALL(Ifaces::NetworkInterface *, d->backendObject(), UnknownState, connectionState());
}

int Solid::Control::NetworkInterface::designSpeed() const
{
    Q_D(const NetworkInterface);
    return_SOLID_CALL(Ifaces::NetworkInterface *, d->backendObject(), 0, designSpeed());
}

Solid::Control::NetworkInterface::Capabilities Solid::Control::NetworkInterface::capabilities() const
{
    Q_D(const NetworkInterface);
    return_SOLID_CALL(Ifaces::NetworkInterface *, d->backendObject(), Capabilities(), capabilities());
}

void Solid::Control::NetworkInterfacePrivate::setBackendObject(QObject *object)
{
    FrontendObjectPrivate::setBackendObject(object);

    if (object) {
        QObject::connect(object, SIGNAL(connectionStateChanged(int)),
                         parent(), SIGNAL(connectionStateChanged(int)));
    }
}


#include "networkinterface.moc"
