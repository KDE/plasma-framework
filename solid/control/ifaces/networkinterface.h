/*  This file is part of the KDE project
    Copyright (C) 2006 Will Stephenson <wstephenson@kde.org>

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

#ifndef SOLID_CONTROL_IFACES_NETWORKINTERFACE_H
#define SOLID_CONTROL_IFACES_NETWORKINTERFACE_H

#include "../solid_control_export.h"
#include "../networkinterface.h"
#include <QtCore/QObject>
#include <QtCore/QList>

namespace Solid
{
namespace Control
{
namespace Ifaces
{
    /**
     * Represents a generic network interface as seen by the networking subsystem.
     *
     * For specialized interfaces @see Solid::Control::Ifaces::WiredNetworkInterface and @see
     * Solid::Control::Ifaces::WirelessNetworkInterface
     */
    class SOLIDCONTROLIFACES_EXPORT NetworkInterface
    {
    public:
        /**
         * Destroys a NetworkInterface object.
         */
        virtual ~NetworkInterface();

        /**
         * Retrieves the Unique Network Identifier (UNI) of the Network.
         * This identifier is unique for each network and network interface in the system.
         *
         * @returns the Unique Network Identifier of the current network
         */
        virtual QString uni() const = 0;
        /**
         * The system name for the network interface
         */
        virtual QString interfaceName() const = 0;

        /**
         * Handle for the system driver controlling this network interface
         */
        virtual QString driver() const = 0;

        /**
         * Access the network configuration for this object
         */
        virtual Solid::Control::IPv4Config ipV4Config() const = 0;

        /**
         * Retrieves the activation status of this network interface.
         *
         * @return true if this network interface is active, false otherwise
         */
        virtual bool isActive() const = 0;
        /**
         * Retrieves the current state of the network connection held by this device.
         * It's a high level view of the connection. It's user oriented so technically
         * it provides states coming from different layers.
         *
         * @return the current connection state
         * @see Solid::Control::NetworkInterface::ConnectionState
         */
        virtual Solid::Control::NetworkInterface::ConnectionState connectionState() const = 0;
        /**
         * Retrieves the maximum speed as reported by the device. Note that it's a design
         * related information and that the device might not reach this maximum.
         *
         * @return the device maximum speed
         */
        virtual int designSpeed() const = 0;

        /**
         * Retrieves the capabilities supported by this device.
         *
         * @return the capabilities of the device
         * @see Solid::Control::NetworkInterface::Capabilities
         */
        virtual Solid::Control::NetworkInterface::Capabilities capabilities() const = 0;

    protected:
    //Q_SIGNALS:
        /**
         * This signal is emitted when the settings of this network have changed.
         */
        virtual void ipDetailsChanged() = 0;

        /**
         * This signal is emitted when the device's connection state changed.
         * For example, if the device was disconnected and started to activate
         *
         * @param state the new state of the connection
         * @see Solid::Control::NetworkInterface::ConnectionState
         */
        virtual void connectionStateChanged(int state) = 0;
    };
} //Ifaces
} //Control
} //Solid

Q_DECLARE_INTERFACE(Solid::Control::Ifaces::NetworkInterface, "org.kde.Solid.Control.Ifaces.NetworkInterface/0.1")

#endif
