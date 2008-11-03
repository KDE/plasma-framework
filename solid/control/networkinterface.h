/*  This file is part of the KDE project
    Copyright (C) 2006 Will Stephenson <wstephenson@kde.org>
    Copyright (C) 2007 Kevin Ottens <ervin@kde.org>

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

#ifndef SOLID_CONTROL_NETWORKINTERFACE_H
#define SOLID_CONTROL_NETWORKINTERFACE_H

#include <QtCore/QObject>
#include <QtCore/QStringList>
#include <QtNetwork/QNetworkAddressEntry>

#include "solid_control_export.h"

#include "networkipv4config.h"

namespace Solid
{
namespace Control
{
    class NetworkInterfacePrivate;

    /**
     * This interface represents a generic network interface in the system
     */
    class SOLIDCONTROL_EXPORT NetworkInterface : public QObject
    {
    Q_OBJECT
    Q_ENUMS(ConnectionState Capability Type)
    Q_FLAGS(Capabilities)
    Q_DECLARE_PRIVATE(NetworkInterface)

    public:
        // == NM ActivationStage
        /**
         * Device connection states describe the possible states of a
         * network connection from the user's point of view.  For
         * simplicity, states from several different layers are present -
         * this is a high level view
         */
        enum ConnectionState{ UnknownState, Unmanaged, Unavailable, Disconnected, Preparing,
                              Configuring, NeedAuth, IPConfig, Activated, Failed };
        /**
         * Possible Device capabilities
         * - IsManageable: denotes that the device can be controlled by this API
         * - SupportsCarrierDetect: the device informs us when it is plugged in to the medium
         * - SupportsWirelessScan: the device can scan for wireless networks
         */
        enum Capability { IsManageable = 0x1, SupportsCarrierDetect = 0x2 };
        /**
         * Device medium types
         * - Ieee8023: wired ethernet
         * - Ieee80211: the popular family of wireless networks
         * - Gsm: cellular device of the GSM family, used in Europe
         * - Cdma: cellular device of the CDMA family, used in the USA
         */
        enum Type { UnknownType, Ieee8023, Ieee80211, Serial, Gsm, Cdma };

        Q_DECLARE_FLAGS(Capabilities, Capability)

        /**
         * Creates a new NetworkInterface object.
         *
         * @param backendObject the network object provided by the backend
         */
        explicit NetworkInterface(QObject *backendObject);

        /**
         * Constructs a copy of a network.
         *
         * @param network the network to copy
         */
        NetworkInterface(const NetworkInterface &network);

        /**
         * Destroys a NetworkInterface object.
         */
        virtual ~NetworkInterface();

        /**
         * Retrieves the interface type.  This is a virtual function that will return the
         * proper type of all sub-classes.
         *
         * @returns the NetworkInterface::Type that corresponds to this device.
         */
        virtual Type type() const;

        /**
         * Retrieves the Unique Network Identifier (UNI) of the NetworkInterface.
         * This identifier is unique for each network and network interface in the system.
         *
         * @returns the Unique Network Identifier of the current network
         */
        QString uni() const;

        /**
         * The system name for the network interface
         */
        QString interfaceName() const;

        /**
         * Handle for the system driver controlling this network interface
         */
        QString driver() const;

        Solid::Control::IPv4Config ipV4Config() const;
        /**
         * Retrieves the activation status of this network interface.
         *
         * @return true if this network interface is active, false otherwise
         */
        bool isActive() const;

        /**
         * Retrieves the current state of the network connection held by this device.
         * It's a high level view of the connection. It is user oriented, so
         * actually it provides state coming from different layers.
         *
         * @return the current connection state
         * @see Solid::Control::NetworkInterface::ConnectionState
         */
        ConnectionState connectionState() const;

        /**
         * Retrieves the maximum speed as reported by the device.
         * Note that this is only a design related piece of information, and that
         * the device might not reach this maximum.
         *
         * @return the device's maximum speed
         */
        int designSpeed() const;

        /**
         * Retrieves the capabilities supported by this device.
         *
         * @return the capabilities of the device
         */
        Capabilities capabilities() const;

    Q_SIGNALS:
        /**
         * This signal is emitted when the settings of this network have changed.
         */
        void ipDetailsChanged();

        /**
         * This signal is emitted when the device's link status changed. For example, if there
         * is no carrier anymore.
         *
         * @param linkActivated true if the carrier got detected, false otherwise
         */
        void linkUpChanged(bool linkActivated);

        /**
         * This signal is emitted when the device's link status changed. For example, if there
         * is no carrier anymore.
         *
         * @param state the new state of the connection
         * @see Solid::Control::NetworkInterface::ConnectionState
         */
        void connectionStateChanged(int state);

    protected:
        /**
         * @internal
         */
        NetworkInterface(NetworkInterfacePrivate &dd, QObject *backendObject);

        /**
         * @internal
         */
        NetworkInterface(NetworkInterfacePrivate &dd, const NetworkInterface &network);

        NetworkInterfacePrivate *d_ptr;

    private:
    //HACK: to make NetworkList polymorphic (containing both wired and wireless networks, I used Network * here - Will.
    };
    typedef QList<NetworkInterface *> NetworkInterfaceList;


} //Control
} //Solid

Q_DECLARE_OPERATORS_FOR_FLAGS(Solid::Control::NetworkInterface::Capabilities)

#endif //SOLID_CONTROL_NETWORKINTERFACE_H

