/*  This file is part of the KDE project
    Copyright (C) 2006 Will Stephenson <wstephenson@kde.org>
    Copyright (C) 2007 Daniel Gollub <dgollub@suse.de>
    Copyright (C) 2008 Tom Patzig <tpatzig@suse.de>


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

#include <QMap>
#include <QStringList>

#include "ifaces/bluetoothremotedevice.h"

#include "frontendobject_p.h"

#include "soliddefs_p.h"
#include "bluetoothmanager.h"
#include "bluetoothinterface.h"

namespace Solid
{
namespace Control
{
    class BluetoothRemoteDevicePrivate : public FrontendObjectPrivate
    {
    public:
        BluetoothRemoteDevicePrivate(QObject *parent)
            : FrontendObjectPrivate(parent) { }

        void setBackendObject(QObject *object);
    };
}
}

Solid::Control::BluetoothRemoteDevice::BluetoothRemoteDevice(QObject *backendObject)
    : QObject(), d_ptr(new BluetoothRemoteDevicePrivate(this))
{
    Q_D(BluetoothRemoteDevice);
    d->setBackendObject(backendObject);
}

Solid::Control::BluetoothRemoteDevice::BluetoothRemoteDevice(const BluetoothRemoteDevice &device)
    : QObject(), d_ptr(new BluetoothRemoteDevicePrivate(this))
{
    Q_D(BluetoothRemoteDevice);
    d->setBackendObject(device.d_ptr->backendObject());
}

Solid::Control::BluetoothRemoteDevice::~BluetoothRemoteDevice()
{}

Solid::Control::BluetoothRemoteDevice &Solid::Control::BluetoothRemoteDevice::operator=(const Solid::Control::BluetoothRemoteDevice  & dev)
{
    Q_D(BluetoothRemoteDevice);
    d->setBackendObject(dev.d_ptr->backendObject());

    return *this;
}

bool Solid::Control::BluetoothRemoteDevice::operator==(const BluetoothRemoteDevice  & other) const
{
	return ubi() == other.ubi();
}

QString Solid::Control::BluetoothRemoteDevice::ubi() const
{
    Q_D(const BluetoothRemoteDevice);
    return_SOLID_CALL(Ifaces::BluetoothRemoteDevice *, d->backendObject(), QString(), ubi());
}

QMap<QString,QVariant> Solid::Control::BluetoothRemoteDevice::getProperties()
{
    Q_D(const BluetoothRemoteDevice);
    return_SOLID_CALL(Ifaces::BluetoothRemoteDevice *, d->backendObject(), (QMap< QString,QVariant >()), getProperties());
}

QStringList Solid::Control::BluetoothRemoteDevice::listNodes()
{
    Q_D(const BluetoothRemoteDevice);
    return_SOLID_CALL(Ifaces::BluetoothRemoteDevice *, d->backendObject(), QStringList(), listNodes());
}

void Solid::Control::BluetoothRemoteDevice::setProperty(const QString &name, const QVariant &value)
{
    Q_D(const BluetoothRemoteDevice);
    SOLID_CALL(Ifaces::BluetoothRemoteDevice *, d->backendObject(), setProperty(name,value));
}

void Solid::Control::BluetoothRemoteDevice::cancelDiscovery()
{
    Q_D(const BluetoothRemoteDevice);
    SOLID_CALL(Ifaces::BluetoothRemoteDevice *, d->backendObject(), cancelDiscovery());
}

void Solid::Control::BluetoothRemoteDevice::disconnect()
{
    Q_D(const BluetoothRemoteDevice);
    SOLID_CALL(Ifaces::BluetoothRemoteDevice *, d->backendObject(), disconnect());
}



/*
QString Solid::Control::BluetoothRemoteDevice::address() const
{
    Q_D(const BluetoothRemoteDevice);
    return_SOLID_CALL(Ifaces::BluetoothRemoteDevice *, d->backendObject(), QString(), address());
}

bool Solid::Control::BluetoothRemoteDevice::isConnected() const
{
    Q_D(const BluetoothRemoteDevice);
    return_SOLID_CALL(Ifaces::BluetoothRemoteDevice *, d->backendObject(), false, isConnected());
}

QString Solid::Control::BluetoothRemoteDevice::version() const
{
    Q_D(const BluetoothRemoteDevice);
    return_SOLID_CALL(Ifaces::BluetoothRemoteDevice *, d->backendObject(), QString(), version());
}

QString Solid::Control::BluetoothRemoteDevice::revision() const
{
    Q_D(const BluetoothRemoteDevice);
    return_SOLID_CALL(Ifaces::BluetoothRemoteDevice *, d->backendObject(), QString(), revision());
}

QString Solid::Control::BluetoothRemoteDevice::manufacturer() const
{
    Q_D(const BluetoothRemoteDevice);
    return_SOLID_CALL(Ifaces::BluetoothRemoteDevice *, d->backendObject(), QString(), manufacturer());
}

QString Solid::Control::BluetoothRemoteDevice::company() const
{
    Q_D(const BluetoothRemoteDevice);
    return_SOLID_CALL(Ifaces::BluetoothRemoteDevice *, d->backendObject(), QString(), company());
}

QString Solid::Control::BluetoothRemoteDevice::majorClass() const
{
    Q_D(const BluetoothRemoteDevice);
    return_SOLID_CALL(Ifaces::BluetoothRemoteDevice *, d->backendObject(), QString(), majorClass());
}

QString Solid::Control::BluetoothRemoteDevice::minorClass() const
{
    Q_D(const BluetoothRemoteDevice);
    return_SOLID_CALL(Ifaces::BluetoothRemoteDevice *, d->backendObject(), QString(), minorClass());
}

QStringList Solid::Control::BluetoothRemoteDevice::serviceClasses() const
{
    Q_D(const BluetoothRemoteDevice);
    return_SOLID_CALL(Ifaces::BluetoothRemoteDevice *, d->backendObject(), QStringList(), serviceClasses());
}
QString Solid::Control::BluetoothRemoteDevice::name() const
{
    Q_D(const BluetoothRemoteDevice);
    return_SOLID_CALL(Ifaces::BluetoothRemoteDevice *, d->backendObject(), QString(), name());
}

QString Solid::Control::BluetoothRemoteDevice::alias() const
{
    Q_D(const BluetoothRemoteDevice);
    return_SOLID_CALL(Ifaces::BluetoothRemoteDevice *, d->backendObject(), QString(), alias());
}

QString Solid::Control::BluetoothRemoteDevice::lastSeen() const
{
    Q_D(const BluetoothRemoteDevice);
    return_SOLID_CALL(Ifaces::BluetoothRemoteDevice *, d->backendObject(), QString(), lastSeen());
}

QString Solid::Control::BluetoothRemoteDevice::lastUsed() const
{
    Q_D(const BluetoothRemoteDevice);
    return_SOLID_CALL(Ifaces::BluetoothRemoteDevice *, d->backendObject(), QString(), lastUsed());
}

bool Solid::Control::BluetoothRemoteDevice::hasBonding() const
{
    Q_D(const BluetoothRemoteDevice);
    return_SOLID_CALL(Ifaces::BluetoothRemoteDevice *, d->backendObject(), false, hasBonding());
}

int Solid::Control::BluetoothRemoteDevice::pinCodeLength() const
{
    Q_D(const BluetoothRemoteDevice);
    return_SOLID_CALL(Ifaces::BluetoothRemoteDevice *, d->backendObject(), 0, pinCodeLength());
}

int Solid::Control::BluetoothRemoteDevice::encryptionKeySize() const
{
    Q_D(const BluetoothRemoteDevice);
    return_SOLID_CALL(Ifaces::BluetoothRemoteDevice *, d->backendObject(), 0, encryptionKeySize());
}

KJob *Solid::Control::BluetoothRemoteDevice::createBonding()
{
    Q_D(BluetoothRemoteDevice);
    return_SOLID_CALL(Ifaces::BluetoothRemoteDevice *, d->backendObject(), 0, createBonding());
}

void Solid::Control::BluetoothRemoteDevice::setAlias(const QString &alias)
{
    Q_D(BluetoothRemoteDevice);
    SOLID_CALL(Ifaces::BluetoothRemoteDevice *, d->backendObject(), setAlias(alias));
}

void Solid::Control::BluetoothRemoteDevice::clearAlias()
{
    Q_D(BluetoothRemoteDevice);
    SOLID_CALL(Ifaces::BluetoothRemoteDevice *, d->backendObject(), clearAlias());
}

void Solid::Control::BluetoothRemoteDevice::disconnect()
{
    Q_D(BluetoothRemoteDevice);
    SOLID_CALL(Ifaces::BluetoothRemoteDevice *, d->backendObject(), disconnect());
}

void Solid::Control::BluetoothRemoteDevice::cancelBondingProcess()
{
    Q_D(BluetoothRemoteDevice);
    SOLID_CALL(Ifaces::BluetoothRemoteDevice *, d->backendObject(), cancelBondingProcess());
}

void Solid::Control::BluetoothRemoteDevice::removeBonding()
{
    Q_D(const BluetoothRemoteDevice);
    SOLID_CALL(Ifaces::BluetoothRemoteDevice *, d->backendObject(), removeBonding());
}
void Solid::Control::BluetoothRemoteDevice::serviceHandles(const QString &filter) const
{
    Q_D(const BluetoothRemoteDevice);
    SOLID_CALL(Ifaces::BluetoothRemoteDevice *, d->backendObject(), serviceHandles(filter));
}

void Solid::Control::BluetoothRemoteDevice::serviceRecordAsXml(uint handle) const
{
    Q_D(const BluetoothRemoteDevice);
    SOLID_CALL(Ifaces::BluetoothRemoteDevice *, d->backendObject(), serviceRecordAsXml(handle));
}
*/

void Solid::Control::BluetoothRemoteDevice::discoverServices(const QString &filter)
{
    Q_D(const BluetoothRemoteDevice);
    SOLID_CALL(Ifaces::BluetoothRemoteDevice *, d->backendObject(), discoverServices(filter));
}

void Solid::Control::BluetoothRemoteDevicePrivate::setBackendObject(QObject *object)
{
    FrontendObjectPrivate::setBackendObject(object);

    if (object) {
        /*
        QObject::connect(object, SIGNAL(classChanged(uint)),
                         parent(), SIGNAL(classChanged(uint)));
        QObject::connect(object, SIGNAL(nameChanged(const QString &)),
                         parent(), SIGNAL(nameChanged(const QString &)));
        QObject::connect(object, SIGNAL(nameResolvingFailed()),
                         parent(), SIGNAL(nameResolvingFailed()));
        QObject::connect(object, SIGNAL(aliasChanged(const QString &)),
                         parent(), SIGNAL(aliasChanged(const QString &)));
        QObject::connect(object, SIGNAL(aliasCleared()),
                         parent(), SIGNAL(aliasCleared()));
        QObject::connect(object, SIGNAL(connected()),
                         parent(), SIGNAL(connected()));
        QObject::connect(object, SIGNAL(requestDisconnection()),
                         parent(), SIGNAL(requestDisconnection()));
        QObject::connect(object, SIGNAL(disconnected()),
                         parent(), SIGNAL(disconnected()));
        QObject::connect(object, SIGNAL(bondingCreated()),
                         parent(), SIGNAL(bondingCreated()));
        QObject::connect(object, SIGNAL(bondingRemoved()),
                         parent(), SIGNAL(bondingRemoved()));
	QObject::connect(object, SIGNAL(serviceHandlesAvailable(const QString &, const QList<uint> &)),
			 parent(), SIGNAL(serviceHandlesAvailable(const QString &, const QList<uint> &)));
	QObject::connect(object, SIGNAL(serviceRecordXmlAvailable(const QString &, const QString &)),
			 parent(), SIGNAL(serviceRecordXmlAvailable(const QString &, const QString &)));
        */

        QObject::connect(object, SIGNAL(serviceDiscoverAvailable(const QString &, const QMap< uint,QString> &)),
                         parent(), SIGNAL(serviceDiscoverAvailable(const QString &, const QMap< uint,QString > &)));
        QObject::connect(object, SIGNAL(propertyChanged(const QString &, const QVariant &)),
                         parent(), SIGNAL(propertyChanged(const QString &, const QVariant &)));
        QObject::connect(object, SIGNAL(disconnectRequested()),
                         parent(), SIGNAL(disconnectRequested()));
        QObject::connect(object, SIGNAL(nodeCreated(const QString &)),
                         parent(), SIGNAL(nodeCreated(const QString &)));
        QObject::connect(object, SIGNAL(nodeRemoved(const QString &)),
                         parent(), SIGNAL(nodeRemoved(const QString &)));


	
    }
}

#include "bluetoothremotedevice.moc"
