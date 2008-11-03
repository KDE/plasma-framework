/*  This file is part of the KDE project
    Copyright (C) 2007 Will Stephenson <wstephenson@kde.org>
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

#ifndef SOLID_IFACES_BLUETOOTHMANAGER
#define SOLID_IFACES_BLUETOOTHMANAGER

#include <QtCore/QObject>
#include "../solid_control_export.h"

class KJob;

namespace Solid
{
namespace Control
{
namespace Ifaces
{
class BluetoothSecurity;
/**
 * This class specifies the interface a backend will have to implement in
 * order to be used in the system.
 *
 * A bluetooth manager allow to query the underlying platform to discover the
 * available bluetooth interfaces.
 */
class SOLIDCONTROLIFACES_EXPORT BluetoothManager : public QObject
{
    Q_OBJECT
public:
    /**
     * Constructs a BluetoothManager.
     *
     * @param parent the parent object
     */
    BluetoothManager(QObject * parent = 0);
    /**
     * Destructs a BluetoothManager object.
     */
    virtual ~BluetoothManager();

    /**
     * Retrieves the list of all the bluetooth interfaces Universal Bluetooth Identifiers (UBIs)
     * in the system.
     *
     * @return the list of bluetooth interfaces available in this system
     */
    virtual QStringList bluetoothInterfaces() const = 0;

    /**
     * Retrieves the default bluetooth interface Universal Bluetooth Identifiers (UBIs)
     * of the system.
     *
     * @return the UBI of the default bluetooth interface
     */
    virtual QString defaultInterface() const = 0;

    /**
     * Returns the UBI of the Bluetooth Interface given its name (e.g. 'hci0'),
     * if found on the system.
     *
     * @return the found UBI of the named bluetooth interface
     */
    virtual QString findInterface(const QString &name) const = 0;

    /**
     * Instantiates a new BluetoothInterface object from this backend given its UBI.
     *
     * @param ubi the identifier of the bluetooth interface instantiated
     * @returns a new BluetoothInterface object if there's a device having the given UBI, 0 otherwise
     */
     virtual QObject *createInterface(const QString &ubi) = 0;

    /**
     * Retrieves the list of Universal Bluetooth Identifiers (UBIs) of bluetooth input devices
     * which are configured in the system. Configured means also not connected devices.
     *
     * @return the list of bluetooth input devices configured in this system
     */
//  virtual QStringList bluetoothInputDevices() const = 0;

    /**
     * Instantiates a new BluetoothInputDevice object from this backend given its UBI.
     *
     * @param ubi the identifier of the bluetooth input device instantiated
     * @returns a new BluetoothInputDevice object if there's a device having the given UBI, 0 otherwise
     */
//  virtual QObject *createBluetoothInputDevice(const QString &ubi) = 0;

    /**
     * Setup a new bluetooth input device.
     *
     * @param ubi the ubi of the bluetooth input device
     * @returns job handling of the operation.
     */
//  virtual KJob *setupInputDevice(const QString &ubi) = 0;
    
    /**
     * Gets an instance of BluetoothSecurity to handle pairing/authorization requests
     */
//  virtual Solid::Control::Ifaces::BluetoothSecurity* security(const QString &interface) = 0;

public Q_SLOTS:
    /**
     * Remove the configuraiton of a bluetooth input device.
     *
     * @param ubi the bluetooth input device identifier
     */
//  virtual void removeInputDevice(const QString  & ubi) = 0;

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
//   void inputDeviceCreated(const QString  & ubi);

    /**
     * This signal is emitted when a bluetooth input device configuration is not available anymore.
     *
     * @param ubi the bluetooth input device identifier
     */
//    void inputDeviceRemoved(const QString  & ubi);

};

} // Ifaces

} // Control

} // Solid

#endif
