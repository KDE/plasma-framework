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

#ifndef FAKE_BLUETOOTH_REMOTEDEVICE_H
#define FAKE_BLUETOOTH_REMOTEDEVICE_H

#include <QString>
#include <QVariant>

#include <kdemacros.h>

#include <solid/control/ifaces/bluetoothremotedevice.h>

using namespace Solid::Control::Ifaces;

class KDE_EXPORT FakeBluetoothRemoteDevice : public Solid::Control::Ifaces::BluetoothRemoteDevice
{
    Q_OBJECT
    Q_INTERFACES(Solid::Control::Ifaces::BluetoothRemoteDevice)
public:
    FakeBluetoothRemoteDevice(const QMap<QString, QVariant>  & propertyMap,
                              QObject *parent = 0);
    virtual ~FakeBluetoothRemoteDevice();
    QString ubi() const
    {
        return mUbi;
    }
    QString address() const
    {
        return mAddress;
    }
    bool isConnected() const
    {
        return mConnected;
    }
    QString version() const
    {
        return mVersion;
    }
    QString revision() const
    {
        return mRevision;
    }
    QString manufacturer() const
    {
        return mManufacturer;
    }
    QString company() const
    {
        return mCompany;
    }
    QString majorClass() const
    {
        return mMajorClass;
    }
    QString minorClass() const
    {
        return mMinorClass;
    }
    QStringList serviceClasses() const
    {
        return mServiceClass;
    }
    QString name() const
    {
        return mName;
    }
    QString alias() const
    {
        return mAlias;
    }
    QString lastSeen() const
    {
        return mLastSeen;
    }
    QString lastUsed() const
    {
        return mLastUsed;
    }
    bool hasBonding() const
    {
        return mBonded;
    }
    int pinCodeLength() const
    {
        return mPinCodeLength;
    }
    int encryptionKeySize() const
    {
        return mEncryptionKeySize;
    }
    KJob *createBonding();

private:
    QString mUbi;
    QString mAddress;
    bool mConnected;
    QString mVersion;
    QString mRevision;
    QString mManufacturer;
    QString mCompany;
    QString mMajorClass;
    QString mMinorClass;
    QStringList mServiceClass;
    QString mName;
    QString mAlias;
    QString mLastSeen;
    QString mLastUsed;
    bool mBonded;
    int mPinCodeLength;
    int mEncryptionKeySize;
    bool servicesFound;

public Q_SLOTS:
    void setAlias(const QString &alias)
    {
        mAlias = alias;
    }
    void clearAlias()
    {
        mAlias = "";
    }
    void disconnect()
    {
        mConnected = false;
    }
    void cancelBondingProcess()
    {}
    void removeBonding()
    {
        mBonded = false;
    }
    void serviceHandles(const QString &) const
    {
	    
    }
    void serviceRecordAsXml(uint ) const
    {
    }
Q_SIGNALS:
    void classChanged(uint devClass);
    void nameChanged(const QString &name);
    void nameResolvingFailed();
    void aliasChanged(const QString &alias);
    void aliasCleared();
    void connected();
    void requestDisconnection();
    void disconnected();
    void bondingCreated();
    void bondingRemoved();
    void serviceRecordXmlAvailable(const QString &ubi, const QString &record);
    void serviceHandlesAvailable(const QString &ubi, const QList<uint> &handles);
protected:
    QMap<QString, QVariant> mPropertyMap;

};

#endif
