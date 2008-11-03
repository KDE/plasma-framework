/*  This file is part of the KDE project
    Copyright (C) 2006 Will Stephenson <wstephenson@kde.org>
    Copyright (C) 2006 Kevin Ottens <ervin@kde.org>
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

#ifndef SOLID_CONTROL_BLUETOOTHMANAGER_H
#define SOLID_CONTROL_BLUETOOTHMANAGER_H

#include <QtCore/QObject>

#include "singletondefs.h"
#include "solid_control_export.h"

#include "bluetoothinterface.h"
#include "bluetoothinputdevice.h"

class KJob;

namespace Solid
{
namespace Control
{
namespace Ifaces
{
    class BluetoothManager;
}

class BluetoothInterface;
class BluetoothSecurity;

typedef QList<BluetoothInterface> BluetoothInterfaceList;
typedef QList<BluetoothInputDevice> BluetoothInputDeviceList;
class BluetoothManagerPrivate;

/**
 * This class specifies the interface a backend will have to implement in
 * order to be used in the system.
 *
 * A bluetooth manager allow to query the underlying platform to discover the
 * available bluetooth interfaces.
 */
class SOLIDCONTROL_EXPORT BluetoothManager : public QObject
{
    Q_OBJECT
    SOLID_SINGLETON(BluetoothManager)

public:
    /**
     * Retrieves the list of all the bluetooth interfaces Universal Bluetooth Identifiers (UBIs)
     * in the system.
     *
     * @return the list of bluetooth interfaces available in this system
     */
    //BluetoothInterfaceList bluetoothInterfaces() const;
    QStringList bluetoothInterfaces() const;

    /**
     * Retrieves the default bluetooth interface Universal Bluetooth Identifiers (UBIs)
     * of the system.
     *
     * @return the UBI of the default bluetooth interface
     */
    QString defaultInterface() const;

    /**
     * Returns the UBI of the Bluetooth Interface given its name (e.g. 'hci0'),
     * if found on the system.
     *
     * @return the found UBI of the named bluetooth interface
     */
    QString findInterface(const QString & name) const;


    /**
     * Find a new BluetoothInterface object given its UBI.
     *
     * @param ubi the identifier of the bluetooth interface to find
     * @returns a valid BlueoothInterface object if there's a device having the given UBI, an invalid one otherwise
     */
    BluetoothInterface findBluetoothInterface(const QString  & ubi) const;


    /**
     * Find a new BluetoothInputDevice object given its UBI.
     *
     * @param ubi the identifier of the bluetooth input device to find
     * @returns a valid BlueoothInputDevice object if there's a device having the given UBI, an invalid one otherwise
     */
  //  BluetoothInputDevice findBluetoothInputDevice(const QString  & ubi) const;

    /**
     * Retrieves the list of Universal Bluetooth Identifiers (UBIs) of bluetooth input devices
     * which are configured in the system. Configured means also not connected devices.
     *
     * @return the list of bluetooth input devices configured in this system
     */
    //BluetoothInputDeviceList bluetoothInputDevices() const;

    /**
     * Instantiates a new BluetoothInputDevice object from this backend given its UBI.
     *
     * @param ubi the identifier of the bluetooth input device instantiated
     * @returns a new BluetoothInputDevice object if there's a device having the given UBI, 0 otherwise
     */
    //BluetoothInputDevice *createBluetoothInputDevice(const QString &ubi);

    /**
     * Setup a new bluetooth input device.
     *
     * @param ubi the ubi of the bluetooth input device
     * @returns the job handling of the operation.
     */
    //KJob *setupInputDevice(const QString &ubi);

    /**
     * Gets an instance of BluetoothSecurity to handle pairing/authorization requests
     */
    //BluetoothSecurity *security(const QString &interface = QString());

public Q_SLOTS:
    /**
     * Remove the configuraiton of a bluetooth input device.
     *
     * @param ubi the bluetooth input device identifier
     */
//  void removeInputDevice(const QString  & ubi);

Q_SIGNALS:
    /**
     * This signal is emitted when a new bluetooth interface is available.
     *
     * @param ubi the bluetooth interface identifier
     */
    void interfaceAdded(const QString  & ubi);

    /**
     * This signal is emitted when a bluetooth interface is not available anymore.
     *
     * @param ubi the bluetooth interface identifier
     */
    void interfaceRemoved(const QString  & ubi);

    /**
     * This signal is emitted when the default bluetooth interface changed.
     *
     * @param ubi the bluetooth interface identifier
     */
    void defaultInterfaceChanged(const QString  & ubi);

    /**
     * This signal is emitted when a new bluetooth input device got configured/created.
     *
     * @param ubi the bluetooth input device identifier
     */
//  void inputDeviceCreated(const QString  & ubi);

    /**
     * This signal is emitted when a bluetooth input device configuration is not available anymore.
     *
     * @param ubi the bluetooth input device identifier
     */
//  void inputDeviceRemoved(const QString  & ubi);


private:
    BluetoothManager();
    ~BluetoothManager();

    //BluetoothInterfaceList buildDeviceList(const QStringList  & ubiList) const;

    Q_PRIVATE_SLOT(d, void _k_interfaceAdded(const QString &))
    Q_PRIVATE_SLOT(d, void _k_interfaceRemoved(const QString &))
    Q_PRIVATE_SLOT(d, void _k_interfaceDestroyed(QObject *))
/*
    Q_PRIVATE_SLOT(d, void _k_inputDeviceCreated(const QString &))
    Q_PRIVATE_SLOT(d, void _k_inputDeviceRemoved(const QString &))
    Q_PRIVATE_SLOT(d, void _k_inputDeviceDestroyed(QObject *))
*/

    BluetoothManagerPrivate * const d;
    friend class BluetoothManagerPrivate;
};
} // Control
} // Solid

#endif
