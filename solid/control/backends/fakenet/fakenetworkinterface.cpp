/*  This file is part of the KDE project
    Copyright (C) 2006,2008 Will Stephenson <wstephenson@kde.org>

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

#include <QStringList>

#include "fakenetworkinterface.h"

#include <kdebug.h>

FakeNetworkInterface::FakeNetworkInterface(const QMap<QString, QVariant>  & propertyMap, QObject * parent)
: QObject(parent), mPropertyMap(propertyMap)
{
}

FakeNetworkInterface::~FakeNetworkInterface()
{
}


QString FakeNetworkInterface::uni() const
{
    return mPropertyMap["uni"].toString();
}


QString FakeNetworkInterface::interfaceName() const
{
    return mPropertyMap["interface"].toString();
}

QString FakeNetworkInterface::driver() const
{
    return mPropertyMap["driver"].toString();
}
bool FakeNetworkInterface::isActive() const
{
    return !mActiveConnection.isEmpty();
}

Solid::Control::NetworkInterface::ConnectionState FakeNetworkInterface::connectionState() const
{
    QString stateString = mPropertyMap["state"].toString();

    if (stateString == "unmanaged")
        return Solid::Control::NetworkInterface::Unmanaged;
    else if (stateString == "unavailable")
        return Solid::Control::NetworkInterface::Unavailable;
    else if (stateString == "disconnected")
        return Solid::Control::NetworkInterface::Disconnected;
    else if (stateString == "preparing")
        return Solid::Control::NetworkInterface::Preparing;
    else if (stateString == "configuring")
        return Solid::Control::NetworkInterface::Configuring;
    else if (stateString == "needauth")
        return Solid::Control::NetworkInterface::NeedAuth;
    else if (stateString == "ipconfig")
        return Solid::Control::NetworkInterface::IPConfig;
    else if (stateString == "activated")
        return Solid::Control::NetworkInterface::Activated;
    else if (stateString == "failed")
        return Solid::Control::NetworkInterface::Failed;
    else
        return Solid::Control::NetworkInterface::UnknownState;
}

int FakeNetworkInterface::designSpeed() const
{
    return mPropertyMap["speed"].toInt();
}

Solid::Control::NetworkInterface::Capabilities FakeNetworkInterface::capabilities() const
{
    QStringList capStrings = mPropertyMap["capabilities"].toString().simplified().split(',');

    Solid::Control::NetworkInterface::Capabilities caps = 0;
    if (capStrings.contains("manageable"))
        caps |= Solid::Control::NetworkInterface::IsManageable;
    if (capStrings.contains("carrierdetect"))
        caps |= Solid::Control::NetworkInterface::SupportsCarrierDetect;
    return caps;
}

void FakeNetworkInterface::activate(const QString & connectionUni, const QString &)
{
    mActiveConnection = connectionUni;
}

void FakeNetworkInterface::deactivate()
{
    mActiveConnection = "";
}


Solid::Control::IPv4Config FakeNetworkInterface::ipV4Config() const
{
    return Solid::Control::IPv4Config();
}

