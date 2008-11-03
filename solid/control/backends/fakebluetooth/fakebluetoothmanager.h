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

#ifndef FAKE_BLUETOOTH_MANAGER_H
#define FAKE_BLUETOOTH_MANAGER_H

#include <QObject>
#include <QStringList>

#include <kdemacros.h>

#include <solid/control/ifaces/bluetoothmanager.h>
#include "fakebluetoothinterface.h"
#include "fakebluetoothinputdevice.h"
#include "fakebluetoothsecurity.h"

class QDomElement;

class KDE_EXPORT FakeBluetoothManager : public Solid::Control::Ifaces::BluetoothManager
{
    Q_OBJECT
public:
    FakeBluetoothManager(QObject * parent, const QStringList  & args);
    FakeBluetoothManager(QObject * parent, const QStringList  & args, const QString  & xmlFile);
    ~FakeBluetoothManager();
    QStringList bluetoothInterfaces() const;
    FakeBluetoothInterface *createBluetoothInterface(const QString &);
    QString defaultInterface() const;
    QObject *createInterface(const QString &);

    QStringList bluetoothInputDevices() const;
    FakeBluetoothInputDevice *createBluetoothInputDevice(const QString &ubi);
    KJob *setupInputDevice(const QString &ubi);
    Solid::Control::Ifaces::BluetoothSecurity * security(const QString &interface);
public Q_SLOTS:
    void removeInputDevice(const QString  & ubi);

private:
    void parseBluetoothFile();
    FakeBluetoothInterface *parseDeviceElement(const QDomElement &deviceElement);
    QMap<QString, QVariant> parseBluetoothElement(const QDomElement &deviceElement);

    QMap<QString, FakeBluetoothInterface *> mBluetoothInterfaces;
    QMap<QString, FakeBluetoothInputDevice *> mBluetoothInputDevices;
    QMap<QString, FakeBluetoothRemoteDevice *> mBluetoothRemoteDevices;
    QString mXmlFile;
};

#endif
