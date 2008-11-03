/*
Copyright 2008 Will Stephenson <wstephenson@kde.org>

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) version 3, or any
later version accepted by the membership of KDE e.V. (or its
successor approved by the membership of KDE e.V.), which shall
act as a proxy defined in Section 6 of version 3 of the license.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public 
License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef SOLIDCONTROL_IFACES_WIRELESSAP_H
#define SOLIDCONTROL_IFACES_WIRELESSAP_H

#include "../solid_control_export.h"
#include "../wirelessaccesspoint.h"
#include "../wirelessnetworkinterface.h"

namespace Solid
{
namespace Control
{
namespace Ifaces
{
    /**
     * A wireless Access Point or another station acting in ad-hoc mode
     */
    class SOLIDCONTROLIFACES_EXPORT AccessPoint : public QObject
    {
    Q_OBJECT
    public:
        AccessPoint(QObject *);
        /**
         * Destructor
         */
        virtual ~AccessPoint();

        /**
         * Unique identifer for this access point
         */
        virtual QString uni() const = 0;

        /**
         * Flags describing the capabilities of the access point
         */
        virtual Solid::Control::AccessPoint::Capabilities capabilities() const = 0;

        /**
         * Flags describing the access point's capabilities according to WPA (Wifi Protected Access)
         */
        virtual Solid::Control::AccessPoint::WpaFlags wpaFlags() const = 0;

        /**
         * Flags describing the access point's capabilities according to RSN (Robust Secure Network)
         * aka WPA2
         */
        virtual Solid::Control::AccessPoint::WpaFlags rsnFlags() const = 0;

        /**
         * The Service Set Identifier of the access point
         */
        virtual QString ssid() const = 0;

        /**
         * Retrieves the frequency in MHz of the radio channel that this access point is operating on
         *
         * @return the frequency
         */
        virtual uint frequency() const = 0;

        /**
         * The hardware address assigned to the access point's wireless interface, in hex-and-colon
         * form
         */
        virtual QString  hardwareAddress() const = 0;

        /**
         * Retrieves the maximum bit rate currently attainable by this device.
         *
         * @return the maximum bitrate in kilobit/s
         */
        virtual uint maxBitRate() const = 0;

        /**
         * Retrieves the operation mode of this access point
         *
         * @return the current mode
         * @see Solid::Control::WirelessNetworkInterface::OperationMode
         */
        virtual Solid::Control::WirelessNetworkInterface::OperationMode mode() const = 0;

        /**
         * Retrieves the current signal strength of this wifi network.
         *
         * @return the signal strength as a percentage
         */
        virtual int signalStrength() const = 0;

    Q_SIGNALS:
        /**
         * This signal is emitted when the signal strength of this network has changed.
         *
         * @param strength the new signal strength value for this network
         */
        void signalStrengthChanged(int strength);

        /**
         * This signal is emitted when the bitrate of this network has changed.
         *
         * @param bitrate the new bitrate value for this network
         */
        void bitRateChanged(int bitrate);

        /**
         * This signal is emitted when the WPA flags in use by this access point change
         *
         * @param flags the new flags
         */
        void wpaFlagsChanged(Solid::Control::AccessPoint::WpaFlags flags);

        /**
         * This signal is emitted when the RSN(WPA2) flags in use by this access point change
         *
         * @param flags the new flags
         */
        void rsnFlagsChanged(Solid::Control::AccessPoint::WpaFlags flags);
        /**
         * This signal is emitted when the ssid of this Access Point changes
         *
         * @param ssid the new SSID
         */
        void ssidChanged(const QString &ssid);

        /**
         * This signal is emitted when the frequency used by this Access Point changes
         *
         * @param frequency the new frequency
         */
        void frequencyChanged(uint frequency);
    };
}
}
}

Q_DECLARE_INTERFACE(Solid::Control::Ifaces::AccessPoint, "org.kde.Solid.Control.Ifaces.AccessPoint/0.1")

#endif // SOLIDCONTROL_IFACES_WIRELESSAP_H

