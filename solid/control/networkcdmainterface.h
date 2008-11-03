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

#ifndef SOLID_CONTROL_CDMANETWORKINTERFACE_H
#define SOLID_CONTROL_CDMANETWORKINTERFACE_H

#include "networkserialinterface.h"

namespace Solid
{
namespace Control
{
    class CdmaNetworkInterfacePrivate;
    /**
     * This interface represents a CDMA cellular network interface
     */
    class SOLIDCONTROL_EXPORT CdmaNetworkInterface : public SerialNetworkInterface
    {
        Q_OBJECT
        Q_DECLARE_PRIVATE(CdmaNetworkInterface)

    public:
        /**
         * Creates a new CdmaNetworkInterface object.
         *
         * @param backendObject the network object provided by the backend
         */
        CdmaNetworkInterface(QObject *backendObject = 0);

        /**
         * Constructs a copy of a network.
         *
         * @param network the network to copy
         */
        CdmaNetworkInterface(const CdmaNetworkInterface &network);

        /**
         * Destroys a CdmaNetworkInterface object.
         */
        virtual ~CdmaNetworkInterface();

        /**
         * The NetworkInterface type.
         *
         * @return the NetworkInterface::Type.  This always returns NetworkInterface::Cdma
         */
        virtual NetworkInterface::Type type() const;

    Q_SIGNALS:
    protected:
        /**
         * @internal
         */
        CdmaNetworkInterface(CdmaNetworkInterfacePrivate &dd, QObject *backendObject);

        /**
         * @internal
         */
        CdmaNetworkInterface(CdmaNetworkInterfacePrivate &dd, const CdmaNetworkInterface &network);

        void makeConnections(QObject * source);
    private Q_SLOTS:
        void _k_destroyed(QObject *object);
    private:
        friend class NetworkInterface;
        friend class NetworkInterfacePrivate;
    };
} //Control
} //Solid

#endif // SOLID_CONTROL_CDMANETWORKINTERFACE_H

