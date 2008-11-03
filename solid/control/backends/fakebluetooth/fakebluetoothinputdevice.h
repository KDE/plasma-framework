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

#ifndef FAKE_BLUETOOTH_INPUTDEVICE_H
#define FAKE_BLUETOOTH_INPUTDEVICE_H

#include <QString>
#include <QVariant>

#include <kdemacros.h>

#include <solid/control/ifaces/bluetoothinputdevice.h>

using namespace Solid::Control::Ifaces;

class KDE_EXPORT FakeBluetoothInputDevice : public Solid::Control::Ifaces::BluetoothInputDevice
{
    Q_OBJECT
    Q_INTERFACES(Solid::Control::Ifaces::BluetoothInputDevice)
public:
    FakeBluetoothInputDevice(const QMap<QString, QVariant>  & propertyMap,
                             QObject *parent = 0);
    virtual ~FakeBluetoothInputDevice();
    QString ubi() const
    {
        return mUbi;
    }
    bool isConnected() const
    {
        return mConnected;
    }
    QString address() const
    {
        return mAddress;
    }
    QString name() const
    {
        return mName;
    }
    QString productID() const
    {
        return mProductID;
    }
    QString vendorID() const
    {
        return mVendorID;
    }

private:
    QString mUbi;
    bool mConnected;
    QString mAddress;
    QString mName;
    QString mVendorID;
    QString mProductID;

public Q_SLOTS:
    void slotConnected()
    {
        mConnected = true;
    }
    void slotDisconnected()
    {
        mConnected = false;
    }

Q_SIGNALS:
    void connected();
    void disconnected();

protected:
    QMap<QString, QVariant> mPropertyMap;

};

#endif
