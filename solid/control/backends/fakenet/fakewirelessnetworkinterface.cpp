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

#include <QStringList>

#include "fakeaccesspoint.h"
#include "fakewirelessnetworkinterface.h"

FakeWirelessNetworkInterface::FakeWirelessNetworkInterface(const QMap<QString, QVariant>  & propertyMap, QObject * parent)
: FakeNetworkInterface(propertyMap, parent)
{
}

FakeWirelessNetworkInterface::~FakeWirelessNetworkInterface()
{
}

MacAddressList FakeWirelessNetworkInterface::accessPoints() const
{
    return mAccessPoints.keys();
}

QString FakeWirelessNetworkInterface::activeAccessPoint() const
{
    return mPropertyMap["activeaccesspoint"].toString();
}

QString FakeWirelessNetworkInterface::hardwareAddress() const
{
    return mPropertyMap["hwaddress"].toString();

}

Solid::Control::WirelessNetworkInterface::OperationMode FakeWirelessNetworkInterface::mode() const
{
    return (Solid::Control::WirelessNetworkInterface::OperationMode)mPropertyMap["operationmode"].toUInt();
}


int FakeWirelessNetworkInterface::bitRate() const
{
    return mPropertyMap["bitrate"].toInt();
}

Solid::Control::WirelessNetworkInterface::Capabilities FakeWirelessNetworkInterface::wirelessCapabilities() const
{
    return (Solid::Control::WirelessNetworkInterface::Capabilities)mPropertyMap["wirelesscaps"].toUInt();
}

QObject * FakeWirelessNetworkInterface::createAccessPoint(const QString & uni)
{
    return 0;
}

void FakeWirelessNetworkInterface::injectAccessPoint( FakeAccessPoint * ap )
{
    mAccessPoints.insert(ap->uni(), ap );
}

#include "fakewirelessnetworkinterface.moc"
