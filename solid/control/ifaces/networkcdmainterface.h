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

#ifndef SOLID_IFACES_CDMANETWORKINTERFACE_H
#define SOLID_IFACES_CDMANETWORKINTERFACE_H

#include "../solid_control_export.h"

#include <QtCore/QStringList>

#include "../networkcdmainterface.h"
#include "networkserialinterface.h"


namespace Solid
{
namespace Control
{
namespace Ifaces
{
    /**
     * Represents a wireless network interface
     */
    class SOLIDCONTROLIFACES_EXPORT CdmaNetworkInterface : virtual public SerialNetworkInterface
    {
    public:
        /**
         * Destroys a CdmaNetworkInterface object
         */
        virtual ~CdmaNetworkInterface();
        /**
         * List of wireless networks currently visible to the hardware
         */

    protected:
    //Q_SIGNALS:
    };
} //Ifaces
} //Control
} //Solid

Q_DECLARE_INTERFACE(Solid::Control::Ifaces::CdmaNetworkInterface, "org.kde.Solid.Control.Ifaces.CdmaNetworkInterface/0.1")

#endif //SOLID_IFACES_CDMANETWORKINTERFACE_H

