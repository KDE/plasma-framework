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

#ifndef SOLID_CONTROL_BLUETOOTHINTERFACE_H
#define SOLID_CONTROL_BLUETOOTHINTERFACE_H

#include <QtCore/QDateTime>
#include <QtCore/QPair>
#include <QtCore/QObject>
#include <QtCore/QMap>

#include "bluetoothremotedevice.h"
#include "ifaces/bluetoothremotedevice.h"



namespace Solid
{
namespace Control
{
class BluetoothRemoteDevice;
typedef QList<BluetoothRemoteDevice> BluetoothRemoteDeviceList;
class BluetoothInterfacePrivate;

/**
 * Represents a bluetooth interface as seen by the bluetooth subsystem.
 */
class SOLIDCONTROL_EXPORT BluetoothInterface : public QObject
{
    Q_OBJECT

public:
    /**
     * Describes the operating mode of a bluetooth interface
     *
     * - Off : The interface's transceiver is turned off
     * - Discoverable : The interface may be discovered by other devices and connected to
     * - Connectable : The interface may only be connected to but not discovered
     */
    //enum Mode { Off, Discoverable, Connectable };
    /**
     * Constructs an invalid bluetooth interface
     */
    BluetoothInterface();

    /**
     * Constructs a bluetooth interface for a given Unique Bluetooth Identifier (UBI).
     * This object will be auto deleted, DON'T delete it.
     * @param ubi the ubi of the bluetooth interface to create
     */
    explicit BluetoothInterface(const QString &ubi);

    /**
     * Constructs a new bluetooth interface taking its data from a backend.
     *
     * @param backendObject the object given by the backend
     */
    explicit BluetoothInterface(QObject *backendObject);

    /**
     * Constructs a copy of a bluetooth interface.
     *
     * @param device the bluetooth interface to copy
     */
    BluetoothInterface(const BluetoothInterface &device);

    /**
     * Destroys the device.
     */
    ~BluetoothInterface();


    /**
     * Assigns a bluetooth interface to this bluetooth interface and returns a reference to it.
     *
     * @param device the bluetooth interface to assign
     * @return a reference to the bluetooth interface
     */
    BluetoothInterface &operator=(const BluetoothInterface &device);

    /**
     * Retrieves the Unique Bluetooth Identifier (UBI) of the BluetoothInterface.
     * This identifier is ubique for each bluetooth and bluetooth interface in the system.
     *
     * @returns the Unique Bluetooth Identifier of the current bluetooth interface
     */
    QString ubi() const;

    /**
     * Create new BluetoothRemoteDevice object from this interface given its address.
     *
     * @param ubi the identifier of the bluetooth device to instantiate
     * @returns a bluetooth object, if a bluetooth device having the given UBI, for this interface exists, 0 otherwise
     */
    Solid::Control::BluetoothRemoteDevice* createBluetoothRemoteDevice(const QString &address);

    /**
     * Finds a BluetoothRemoteDevice object given its UBI.
     *
     * @param ubi the identifier of the bluetooth remote device to find from this bluetooth interface
     * @returns a valid BluetoothRemoteDevice object if a remote device having the given UBI for this interface exists, an invalid BluetoothRemoteDevice object otherwise.
     */
    Solid::Control::BluetoothRemoteDevice findBluetoothRemoteDevice(const QString &address) const;

    /**
     * Retrieves the MAC address of the bluetooth interface/adapter.
     *
     * @returns MAC address of bluetooth interface
     */
    //QString address() const;

    /**
     * Retrieves the version of the chip of the bluetooth interface/adapter.
     * Example: "Bluetooth 2.0 + EDR"
     *
     * @returns bluetooth chip version
     */
    //QString version() const;

    /**
     * Retrieves the revision of the chip of the bluetooth interface/adapter.
     * Example: "HCI 19.2"
     *
     * @returns bluetooth chip revision
     */
    //QString revision() const;

    /**
     * Retrieves the name of the bluetooth chip manufacturer.
     * Example: "Boston Silicon Radio"
     *
     * @returns manufacturer string of bluetooth interface/adapter
     */
    //QString manufacturer() const;

    /**
     * Retrieves the name of the manufacturer of the bluetooth interface,
     * using the chip supplied by BluetoothInterface::manufacterer()
     * Based on device address.
     *
     * @returns company string of bluetooth interface/adapter
     */
    //QString company() const;

    /**
     * Retrieves the current mode of the bluetooth interface/adapter.
     *
     * @returns the current mode of bluetooth interface/adapter
     */
    //Mode mode() const;

    /**
     * Retrieves the discoverable timeout of the bluetooth interface/adapter.
     * Discoverable timeout of 0 means never disappear.
     *
     * @returns current discoverable timeout in seconds
     */
    //int discoverableTimeout() const;

    /**
     * Retrieves the current discoverable staut of the bluetooth interface/adapter.
     *
     * @returns current discoverable status of bluetooth interface/adapter
     */
    //bool isDiscoverable() const;

    /**
     * List all UBIs of connected remote bluetooth devices of this handled bluetooth
     * interface/adapter.
     *
     * @returns list UBIs of connected bluetooth remote devices
     */
    //BluetoothRemoteDeviceList listConnections() const;

    /**
     * Retrieves major class of the bluetooth interface/adapter.
     * @todo enum
     * @returns current major class of the bluetooth interface/adapter
     */
    //QString majorClass() const;

    /**
     * List supported minor classes of the bluetooth interface/adapter.
     * @todo enum
     *
     * @returns list of supported minor classes by bluetooth interface/adapter
     */
    //QStringList listAvailableMinorClasses() const;

    /**
     * Retrieves minor class of the bluetooth interface/adapter.
     * Valid classes, see listAvailableMinorClasses()
     * @todo enum
     *
     * @returns minor class of the bluetooth interface/adapter.
     */
    //QString minorClass() const;

    /**
     * List services class of the bluetooth interface/adapter.
     * @todo enum
     *
     * @returns list of service classes or empty list if no services registered
     */
    //QStringList serviceClasses() const;

    /**
     * Retrieves name of bluetooth interface/adapter.
     * @todo enum
     *
     * @returns name of bluetooth interface/adapter
     */
    //QString name() const;

    /**
     * Returns the name of the remote device, given its mac address (mac).
     *
     * @return the name of the remote device
     */
    //QString getRemoteName(const QString & mac);

    /**
     * List UBIs of bonded/paired remote bluetooth devices with this bluetooth
     * interface/adapter.
     *
     * @returns UBIs of bonded/paired bluetooth remote devices
     */
    //QStringList listBondings() const;

    /**
     * Periodic discovery status of this bluetooth interface/adapter.
     *
     * @returns true if periodic discovery is already active otherwise false
     */
   // bool isPeriodicDiscoveryActive() const;

    /**
     * Name resolving status of periodic discovery routing.
     *
     * @returns true if name got resolved while periodic discovery of this bluetooth
     * interface/adapter
     */
    //bool isPeriodicDiscoveryNameResolvingActive() const;

    /**
     * List the Unique Bluetooth Identifier (UBI) of all known remote devices, 
     * whether they are seen, used or paired/bonded.
     *
     * See listConnections()
     *
     * @returns a QStringList of UBIs of all known remote bluetooth devices
     */
    //QStringList listRemoteDevices() const;

    /**
     * List the Unique Bluetooth Identifier (UBI) of all known remote devices since a specific
     * datestamp. Known remote devices means remote bluetooth which are seen, used or
     * paired/bonded.
     *
     * See listConnections(), listRemoteDevices()
     *
     * @param date the datestamp of the beginning of recent used devices
     * @returns a QStringList of UBIs of all known remote bluetooth devices
     */
  //  QStringList listRecentRemoteDevices(const QDateTime &date) const;

    /**
     * Returns true if the remote bluetooth device is trusted otherwise false.
     *
     * @param mac the address of the remote device
     */
//    bool isTrusted(const QString &);


    QString createPairedDevice(const QString &, const QString &, const QString &) const;

    QMap< QString, QVariant > getProperties() const;

    BluetoothRemoteDeviceList listDevices() const;




public Q_SLOTS:
    /**
     * Set mode of bluetooth interface/adapter.
     * Valid modes, see mode()
     *
     * @param mode the mode of the bluetooth interface/adapter
     */
    //void setMode(const Mode mode);

    /**
     * Set discoverable timeout of bluetooth interface/adapter.
     *
     * @param timeout timeout in seconds
     */
    //void setDiscoverableTimeout(int timeout);

    /**
     * Set minor class of bluetooth interface/adapter.
     *
     * @param minor set minor class. Valid mode see listAvaliableMinorClasses()
     */
    //void setMinorClass(const QString &minor);

    /**
     * Set name of bluetooth interface/adapter.
     *
     * @param name the name of bluetooth interface/adapter
     */
    //void setName(const QString &name);

    /**
     * Start discovery of remote bluetooth devices with device name resolving.
     */
    //void discoverDevices();
    /**
     * Start discovery of remote bluetooth devices without device name resolving.
     */
    //void discoverDevicesWithoutNameResolving();
    /**
     * Cancel discovery of remote bluetooth devices.
     */
    //void cancelDiscovery();

    /**
     * Start periodic discovery of remote bluetooth devices.
     * See stopPeriodicDiscovery()
     */
    //void startPeriodicDiscovery();

    /**
     * Stop periodic discovery of remote bluetooth devices.
     */
    //void stopPeriodicDiscovery();

    /**
     * Enable/Disable name resolving of remote bluetooth devices in periodic discovery.
     *
     * @param resolveName true to enable name resolving otherwise false
     */
    //void setPeriodicDiscoveryNameResolving(bool resolveNames);

    /**
     * Marks the device as trusted.
     *
     * @param mac the address of the remote device
     */
    //void setTrusted(const QString &);

    /**
     * Marks the device as not trusted.
     *
     * @param mac the address of the remote device
     */
    //void removeTrust(const QString &);

    void registerAgent(const QString &,const QString &) const;

    void releaseSession() const;

    void removeDevice(const QString &) const;

    void requestSession() const;

    void setProperty(const QString &, const QVariant &) const;

    void startDiscovery() const;

    void stopDiscovery() const;

    void unregisterAgent(const QString &) const;

    void cancelDeviceCreation(const QString &) const;



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
     * @param deviceClass the device class of the remote device
     * @param rssi the Received Signal Strength Information (RSSI) of the remote device
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
    void deviceFound(const QString &address, const QMap< QString, QVariant > &properties);

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


private:
    Q_PRIVATE_SLOT(d, void _k_destroyed(QObject *))

    BluetoothInterfacePrivate * const d;
};

} //Control
} //Solid

#endif

