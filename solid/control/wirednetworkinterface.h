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

#ifndef SOLID_CONTROL_WIREDNETWORKINTERFACE_H
#define SOLID_CONTROL_WIREDNETWORKINTERFACE_H

#include "solid_control_export.h"

#include "networkinterface.h"

namespace Solid
{
namespace Control
{
    class WiredNetworkInterfacePrivate;

    /**
     * This interface represents a wired Ethernet network interface
     */
    class SOLIDCONTROL_EXPORT WiredNetworkInterface : public NetworkInterface
    {
        Q_OBJECT
        Q_DECLARE_PRIVATE(WiredNetworkInterface)

    public:
        /**
         * Creates a new WiredNetworkInterface object.
         *
         * @param backendObject the network object provided by the backend
         */
        WiredNetworkInterface(QObject *backendObject = 0);

        /**
         * Constructs a copy of a network.
         *
         * @param network the network to copy
         */
        WiredNetworkInterface(const WiredNetworkInterface &network);

        /**
         * Destroys a WiredNetworkInterface object.
         */
        virtual ~WiredNetworkInterface();

        /**
         * The NetworkInterface type.
         *
         * @return the NetworkInterface::Type.  This always returns NetworkInterface::Ieee8023
         */
        virtual NetworkInterface::Type type() const;

        /**
         * The hardware address assigned to the network interface
         */
        QString hardwareAddress() const;

        /**
         * Retrieves the effective bit rate currently attainable by this device.
         *
         * @return the bitrate in bit/s
         */
        int bitRate() const;

        /**
         * Indicates if the network interfaces sees a carrier.
         *
         * @return true if there's a carrier, false otherwise
         */
        bool carrier() const;

    Q_SIGNALS:
        /**
         * This signal is emitted when the bitrate of this network has changed.
         *
         * @param bitrate the new bitrate value for this network
         */
        void bitRateChanged(int bitRate);

        /**
         * This signal indicates if the physical carrier changed state (eg if the network cable was
         * plugged or unplugged)
         */
        void carrierChanged(bool plugged);
    protected:
        /**
         * @internal
         */
        WiredNetworkInterface(WiredNetworkInterfacePrivate &dd, QObject *backendObject);

        /**
         * @internal
         */
        WiredNetworkInterface(WiredNetworkInterfacePrivate &dd, const WiredNetworkInterface &network);

    private Q_SLOTS:
        void _k_destroyed(QObject *object);
    private:
        friend class NetworkInterface;
        friend class NetworkInterfacePrivate;
    };
} //Control
} //Solid

#endif //SOLID_CONTROL_WIREDNETWORKINTERFACE_H

