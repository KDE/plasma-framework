/*  This file is part of the KDE project
    Copyright (C) 2006 Will Stephenson <wstephenson@kde.org>
    Copyright (C) 2006-2007 Kevin Ottens <ervin@kde.org>

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

#ifndef SOLID_CONTROL_NETWORKMANAGER
#define SOLID_CONTROL_NETWORKMANAGER

#include <QtCore/QObject>
#include <QtCore/QVariant>

#include <solid/networking.h>
#include "solid_control_export.h"

namespace Solid
{
namespace Control
{
    namespace Ifaces
    {
        class NetworkManager;
    }
    class Network;
    class NetworkInterface;
    typedef QList<NetworkInterface*> NetworkInterfaceList;
        
    /**
     * This class allow to query the underlying system to discover the available
     * network interfaces and reachable network.It has also the
     * responsibility to notify when a network interface or a network appear or disappear.
     *
     * It's the unique entry point for network management. Applications should use
     * it to find network interfaces, or to be notified about network related changes.
     *
     * Note that it's implemented as a singleton and encapsulates the backend logic.
     */
    namespace NetworkManager
    {
        /**
         * Get the manager connection state
         */
        SOLIDCONTROL_EXPORT Solid::Networking::Status status();
        /**
         * Retrieves the list of all the network interfaces in the system.
         * It includes both hardware and virtual devices.
         *
         * @return the list of network interfaces available in this system
         */
        SOLIDCONTROL_EXPORT NetworkInterfaceList networkInterfaces();

        /**
         * Find a new NetworkInterface object given its UNI.  This pointer is owned by the Solid
         * infrastructure.
         *
         * @param uni the identifier of the network interface to find
         * @returns a valid NetworkInterface object if there's a device having the given UNI, an invalid one otherwise
         */
        SOLIDCONTROL_EXPORT NetworkInterface * findNetworkInterface(const QString &uni);

        /**
         * Retrieves the status of networking (as a whole) in the system.
         * This is distinct from whether the system's networking is online or offline.
         * To check that, see @ref NetworkStatus.
         *
         * @return true if this networking is enabled, false otherwise
         */
        SOLIDCONTROL_EXPORT bool isNetworkingEnabled();

        /**
         * Retrieves the activation status of wireless networking in the system.
         *
         * @return true if this wireless networking is enabled, false otherwise
         */
        SOLIDCONTROL_EXPORT bool isWirelessEnabled();


        /**
         * Activates or deactivates networking (as a whole).
         *
         * @param enabled true to activate networking, false otherwise
         */
        SOLIDCONTROL_EXPORT void setNetworkingEnabled(bool enabled);

        /**
         * Retrieves the status of wireless hardware in the system.  This is typically
         * controlled by a physical switch so there is no way to set this in software.
         *
         * @since KDE 4.1
         * @return true if this wireless networking is enabled, false otherwise
         */
        SOLIDCONTROL_EXPORT bool isWirelessHardwareEnabled();

        /**
         * Activates or deactivates wireless networking.
         *
         * @param enabled true to activate wireless networking, false otherwise
         */
        SOLIDCONTROL_EXPORT void setWirelessEnabled(bool enabled);

        /**
         * @param deviceUni unique identifier of the network interface to be activated
         * @param connectionUni unique identifier for the connection to be activated
         * @param connectionParameters can be used to specify extra parameters not specific to the NetworkInterface or the connection, eg which AP to use when several present with same ESSID in range (because ESSID no guarantee that the AP is part of the network you want to join!)
         *
         */
        SOLIDCONTROL_EXPORT void activateConnection(const QString & deviceUni, const QString & connectionUni,
                const QVariantMap & connectionParameters );
        /**
         * Deactivate this network interface, if active
         * @param activeConnectionUni identifer of the connection to deactivate
         */
        SOLIDCONTROL_EXPORT void deactivateConnection(const QString & activeConnectionUni);

        /**
         * Access the list of any active connections
         */
        QStringList SOLIDCONTROL_EXPORT activeConnections();

        class SOLIDCONTROL_EXPORT Notifier : public QObject
        {
            Q_OBJECT
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
            void networkInterfaceAdded(const QString &uni);

            /**
             * This signal is emitted when a network interface is not available anymore.
             *
             * @param uni the network interface identifier
             */
            void networkInterfaceRemoved(const QString &uni);
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

        SOLIDCONTROL_EXPORT Notifier *notifier();
    }

} // Control
} // Solid

#endif
