/*  This file is part of the KDE project
    Copyright (C) 2006,2007,2008 Will Stephenson <wstephenson@kde.org>

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

#ifndef SOLID_IFACES_WIRELESSNETWORKINTERFACE_H
#define SOLID_IFACES_WIRELESSNETWORKINTERFACE_H

#include "../solid_control_export.h"

#include <QtCore/QStringList>

#include "../wirelessnetworkinterface.h"
#include "networkinterface.h"


typedef QString MacAddress;
typedef QStringList MacAddressList;

namespace Solid
{
namespace Control
{
namespace Ifaces
{
    /**
     * Represents a wireless network interface
     */
    class SOLIDCONTROLIFACES_EXPORT WirelessNetworkInterface : virtual public NetworkInterface
    {
    public:
        /**
         * Destroys a WirelessNetworkInterface object
         */
        virtual ~WirelessNetworkInterface();
        /**
         * List of wireless networks currently visible to the hardware
         */
        virtual MacAddressList accessPoints() const = 0;
        /**
         * Identifier of the network this interface is currently associated with
         */
        virtual QString activeAccessPoint() const = 0;
        /**
         * The hardware address assigned to the network interface
         */
        virtual QString hardwareAddress() const = 0;
        /**
         * Retrieves the operation mode of this network.
         *
         * @return the current mode
         * @see Solid::Control::WirelessNetworkInterface::OperationMode
         */
        virtual Solid::Control::WirelessNetworkInterface::OperationMode mode() const = 0;

        /**
         * Retrieves the effective bit rate currently attainable by this device.
         *
         * @return the bitrate in bit/s
         */
        virtual int bitRate() const = 0;

        /**
         * Retrieves the capabilities of this wifi network.
         *
         * @return the flag set describing the capabilities
         * @see Solid::Control::WirelessNetworkInterface::DeviceInterface
         */
        virtual Solid::Control::WirelessNetworkInterface::Capabilities wirelessCapabilities() const = 0;
        /**
         * Instantiates a new AccessPoint object from the current backend given its UNI.
         *
         * @param uni the identifier of the network instantiated
         * @returns a new AccessPoint object if there's a network having the given UNI for this device, 0 otherwise
         */
         virtual QObject * createAccessPoint(const QString & uni) = 0;
    protected:
    //Q_SIGNALS:
        /**
         * This signal is emitted when the bitrate of this network has changed.
         *
         * @param bitrate the new bitrate value for this network
         */
        virtual void bitRateChanged(int bitrate) = 0;
        /**
         * The accesspoint in use changed.
         */
        virtual void activeAccessPointChanged(const QString &) = 0;
        /**
         * The device switched operating mode.
         */
        virtual void modeChanged(Solid::Control::WirelessNetworkInterface::OperationMode) = 0;
        /**
         * A new wireless access point appeared
         */
        virtual void accessPointAppeared(const QString &) = 0;
        /**
         * A wireless access point disappeared
         */
        virtual void accessPointDisappeared(const QString &) = 0;
    };
} //Ifaces
} //Control
} //Solid

Q_DECLARE_INTERFACE(Solid::Control::Ifaces::WirelessNetworkInterface, "org.kde.Solid.Control.Ifaces.WirelessNetworkInterface/0.1")

#endif //SOLID_IFACES_WIRELESSNETWORKINTERFACE_H

