/*  This file is part of the KDE project
    Copyright (C) 2006 Will Stephenson <wstephenson@kde.org>
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

// KDE-QTestLib include
#include <qtest_kde.h>

#include "fakebluetoothmanager.h"

#include "fakebluetoothtest.h"

QTEST_KDEMAIN_CORE(FakeBluetooth_Test)

void FakeBluetooth_Test::testFakeBluetooth()
{
    FakeBluetoothManager *fbm = new FakeBluetoothManager(0, QStringList(), TEST_DATA);
    QVERIFY(!fbm->bluetoothInterfaces().isEmpty());

    // Bluetooth Interface
    FakeBluetoothInterface *interface = static_cast<FakeBluetoothInterface *>(fbm->createBluetoothInterface("/org/kde/solid/fakebluetooth/hci0"));

    QCOMPARE(interface->ubi(), QString("/org/kde/solid/fakebluetooth/hci0"));
    QCOMPARE(interface->address(), QString("00:E0:98:DD:4E:FF"));
    QCOMPARE(interface->version(), QString("Bluetooth 1.1"));
    QCOMPARE(interface->revision(), QString("Build 525"));
    QCOMPARE(interface->name(), QString("Daniel's Laptop"));

    interface->setName("CrashDummy");
    QCOMPARE(interface->name(), QString("CrashDummy"));

    // Bluetooth Remote Devices
    QCOMPARE(interface->bluetoothRemoteDevices().count(), 4);
    kDebug() << "REMOTE DEVICES: " << interface->bluetoothRemoteDevices();
    FakeBluetoothRemoteDevice *device =
        static_cast<FakeBluetoothRemoteDevice *>(interface->createBluetoothRemoteDevice("/org/kde/solid/fakebluetooth/hci0/EE:EE:EE:EE:EE:EE"));

    QCOMPARE(device->name(), QString("N800"));
    QCOMPARE(device->alias(), QString("My N800"));

    // Bluetooth Interface
    interface = static_cast<FakeBluetoothInterface *>(fbm->createBluetoothInterface("/org/kde/solid/fakebluetooth/XXX"));

    QCOMPARE(interface->ubi(), QString("/org/kde/solid/fakebluetooth/XXX"));
    QCOMPARE(interface->address(), QString("AA:FF:EE:AA:FF:EE"));
    QCOMPARE(interface->version(), QString("Bluetooth 2.0"));
    QCOMPARE(interface->company(), QString(""));
    QCOMPARE(interface->name(), QString("BlueZ (0) sugga"));

    interface->setName("CrashDummy2");
    QCOMPARE(interface->name(), QString("CrashDummy2"));

    delete fbm;
}

#include "fakebluetoothtest.moc"
