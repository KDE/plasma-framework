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

#ifndef SOLID_IFACES_BLUETOOTHREMOTEDEVICE
#define SOLID_IFACES_BLUETOOTHREMOTEDEVICE

#include <QtCore/QObject>
#include <QtCore/QStringList>
#include "../solid_control_export.h"

class KJob;

namespace Solid
{
namespace Control
{
namespace Ifaces
{
/**
 * This interface represents a remote bluetooth device which we may be connected to.
 */
class SOLIDCONTROLIFACES_EXPORT BluetoothRemoteDevice : public QObject
{
    Q_OBJECT
public:
    /**
     * Create a BluetoothRemoteDevice.
     *
     * @param parent the parent object
     */
    BluetoothRemoteDevice(QObject *parent = 0);

    /**
     * Destructs a BluetoothRemoteDevice object.
     */
    virtual ~BluetoothRemoteDevice();
    /**
     * Retrieve all properties from the remote device.
     *
     * @returns a hash of named properties
     */
    virtual QMap<QString,QVariant> getProperties() const = 0;
    /**
     * Discover all available Services from the remote Device.
     * When the call is finished serviceDiscoverAvailable is thrown.
     *
     * @param pattern a service handle pattern to search for
     */
    virtual void discoverServices(const QString &pattern) const = 0;
    /**
     * List all defined Nodes.
     *
     * @returns a List of ObjectPaths from all defined Nodes
     */
    virtual QStringList listNodes() const = 0;
    /**
     * The UBI of the remote device.
     */
    virtual QString ubi() const = 0;


public Q_SLOTS:
    /**
     * Set a new Value for a named property.
     *
     * @param name the name of the property
     * @param value the new value to be set
     */
    virtual void setProperty(const QString &name, const QVariant &value) = 0;
    /**
     * Cancel a started service Discovery.
     */
    virtual void cancelDiscovery() = 0;
    /**
     * Request a disconnect from the remote device.
     */
    virtual void disconnect() = 0;



Q_SIGNALS:
    /**
     * Search for services is done.
     *
     * @param status the result of the discovering.
     * @param services the discovered Services.
     */
    virtual void serviceDiscoverAvailable(const QString &status, const QMap<uint,QString> &services) = 0;
    /**
     * A Property for the remote device has changed.
     *
     * @param name the name of the changed property
     * @param value the new value
     */
    virtual void propertyChanged(const QString &name, const QVariant &value) = 0;
    /**
     * Disconnect to the remote device requested.
     */
    virtual void disconnectRequested() = 0;



};

} // Ifaces

} // Control

} // Solid

Q_DECLARE_INTERFACE(Solid::Control::Ifaces::BluetoothRemoteDevice, "org.kde.Solid.Control.Ifaces.BluetoothRemoteDevice/0.1")

#endif
