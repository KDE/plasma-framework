/*  This file is part of the KDE project
    Copyright (C) 2007 Daniel Gollub <dgollub@suse.de>


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

#include <QStringList>
#include <QTimer>

#include "fakebluetoothinterface.h"
#include "fakebluetoothremotedevice.h"

#include <kdebug.h>

FakeBluetoothInterface::FakeBluetoothInterface(const QMap<QString, QVariant>  & propertyMap, QObject * parent)
        : Solid::Control::Ifaces::BluetoothInterface(parent), mPropertyMap(propertyMap)
{
    mAddress = mPropertyMap["address"].toString();
    mVersion = mPropertyMap["version"].toString();
    mRevision = mPropertyMap["revision"].toString();
    mManufacturer = mPropertyMap["manufacturer"].toString();
    mCompany = mPropertyMap["company"].toString();
    mMode = (Solid::Control::BluetoothInterface::Mode) mPropertyMap["mode"].toInt();
    mDiscoverableTimeout = mPropertyMap["discoverableTimeout"].toInt();
    mDiscoverable = mPropertyMap["isDiscoverable"].toBool();
    mConnections.append(mPropertyMap["connections"].toString());
    mMajorClass = mPropertyMap["majorClass"].toString();
    mMinorClasses.append(mPropertyMap["minorClasses"].toString());
    mMinorClass = mPropertyMap["minorClass"].toString();
    mServiceClasses.append(mPropertyMap["serviceClasses"].toString());
    mName = mPropertyMap["name"].toString();
    mBondings.append(mPropertyMap["bondings"].toString());
    mPeriodicDiscovery = mPropertyMap["isPeriodicDiscovery"].toBool();
    mPeriodicDiscoveryNameResolving = mPropertyMap["periodicDiscoveryNameResolving"].toBool();
    mRemoteDevices.append(mPropertyMap["remoteDevices"].toString());
    mRecentRemoteDevices.append(mPropertyMap["recentRemoteDevices"].toString());

    mUbi = mPropertyMap["ubi"].toString();
}

FakeBluetoothInterface::~FakeBluetoothInterface()
{
    foreach (FakeBluetoothRemoteDevice *device, mBluetoothRemoteDevices) {
        kDebug() << "DEVICE: " << device->ubi();
        delete device;
    }

    mBluetoothRemoteDevices.clear();
}

QObject * FakeBluetoothInterface::createBluetoothRemoteDevice(const QString  & ubi)
{
    if (mBluetoothRemoteDevices.contains(ubi)) {
        kDebug() << "found " << ubi;
        return mBluetoothRemoteDevices[ubi];
    } else {
        kDebug() << "NOT found " << ubi;
        return 0;
    }
}

QStringList FakeBluetoothInterface::bluetoothRemoteDevices() const
{
    kDebug() ;
    return mBluetoothRemoteDevices.keys();
}

void FakeBluetoothInterface::discoverDevices()
{
    kDebug() ;
    QTimer::singleShot(1000, this,
                       SIGNAL(remoteDeviceFound(QString("/org/kde/solid/fakebluetooth/hci0/EE:EE:EE:EE:EE:EE"),
                                                0, 99)));
    QTimer::singleShot(5000, this,
                       SIGNAL(remoteDeviceFound(QString("/org/kde/solid/fakebluetooth/hci0/EE:FF:EE:FF:EE:FF"),
                                                0, 42)));

}

/*************************************/

void FakeBluetoothInterface::injectDevice(const QString &ubi , FakeBluetoothRemoteDevice *device)
{
    mBluetoothRemoteDevices.insert(ubi, device);
}

#include "fakebluetoothinterface.moc"
