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

#ifndef SOLID_NETWORK_P_H
#define SOLID_NETWORK_P_H

#include "networkinterface_p.h"

namespace Solid
{
namespace Control
{
    class AccessPoint;
namespace Ifaces
{
    class AccessPoint;
}

    typedef QPair<AccessPoint *, Ifaces::AccessPoint *> AccessPointPair;
    typedef QMap<QString, AccessPointPair > AccessPointMap;


    class WirelessNetworkInterfacePrivate : public NetworkInterfacePrivate
    {
    public:
        explicit WirelessNetworkInterfacePrivate(QObject *parent)
            : NetworkInterfacePrivate(parent) { }

        void setBackendObject(QObject *object);

        //Solid::Control::AccessPoint *findRegisteredAccessPoint(const QString &uni) const;
        void readAccessPoints();
        AccessPointPair createAP(const QString &uni) const;

        //mutable QMap<QString, AccessPoint *> apMap;
        mutable AccessPointMap apMap;
    };
}
}

#endif
