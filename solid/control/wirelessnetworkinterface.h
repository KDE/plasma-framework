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

#ifndef SOLID_CONTROL_WIRELESSNETWORKINTERFACE_H
#define SOLID_CONTROL_WIRELESSNETWORKINTERFACE_H

#include "solid_control_export.h"

#include "networkinterface.h"

typedef QStringList AccessPointList;
namespace Solid
{
namespace Control
{
    class AccessPoint;
    class WirelessNetworkInterfacePrivate;
namespace Ifaces
{
    class AccessPoint;
}

    /**
     * This interface represents a wireless network interface
     */
    class SOLIDCONTROL_EXPORT WirelessNetworkInterface : public NetworkInterface
    {
        Q_OBJECT
        Q_ENUMS(OperationMode DeviceInterface)
        Q_FLAGS(Capabilities)
        Q_DECLARE_PRIVATE(WirelessNetworkInterface)

    public:
        enum OperationMode { Unassociated, Adhoc, Managed, Master, Repeater };
        // corresponding to 802.11 capabilities defined in NetworkManager.h
        enum Capability { NoCapability = 0x0, Wep40 = 0x2, Wep104, Tkip = 0x4, Ccmp = 0x8, Wpa = 0x10,
                          Rsn };
        Q_DECLARE_FLAGS(Capabilities, Capability)

        /**
         * Creates a new WirelessNetworkInterface object.
         *
         * @param backendObject the network object provided by the backend
         */
        WirelessNetworkInterface(QObject *backendObject = 0);

        /**
         * Constructs a copy of a network.
         *
         * @param network the network to copy
         */
        WirelessNetworkInterface(const WirelessNetworkInterface &network);

        /**
         * Destroys a WirelessNetworkInterface object.
         */
        virtual ~WirelessNetworkInterface();

        /**
         * The NetworkInterface type.
         *
         * @return the NetworkInterface::Type.  This always returns NetworkInterface::Ieee80211
         */
        virtual NetworkInterface::Type type() const;

        /**
         * List of wireless networks currently visible to the hardware
         */
        AccessPointList accessPoints() const;

        /**
         * Identifier of the network this interface is currently associated with
         */
        QString activeAccessPoint() const;

        /**
         * The hardware address assigned to the network interface
         */
        QString hardwareAddress() const;

        /**
         * Retrieves the operation mode of this network.
         *
         * @return the current mode
         * @see Solid::Control::WirelessNetworkInterface::OperationMode
         */
        Solid::Control::WirelessNetworkInterface::OperationMode mode() const;

        /**
         * Retrieves the effective bit rate currently attainable by this device.
         *
         * @return the bitrate in bit/s
         */
        int bitRate() const;

        /**
         * Retrieves the capabilities of this wifi network.
         *
         * @return the flag set describing the capabilities
         * @see Solid::Control::WirelessNetworkInterface::DeviceInterface
         */
        Solid::Control::WirelessNetworkInterface::Capabilities wirelessCapabilities() const;

        /**
         * Finds access point object given its Unique Network Identifier.
         *
         * @param uni the identifier of the AP to find from this network interface
         * @returns a valid AccessPoint object if a network having the given UNI for this device is known to the system, 0 otherwise
         */
        AccessPoint *findAccessPoint(const QString & uni) const;

    Q_SIGNALS:
        /**
         * This signal is emitted when the bitrate of this network has changed.
         *
         * @param bitrate the new bitrate value for this network
         */
        void bitRateChanged(int);
        /**
         * The active network changed.
         */
        void activeAccessPointChanged(const QString &);
        /**
         * The device switched operating mode.
         */
        void modeChanged(Solid::Control::WirelessNetworkInterface::OperationMode);
        /**
         * A new wireless access point appeared
         */
        void accessPointAppeared(const QString &);
        /**
         * A wireless access point disappeared
         */
        void accessPointDisappeared(const QString &);
    protected:
        /**
         * @internal
         */
        WirelessNetworkInterface(WirelessNetworkInterfacePrivate &dd, QObject *backendObject);

        /**
         * @internal
         */
        WirelessNetworkInterface(WirelessNetworkInterfacePrivate &dd, const WirelessNetworkInterface &network);

        void makeConnections(QObject * source);
        QPair<AccessPoint *, Ifaces::AccessPoint *> findRegisteredAccessPoint(const QString &uni) const;
    private Q_SLOTS:
        void _k_accessPointAdded(const QString &uni);
        void _k_accessPointRemoved(const QString &uni);
        void _k_destroyed(QObject *object);
    };
} //Control
} //Solid

#endif //SOLID_CONTROL_WIREDNETWORKINTERFACE_H

