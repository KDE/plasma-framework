/*  This file is part of the KDE project
    Copyright (C) 2005 Kevin Ottens <ervin@kde.org>
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

#include "solidbluetoothtest.h"

#include <qtest_kde.h>

#include <solid/control/bluetoothmanager.h>
#include <solid/control/bluetoothinterface.h>
#include "solid/control/managerbase_p.h"
#include <kdebug.h>

#include <fakebluetoothmanager.h>

#ifndef FAKE_BLUETOOTH_XML
#error "FAKE_BLUETOOTH_XML not set. An XML file describing a bluetooth context is required for this test"
#endif

QTEST_KDEMAIN_CORE(SolidBluetoothTest)

void SolidBluetoothTest::initTestCase()
{
    fakeManager = new FakeBluetoothManager(0, QStringList(), FAKE_BLUETOOTH_XML);
    Solid::Control::ManagerBasePrivate::_k_forcePreloadedBackend("Solid::Control::Ifaces::BluetoothManager", fakeManager);
}

void SolidBluetoothTest::testBluetoothInterfaces()
{
    Solid::Control::BluetoothManager &manager = Solid::Control::BluetoothManager::self();

    Solid::Control::BluetoothInterfaceList interfaces = manager.bluetoothInterfaces();

    // Verify that the framework reported correctly the interfaces available
    // in the backend.
    QSet<QString> expected_ubis, received_ubis;

    expected_ubis = QSet<QString>::fromList(fakeManager->bluetoothInterfaces());

    foreach (Solid::Control::BluetoothInterface iface , interfaces) {
        received_ubis << iface.ubi();
    }

    QCOMPARE(expected_ubis, received_ubis);
}

void SolidBluetoothTest::testManagerBasicFeatures()
{
    //Solid::Control::BluetoothManager &manager = Solid::Control::BluetoothManager::self();

}

void SolidBluetoothTest::testInterfaceBasicFeatures()
{
    //Solid::Control::BluetoothManager &manager = Solid::Control::BluetoothManager::self();

    // Retrieve a valid BluetoothInterface object
    Solid::Control::BluetoothInterface valid_iface("/org/kde/solid/fakebluetooth/hci0");

}

#include "solidbluetoothtest.moc"

