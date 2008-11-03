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

#ifndef SOLID_IFACES_BLUETOOTHINPUTDEVICE
#define SOLID_IFACES_BLUETOOTHINPUTDEVICE

#include <QtCore/QObject>
#include <QtCore/QMap>
#include <QtCore/QVariant>

#include "../solid_control_export.h"

namespace Solid
{
namespace Control
{
namespace Ifaces
{
/**
 * A BluetoothInputDevice object allows to manage the connection of a bluetooth input device.
 */
class SOLIDCONTROLIFACES_EXPORT BluetoothInputDevice : public QObject
{
    Q_OBJECT
public:
    /**
     * Constructs a BluetoothInputDevice.
     *
     * @param parent the parent object
     */
    BluetoothInputDevice(QObject * parent = 0);

    /**
     * Destructs a BluetoothInputDevice object.
     */
    virtual ~BluetoothInputDevice();

    /**
     * Retrieves ubi of bluetooth input device.
     *
     * @returns ubi of bluetooth input device
     */
    virtual QString ubi() const = 0;
    /**
     * Retrieve all properties from the input device.
     *
     * @returns a hash of named properties
     */
    virtual QMap<QString,QVariant> getProperties() const = 0;


public Q_SLOTS:
    /**
     * Connect bluetooth input device.
     */
    virtual void connect() = 0;

    /**
     * Disconnect bluetooth input device.
     */
    virtual void disconnect() = 0;

Q_SIGNALS:
     /**
     * A Property for the input device has changed.
     *
     * @param name the name of the changed property
     * @param value the new value
     */
    virtual void propertyChanged(const QString &name, const QVariant &value) = 0;

};

} // Ifaces

} // Control

} // Solid

Q_DECLARE_INTERFACE(Solid::Control::Ifaces::BluetoothInputDevice, "org.kde.Solid.Control.Ifaces.BluetoothInputDevice/0.1")

#endif
