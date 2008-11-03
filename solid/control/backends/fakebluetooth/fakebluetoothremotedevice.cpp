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

#include "fakebluetoothmanager.h"
#include "fakebluetoothinterface.h"
#include "fakebluetoothremotedevice.h"

#include <kdebug.h>

FakeBluetoothRemoteDevice::FakeBluetoothRemoteDevice(const QMap<QString, QVariant>  & propertyMap, QObject * parent)
        : Solid::Control::Ifaces::BluetoothRemoteDevice(parent), mPropertyMap(propertyMap)
{

    mUbi = mPropertyMap["ubi"].toString();

    kDebug() << "UBI: " << mUbi;

    mAddress = mPropertyMap["address"].toString();
    mConnected = mPropertyMap["connected"].toBool();
    mVersion = mPropertyMap["version"].toString();
    mRevision = mPropertyMap["revision"].toString();
    mManufacturer = mPropertyMap["manufacturer"].toString();
    mCompany = mPropertyMap["company"].toString();
    mMajorClass = mPropertyMap["majorClass"].toString();
    mMinorClass = mPropertyMap["minorClass"].toString();
    mServiceClass.append(mPropertyMap["serviceClasses"].toString());
    mName = mPropertyMap["name"].toString();
    mAlias = mPropertyMap["alias"].toString();
    mLastSeen = mPropertyMap["lastSeen"].toString();
    mLastUsed = mPropertyMap["lastUsed"].toString();
    mBonded = mPropertyMap["bonded"].toBool();
    mPinCodeLength = mPropertyMap["pinCodeLength"].toInt();
    mEncryptionKeySize = mPropertyMap["encryptionKeySize"].toInt();

}

FakeBluetoothRemoteDevice::~FakeBluetoothRemoteDevice()
{}

// TODO
KJob *FakeBluetoothRemoteDevice::createBonding()
{
    mBonded = true;
    return NULL;
}

#include "fakebluetoothremotedevice.moc"
