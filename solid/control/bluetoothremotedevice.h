/*  This file is part of the KDE project
    Copyright (C) 2007 Will Stephenson <wstephenson@kde.org>
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

#ifndef SOLID_CONTROL_BLUETOOTHREMOTEDEVICE_H
#define SOLID_CONTROL_BLUETOOTHREMOTEDEVICE_H

#include <QtCore/QObject>
#include <QtCore/QStringList>
#include <QtCore/QMap>
#include <QtCore/QVariant>

#include "bluetoothmanager.h"
#include "bluetoothinterface.h"

class KJob;

namespace Solid
{
namespace Control
{
class BluetoothRemoteDevicePrivate;
/**
 * Represents a bluetooth remote device as seen by the bluetoothing subsystem.
 */
class SOLIDCONTROL_EXPORT BluetoothRemoteDevice : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(BluetoothRemoteDevice)

public:
    /**
     * Creates a new BluetoothRemoteDevice object.
     *
     * @param backendObject the bluetooth remote device object provided by the backend
     */
    BluetoothRemoteDevice(QObject *backendObject = 0);

    /**
     * Constructs a copy of a bluetooth remote device.
     *
     * @param device the bluetooth remote device to copy
     */
    BluetoothRemoteDevice(const BluetoothRemoteDevice &device);

    /**
     * Destroys the device.
     */
    ~BluetoothRemoteDevice();

    /**
     * Assigns a bluetooth remote device to this bluetooth remote device and returns a reference to it.
     *
     * @param device the bluetooth remote device to assign
     * @return a reference to the bluetooth remote device
     */
    BluetoothRemoteDevice &operator=(const BluetoothRemoteDevice &device);

    /**
     * Checks for equality.
     * @param other the bluetooth remote device to compare with this
     * @return true if this->ubi() equals other.ubi(), false otherwise
     */
    bool operator==(const BluetoothRemoteDevice  & other) const;

    /**
     * The UBI of the remote de device.
     */
    QString ubi() const;

    /**
     * Retrieve all properties from the remote device.
     *
     * @returns a hash of named properties
     */
    QMap<QString,QVariant> getProperties();

    /**
     * Discover all available Services from the remote Device.
     * When the call is finished serviceDiscoverAvailable is thrown.
     *
     * @param pattern a service handle pattern to search for
     */
    void discoverServices(const QString &pattern);

    /**
     * List all defined Nodes.
     *
     * @returns a List of ObjectPaths from all defined Nodes
     */
    QStringList listNodes();

public Q_SLOTS:

    /**
     * Set a new Value for a named property.
     *
     * @param name the name of the property
     * @param value the new value to be set
     */
    void setProperty(const QString &name, const QVariant &value);

    /**
     * Cancel a started service Discovery.
     */
    void cancelDiscovery();

    /**
     * Request a disconnect from the remote device.
     */
    void disconnect();

Q_SIGNALS:

    /**
     * Search for services is done.
     *
     * @param status the result of the discovering.
     * @param services the discovered Services.
     */
    void serviceDiscoverAvailable(const QString &status, const QMap<uint,QString> &services);

    /**
     * A Property for the remote device has changed.
     *
     * @param name the name of the changed property
     * @param value the new value
     */
    void propertyChanged(const QString &name, const QVariant &value);

    /**
     * Disconnect to the remote device requested.
     */
    void disconnectRequested();


protected:
    BluetoothRemoteDevicePrivate *d_ptr;

private:
    Q_PRIVATE_SLOT(d_func(), void _k_destroyed(QObject *))
};

} //Control
} //Solid

#endif
