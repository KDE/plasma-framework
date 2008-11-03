/*  This file is part of the KDE project
    Copyright (C) 2007 Will Stephenson <wstephenson@kde.org>
    Copyright (C) 2007 Daniel Gollub <dgollub@suse.de>
    Copyright (C) 2007 Juan González <jaguilera@opsiland.info>

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

#ifndef SOLID_IFACES_BLUETOOTHSECURITY_H
#define SOLID_IFACES_BLUETOOTHSECURITY_H

#include "../solid_control_export.h"
#include "../bluetoothsecurity.h" // So we know about Solid::Control::BluetoothPassKeyAgent

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
class SOLIDCONTROLIFACES_EXPORT BluetoothSecurity : public QObject
{
    Q_OBJECT
public:
    /**
     * Create a BluetoothSecurity object for all Bluetooth Interfaces.
     *
     * @param parent the parent object
     */
    BluetoothSecurity(QObject *parent = 0);

    /**
     * Create a BluetoothSecurity object for a certain Bluetooth Interfaces.
     *
     * @param interface the ubi of a certain Bluetooth Interface
     * @param parent the parent object
     */
    BluetoothSecurity(const QString &interface, QObject *parent = 0);

    /**
     * Destructs a BluetoothSecurity object.
     */
    virtual ~BluetoothSecurity();
public Q_SLOTS:
    virtual void setPasskeyAgent(Solid::Control::BluetoothPasskeyAgent *agent) = 0;
    virtual void setAuthorizationAgent(Solid::Control::BluetoothAuthorizationAgent *agent) = 0;
};
} //Ifaces
} //Control
} //Solid

Q_DECLARE_INTERFACE(Solid::Control::Ifaces::BluetoothSecurity, "org.kde.Solid.Ifaces.BluetoothSecurity/0.1")

#endif

