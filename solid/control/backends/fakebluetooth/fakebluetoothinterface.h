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

#ifndef FAKE_BLUETOOTH_INTERFACE_H
#define FAKE_BLUETOOTH_INTERFACE_H

#include <QString>
#include <QVariant>

#include <kdebug.h>

#include <kdemacros.h>

#include <solid/control/ifaces/bluetoothinterface.h>

#include "fakebluetoothremotedevice.h"

using namespace Solid::Control::Ifaces;

class KDE_EXPORT FakeBluetoothInterface : public Solid::Control::Ifaces::BluetoothInterface
{
    friend class FakeBluetoothManager;
    Q_OBJECT
    Q_INTERFACES(Solid::Control::Ifaces::BluetoothInterface)
public:
    FakeBluetoothInterface(const QMap<QString, QVariant>  & propertyMap,
                           QObject *parent = 0);
    virtual ~FakeBluetoothInterface();

    QObject * createBluetoothRemoteDevice(const QString  & ubi);
    QStringList bluetoothRemoteDevices() const;

    QString ubi() const
    {
        return mUbi;
    }

    QString address() const
    {
        kDebug() ; return mAddress;
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
    Solid::Control::BluetoothInterface::Mode mode() const
    {
        return mMode;
    }
    int discoverableTimeout() const
    {
        return mDiscoverableTimeout;
    }
    bool isDiscoverable() const
    {
        return mDiscoverable;
    }
    QStringList listConnections() const
    {
        return mConnections;
    }
    QString majorClass() const
    {
        return mMajorClass;
    }
    QStringList listAvailableMinorClasses() const
    {
        return mMinorClasses;
    }
    QString minorClass() const
    {
        return mMinorClass;
    }
    QStringList serviceClasses() const
    {
        return mServiceClasses;
    }
    QString name() const
    {
        return mName;
    }
    QString getRemoteName(const QString &)
    {
	return "";
    }
    QStringList listBondings() const
    {
        return mBondings;
    }
    bool isPeriodicDiscoveryActive() const
    {
        return mPeriodicDiscovery;
    }
    bool isPeriodicDiscoveryNameResolvingActive() const
    {
        return mPeriodicDiscoveryNameResolving;
    }
    QStringList listRemoteDevices() const
    {
        return mRemoteDevices;
    }
    QStringList listRecentRemoteDevices(const QDateTime &) const
    {
        return mRecentRemoteDevices;
    }
    bool isTrusted(const QString& mac)
    {
	return true;
    }

    void injectDevice(const QString &, FakeBluetoothRemoteDevice *);
private:
    int mTimeout;
    QString mName;
    QString mUbi;
    QString mAddress;
    QString mVersion;
    QString mRevision;
    QString mManufacturer;
    QString mCompany;
    Solid::Control::BluetoothInterface::Mode mMode;
    int mDiscoverableTimeout;
    bool mDiscoverable;
    QStringList mConnections;
    QString mMajorClass;
    QStringList mMinorClasses;
    QString mMinorClass;
    QStringList mServiceClasses;
    QStringList mBondings;
    bool mPeriodicDiscovery;
    bool mPeriodicDiscoveryNameResolving;
    QStringList mRemoteDevices;
    QStringList mRecentRemoteDevices;
    QString mSetTrusted;
    QString mRemoveTrust;


public Q_SLOTS:
    void setMode(const Solid::Control::BluetoothInterface::Mode mode)
    {
        mMode = mode; /* emit modeChanged(mMode); */
    }
    void setDiscoverableTimeout(int timeout)
    {
        mTimeout = timeout;
    }
    void setMinorClass(const QString &minorClass)
    {
        mMinorClass = minorClass;
    }
    void setName(const QString &name)
    {
        mName = name;
    }
    void discoverDevices();
    void discoverDevicesWithoutNameResolving()
    {}
    void cancelDiscovery()
    {}
    void startPeriodicDiscovery()
    {}
    void stopPeriodicDiscovery()
    {}
    void setPeriodicDiscoveryNameResolving(bool resolving)
    {
        mPeriodicDiscoveryNameResolving = resolving;
    }
    void setTrusted(const QString& mac) 
    {
	mSetTrusted = mac;
    }
    void removeTrust(const QString& mac) 
    {
	mRemoveTrust = mac;
    }

Q_SIGNALS:
    void modeChanged(Solid::Control::BluetoothInterface::Mode mode);
    void discoverableTimeoutChanged(int timeout);
    void minorClassChanged(const QString &minor);
    void nameChanged(const QString &name);
    void discoveryStarted();
    void discoveryCompleted();
    void remoteDeviceFound(const QString &ubi, int deviceClass, int rssi);
    void remoteDeviceDisappeared(const QString &ubi);

    void remoteNameUpdated(const QString &address, const QString &name);
    void remoteDeviceConnected(const QString &address);
    void remoteDeviceDisconnected(const QString &address);
    void trustAdded(const QString &address);
    void trustRemoved(const QString &address);
    void bondingCreated(const QString &address);
    void bondingRemoved(const QString &address);
protected:
    /* These methods are operations that are carried out by the manager
       In a real backend they would be events coming up from the network layer */
    void injectBluetooth(const QString  & ubi, FakeBluetoothRemoteDevice * device);
    QMap<QString, FakeBluetoothRemoteDevice *> mBluetoothRemoteDevices;
    QMap<QString, QVariant> mPropertyMap;
};

#endif
