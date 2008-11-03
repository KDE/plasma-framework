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

#ifndef SOLID_IFACES_BLUETOOTHINTERFACE_H
#define SOLID_IFACES_BLUETOOTHINTERFACE_H

#include <QtCore/QList>
#include "../solid_control_export.h"

#include "../bluetoothinterface.h"
#include "bluetoothremotedevice.h"

#include <QtCore/QObject>

namespace Solid
{
namespace Control
{
namespace Ifaces
{
/**
 * Represents a bluetooth interface as seen by the bluetooth subsystem.
 */
class SOLIDCONTROLIFACES_EXPORT BluetoothInterface : public QObject
{
    Q_OBJECT
public:
    /**
     * Create a BluetoothInterface.
     *
     * @param parent the parent object
     */
    BluetoothInterface(QObject *parent = 0);

    /**
     * Destructs a BluetoothInterface object.
     */
    virtual ~BluetoothInterface();

    /**
     * Retrieves the Universal Bluetooth Identifier (UBI) of the BluetoothInterface.
     * This identifier is unique for each bluetooth remote device and bluetooth interface in the system.
     *
     * @returns the Universal Bluetooth Identifier of the current bluetooth interface
     */
    virtual QString ubi() const = 0;

    /**
     * Retrieves the MAC address of the bluetooth interface/adapter.
     *
     * @returns MAC address of bluetooth interface
     */
    //virtual QString address() const = 0;

    /**
     * Retrieves the version of the chip of the bluetooth interface/adapter.
     * Example: "Bluetooth 2.0 + EDR"
     *
     * @returns bluetooth chip version
     */
    //virtual QString version() const = 0;

    /**
     * Retrieves the revision of the chip of the bluetooth interface/adapter.
     * Example: "HCI 19.2"
     *
     * @returns bluetooth chip revision
     */
    //virtual QString revision() const = 0;

    /**
     * Retrieves the name of the bluetooth chip manufacturer.
     * Example: "Cambdirge Silicon Radio"
     *
     * @returns manufacturer string of bluetooth interface/adapter
     */
    //virtual QString manufacturer() const = 0;

    /**
     * Retrieves the name of the bluetooth chip company.
     * Based on device address.
     *
     * @returns company string of bluetooth interface/adapter
     */
    //virtual QString company() const = 0;


    /**
     * Retrieves the current mode of the bluetooth interface/adapter.
     * Valid modes: "off", "connectable", "discoverable"
     *
     * @todo determine unify type for valid modes.. enum?! what about other bluetooth APIs?
     * three modes?
     *
     * @returns current mode of bluetooth interface/adaoter
     */
    //virtual Solid::Control::BluetoothInterface::Mode mode() const = 0;

    /**
     * Retrieves the discoverable timeout of the bluetooth interface/adapter.
     * Discoverable timeout of 0 means never disappear.
     *
     * @returns current discoverable timeout in seconds
     */
    //virtual int discoverableTimeout() const = 0;

    /**
     * Retrieves the current discoverable staut of the bluetooth interface/adapter.
     *
     * @returns current discoverable status of bluetooth interface/adapter
     */
    //virtual bool isDiscoverable() const = 0;


    /**
     * List all UBIs of connected remote bluetooth devices of this handled bluetooth
     * interface/adapter.
     *
     * @returns list UBIs of connected bluetooth remote devices
     */
    //virtual QStringList listConnections() const = 0;


    /**
     * Retrieves major class of the bluetooth interface/adapter.
     *
     * @returns current major class of the bluetooth interface/adapter
     */
    //virtual QString majorClass() const = 0;

    /**
     * List supported minor classes of the bluetooth interface/adapter.
     *
     * @returns list of supported minor classes by bluetooth interface/adapter
     */
    //virtual QStringList listAvailableMinorClasses() const = 0;

    /**
     * Retrievies minor class of the bluetooth interface/adapter.
     * Valid classes, see listAvailableMinorClasses()
     *
     * @returns minor class of the bluetooth interface/adapter.
     */
    //virtual QString minorClass() const = 0;

    /**
     * List services class of the bluetooth interface/adapter.
     *
     * @returns list of service classes or empty list if no services registered
     */
    //virtual QStringList serviceClasses() const = 0;
    /**
     * Retrieves name of bluetooth interface/adapter.
     *
     * @returns name of bluetooth interface/adapter
     */
    //virtual QString name() const = 0;

    /**
     * Returns the name of the remote device, given its mac address (mac).
     *
     * @return the name of the remote device
     */
    //virtual QString getRemoteName(const QString & mac) = 0;

    /**
     * List UBIs of bonded/paired remote bluetooth devices with this bluetooth
     * interface/adapter.
     *
     * @returns UBIs of bonded/paired bluetooth remote devices
     */
    //virtual QStringList listBondings() const = 0;

    /**
     * Periodic discovery status of this bluetooth interface/adapter.
     *
     * @returns true if periodic discovery is already active otherwise false
     */
    //virtual bool isPeriodicDiscoveryActive() const = 0;

    /**
     * Name resolving status of periodic discovery routing. 
     *
     * @returns true if name got resolved while periodic discovery of this bluetooth
     * interface/adapter
     */
    //virtual bool isPeriodicDiscoveryNameResolvingActive() const = 0;

    /**
     * List the Universal Bluetooth Identifier (UBI) of all known remote devices, which are
     * seen, used or paired/bonded.
     *
     * See listConnections()
     *
     * @returns a QStringList of UBIs of all known remote bluetooth devices
     */
    //virtual QStringList listRemoteDevices() const = 0;

    /**
     * List the Universal Bluetooth Identifier (UBI) of all known remote devices since a specific
     * datestamp. Known remote  devices means remote bluetooth which are seen, used or
     * paired/bonded.
     *
     * See listConnections(), listRemoteDevices()
     *
     * @param date the datestamp of the beginning of recent used devices
     * @returns a QStringList of UBIs of all known remote bluetooth devices
     */
  //  virtual QStringList listRecentRemoteDevices(const QDateTime &date) const = 0;

    /**
     * Returns true if the remote bluetooth device is trusted otherwise false.
     *
     * @param mac the address of the remote device
     */
//    virtual bool isTrusted(const QString &) = 0;

    /**
     *
     *
     *
     */


    virtual QString createPairedDevice(const QString &, const QString &, const QString &) const = 0;

    virtual QMap< QString, QVariant > getProperties() const = 0;

    virtual QStringList listDevices() const = 0;

    virtual QString findDevice(const QString &) const = 0;

    virtual QString createDevice(const QString &) const = 0;



//public Q_SLOTS:
    /**
     * Set mode of bluetooth interface/adapter.
     * Valid modes, see mode()
     *
     * @param mode the mode of the bluetooth interface/adapter
     */
    //virtual void setMode(const Solid::Control::BluetoothInterface::Mode mode) = 0;

    /**
     * Set discoverable timeout of bluetooth interface/adapter.
     *
     * @param timeout timeout in seconds
     */
    //virtual void setDiscoverableTimeout(int timeout) = 0;

    /**
     * Set minor class of bluetooth interface/adapter.
     *
     * @param minor set minor class. Valid mode see listAvaliableMinorClasses()
     */
    //virtual void setMinorClass(const QString &minor) = 0;

    /**
     * Set name of bluetooth interface/adapter.
     *
     * @param name the name of bluetooth interface/adapter
     */
    //virtual void setName(const QString &name) = 0;


    /**
     * Start discovery of remote bluetooth devices with device name resolving.
     */
    //virtual void discoverDevices() = 0;
    /**
     * Start discovery of remote bluetooth devices without device name resolving.
     */
    //virtual void discoverDevicesWithoutNameResolving() = 0;
    /**
     * Cancel discovery of remote bluetooth devices.
     */
    //virtual void cancelDiscovery() = 0;

    /**
     * Start periodic discovery of remote bluetooth devices.
     * See stopPeriodicDiscovery()
     */
    //virtual void startPeriodicDiscovery() = 0;

    /**
     * Stop periodic discovery of remote bluetooth devices.
     */
    //virtual void stopPeriodicDiscovery() = 0;

    /**
     * Enable/Disable name resolving of remote bluetooth devices in periodic discovery.
     *
     * @param resolveName true to enable name resolving otherwise false
     */
    //virtual void setPeriodicDiscoveryNameResolving(bool resolveNames) = 0;

    /**
     * Instantiates a new BluetoothRemoteDevice object from this backend given its address.
     *
     * @param ubi the identifier of the bluetooth remote device instantiated
     * @returns a new BluetoothRemoteDevice object if there's a device having the given UBI, 0 otherwise
     */
    virtual QObject *createBluetoothRemoteDevice(const QString &address) = 0;

    /**
     * Marks the device as trusted.
     *
     * @param mac the address of the remote device
     */
    //virtual void setTrusted(const QString &) = 0;

    /**
     * Marks the device as not trusted.
     *
     * @param mac the address of the remote device
     */
    //virtual void removeTrust(const QString &) = 0;

    virtual void registerAgent(const QString &,const QString &) = 0; 

    virtual void releaseSession() = 0;

    virtual void removeDevice(const QString &) = 0;

    virtual void requestSession() = 0;

    virtual void setProperty(const QString &, const QVariant &) = 0;

    virtual void startDiscovery() = 0;

    virtual void stopDiscovery() = 0;

    virtual void unregisterAgent(const QString &) = 0;

    virtual void cancelDeviceCreation(const QString &) = 0;


Q_SIGNALS:

    /**
     * This signal is emitted if the mode of the bluetooth interface/adapter has changed.
     * See mode() for valid modes.
     *
     * @param mode the changed mode
     */
    //void modeChanged(Solid::Control::BluetoothInterface::Mode);

    /**
     * The signal is emitted if the discoverable timeout of the bluetooth interface/adapter
     * has changed.
     *
     * @param timeout the changed timeout in seconds
     */
    //void discoverableTimeoutChanged(int timeout);

    /**
     * The signal is emitted if the minor class of the bluetooth interface/adapter has changed.
     *
     * @param minor the new minor class
     */
    //void minorClassChanged(const QString &minor);

    /**
     * The signal is emitted if the name of the bluetooth interface/adapter has changed.
     *
     * @param name the new name of the device
     */
    //void nameChanged(const QString &name);

    /**
     * This signal is emitted if a discovery has started.
     */
    //void discoveryStarted();

    /**
     * This signal is emitted if a discovery has completed.
     */
    //void discoveryCompleted();

    /**
     * This signal is emitted if the bluetooth interface/adapter detects a new remote bluetooth device.
     *
     * @todo change arguments types of deviceClass (uint32) and rssi (int16)
     *
     * @param ubi the new bluetooth identifier
     * @param deviceClass the device Class of the remote device
     * @param rssi the RSSI link of the remote device
     */
    //void remoteDeviceFound(const QString &ubi, int deviceClass, int rssi);

    /**
     * This signal is emitted if the bluetooth interface/adapter detectes a bluetooth device
     * disappeared.
     *
     * @param ubi the ubi of the disappering bluetooth remote device
     */
    //void remoteDeviceDisappeared(const QString &ubi);

    /**
     * This signal is emitted if the bluetooth interface/adapter detectes a new name for a 
     * bluetooth device.
     *
     * @param address the address of the bluetooth remote device
     * @param name the name of the bluetooth remote device
     */
    //void remoteNameUpdated(const QString &address, const QString &name);

    /**
     * This signal is emitted if a bluetooth connection has been created. 
     *
     * @param address the address of the connected bluetooth remote device
     */
    //void remoteDeviceConnected(const QString &address);

    /**
     * This signal is emitted if a bluetooth connection has been terminated.
     *
     * @param address the address of the disconnected bluetooth remote device
     */
    //void remoteDeviceDisconnected(const QString &address);

    /**
     * This signal is emitted if a bluetooth device was set trusted.
     *
     * @param address the address of the trusted bluetooth remote device
     */
    //void trustAdded(const QString &address);

    /**
     * This signal is emitted if the trust to the bluetooth device was removed. 
     *
     * @param address the address of the bluetooth remote device
     */
    //void trustRemoved(const QString &address);

    /**
     * This signal is emitted if a successful bonding has been created. 
     *
     * @param address the address of the bluetooth remote device
     */
    //void bondingCreated(const QString &address);

    /**
     * This signal is emitted if the bonding to a bluetooth device has been removed. 
     *
     * @param address the address of the bluetooth remote device
     */
    //void bondingRemoved(const QString &address);

    /**
     * This signal is emitted when a remote device is created.
     *
     * @param ubi the object path of the device on the system bus
     */
    void deviceCreated(const QString &ubi);

    /**
     * This signal is emitted when an inquiry session for a periodic discovery finishes and previously found
     * devices are no longer in range or visible.
     *
     * @param address the address of the remote device
     */
    void deviceDisappeared(const QString &address);

    /**
     * This signal will be emitted every time an inquiry result has been found by the service daemon.
     * In general they only appear during a device discovery.
     *
     * @param address the address of the remote device
     * @param properties the properties of the remote device
     */
    void deviceFound(const QString &address, const QMap< QString,QVariant > &properties);

    /**
     * This signal is emitted when a remote device is removed from the system bus.
     *
     * @param ubi the object path on the system bus from the remote device
     */
    void deviceRemoved(const QString &ubi);

    /**
     * This signal is emitted when a property of the adapter is set to a new value.
     *
     * @param property the named property of the adapter
     * @value the new value for the property
     */
    void propertyChanged(const QString &property, const QVariant &value);


};
} //Ifaces
} //Control
} //Solid

Q_DECLARE_INTERFACE(Solid::Control::Ifaces::BluetoothInterface, "org.kde.Solid.Control.Ifaces.BluetoothInterface/0.1")

#endif
