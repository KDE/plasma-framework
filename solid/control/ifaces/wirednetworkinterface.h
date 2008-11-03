/*  This file is part of the KDE project
    Copyright (C) 2006,2007 Will Stephenson <wstephenson@kde.org>

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

#ifndef SOLID_IFACES_WIREDNETWORKINTERFACE_H
#define SOLID_IFACES_WIREDNETWORKINTERFACE_H

#include "../solid_control_export.h"

#include <QtCore/QStringList>

#include "networkinterface.h"


namespace Solid
{
namespace Control
{
namespace Ifaces
{
    /**
     * Represents a wired network interface
     */
    class SOLIDCONTROLIFACES_EXPORT WiredNetworkInterface : virtual public NetworkInterface
    {
    public:
        /**
         * Destroys a WiredNetworkInterface object
         */
        virtual ~WiredNetworkInterface();
        /**
         * The hardware address assigned to the network interface
         */
        virtual QString hardwareAddress() const = 0;
        /**
         * Retrieves the effective bit rate currently attainable by this device.
         *
         * @return the bitrate in bit/s
         */
        virtual int bitRate() const = 0;

        /**
         * Indicates if the network interfaces sees a carrier.
         *
         * @return true if there's a carrier, false otherwise
         */
        virtual bool carrier() const = 0;

    protected:
    //Q_SIGNALS:
        /**
         * This signal is emitted when the bitrate of this network has changed.
         *
         * @param bitrate the new bitrate value for this network
         */
        virtual void bitRateChanged(int bitRate) = 0;
        /**
         * This signal indicates if the physical carrier changed state (eg if the network cable was
         * plugged or unplugged)
         */
        virtual void carrierChanged(bool plugged) = 0;
    };
} //Ifaces
} //Control
} //Solid

Q_DECLARE_INTERFACE(Solid::Control::Ifaces::WiredNetworkInterface, "org.kde.Solid.Control.Ifaces.WiredNetworkInterface/0.1")

#endif // SOLID_IFACES_WIRELESSNETWORKINTERFACE_H

