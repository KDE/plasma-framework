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

#ifndef SOLID_IFACES_NETWORKMANAGER
#define SOLID_IFACES_NETWORKMANAGER

#include <QtCore/QObject>
#include <QtCore/QStringList>
#include <solid/networking.h>
#include "../solid_control_export.h"
#include "../networkmanager.h"

namespace Solid
{
namespace Control
{
namespace Ifaces
{
    /**
     * This class specifies the interface a backend will have to implement in
     * order to be used in the system.
     *
     * A network manager allow to query the underlying platform to discover the
     * available network interfaces and reachable network. It has also the
     * responsibility to notify when a network interface or a network appear or disappear.
     */
    class SOLIDCONTROLIFACES_EXPORT NetworkManager : public QObject
    {
        Q_OBJECT
    public:
        /**
         * Constructs a NetworkManager.
         *
         * @param parent the parent object
         */
        NetworkManager(QObject * parent = 0);
        /**
         * Destructs a NetworkManager object.
         */
        virtual ~NetworkManager();

        /**
         * Get the manager connection state
         */
        virtual Solid::Networking::Status status() const = 0;

        /**
         * Retrieves the list of all the network interfaces Unique Network Identifiers (UNIs)
         * in the system. It includes both hardware and virtual devices.
         *
         * @return the list of network interfaces available in this system
         */
        virtual QStringList networkInterfaces() const = 0;

        /**
         * Instantiates a new NetworkInterface object from this backend given its UNI.
         *
         * @param uni the identifier of the network interface instantiated
         * @returns a new NetworkInterface object if there's a device having the given UNI, 0 otherwise
         */
        virtual QObject *createNetworkInterface(const QString &uni) = 0;

        /**
         * Retrieves the activation status of networking (as a whole) in the system.
         *
         * @return true if this networking is enabled, false otherwise
         */
        virtual bool isNetworkingEnabled() const = 0;

        /**
         * Retrieves the activation status of wireless networking in the system.
         *
         * @return true if this wireless networking is enabled, false otherwise
         */
        virtual bool isWirelessEnabled() const = 0;

        /**
         * Retrieves the status of wireless hardware in the system.  This is typically
         * controlled by a physical switch so there is no way to set this in software.
         *
         * @since KDE 4.1
         * @return true if this wireless networking is enabled, false otherwise
         */
        virtual bool isWirelessHardwareEnabled() const = 0;

        virtual void activateConnection(const QString & interfaceUni, const QString & connectionUni, const QVariantMap & connectionParameters ) = 0;

        virtual void deactivateConnection(const QString & activeConnection) = 0;

        /**
         * Access the list of active connection UNIs
         */
        virtual QStringList activeConnections() const = 0;

    public Q_SLOTS:
        /**
         * Activates or deactivates networking (as a whole).
         *
         * @param enabled true to activate networking, false otherwise
         */
        virtual void setNetworkingEnabled(bool enabled) = 0;

        /**
         * Activates or deactivates wireless networking.
         *
         * @param enabled true to activate wireless networking, false otherwise
         */
        virtual void setWirelessEnabled(bool enabled) = 0;

    Q_SIGNALS:
        /**
         * This signal is emitted when the system's connection state changes
         */
        void statusChanged(Solid::Networking::Status status);

        /**
         * This signal is emitted when a new network interface is available.
         *
         * @param uni the network interface identifier
         */
        void networkInterfaceAdded(const QString  & uni);

        /**
         * This signal is emitted when a network interface is not available anymore.
         *
         * @param uni the network interface identifier
         */
        void networkInterfaceRemoved(const QString  & uni);

        /**
         * This signal is emitted when the status of the wireless changed
         */
        void wirelessEnabledChanged(bool enabled);

        /**
         * This signal is emitted when the status of the wireless hardware changed
         */
        void wirelessHardwareEnabledChanged(bool enabled);

        /**
         * This signal is emitted when the set of active connections changes
         */
        void activeConnectionsChanged();
    };

} // Ifaces

} // Control

} // Solid

Q_DECLARE_INTERFACE(Solid::Control::Ifaces::NetworkManager, "org.kde.Solid.Control.Ifaces.NetworkManager/0.1")

#endif
