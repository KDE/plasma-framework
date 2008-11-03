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

#include "fakeaccesspoint.h"

FakeAccessPoint::FakeAccessPoint( const QMap<QString, QVariant>  & propertyMap,
                                          QObject * parent)
: Solid::Control::Ifaces::AccessPoint( parent ), mPropertyMap(propertyMap)
{
}

FakeAccessPoint::~FakeAccessPoint()
{

}

// PHY stuff
QString FakeAccessPoint::uni() const
{
    return mPropertyMap["uni"].toString();
}


QString FakeAccessPoint::ssid() const
{
    return mPropertyMap["ssid"].toString();
}


uint FakeAccessPoint::frequency() const
{
    return mPropertyMap["frequency"].toUInt();
}

QString FakeAccessPoint::hardwareAddress() const
{
    return mPropertyMap["hwaddress"].toString();
}

uint FakeAccessPoint::maxBitRate() const
{
    return mPropertyMap["maxbitrate"].toUInt();
}

int FakeAccessPoint::signalStrength() const
{
    return mPropertyMap["signalstrength"].toInt();
}

Solid::Control::AccessPoint::WpaFlags FakeAccessPoint::wpaFlags() const
{
    return wpaPropsToFlags( "wpaflags" );
}

Solid::Control::AccessPoint::WpaFlags FakeAccessPoint::rsnFlags() const
{
    return wpaPropsToFlags( "rsnflags" );
}

Solid::Control::AccessPoint::WpaFlags FakeAccessPoint::wpaPropsToFlags( const QString & property ) const
{
    QStringList capStrings = mPropertyMap[property].toStringList();

    Solid::Control::AccessPoint::WpaFlags caps = 0;
    if (capStrings.contains("pairwep40"))
        caps |= Solid::Control::AccessPoint::PairWep40;
    if (capStrings.contains("pairweb104"))
        caps |= Solid::Control::AccessPoint::PairWep104;
    if (capStrings.contains("pairtkip"))
        caps |= Solid::Control::AccessPoint::PairTkip;
    if (capStrings.contains("pairccmp"))
        caps |= Solid::Control::AccessPoint::PairCcmp;
    if (capStrings.contains("groupwep40"))
        caps |= Solid::Control::AccessPoint::GroupWep40;
    if (capStrings.contains("groupweb104"))
        caps |= Solid::Control::AccessPoint::GroupWep104;
    if (capStrings.contains("grouptkip"))
        caps |= Solid::Control::AccessPoint::GroupTkip;
    if (capStrings.contains("groupccmp"))
        caps |= Solid::Control::AccessPoint::GroupCcmp;
    if (capStrings.contains("keypsk"))
        caps |= Solid::Control::AccessPoint::KeyMgmtPsk;
    if (capStrings.contains("key8021x"))
        caps |= Solid::Control::AccessPoint::KeyMgmt8021x;
    return caps;
}

Solid::Control::AccessPoint::Capabilities FakeAccessPoint::capabilities() const
{
    //TODO
    QStringList capStrings = mPropertyMap["capabilities"].toStringList();

    Solid::Control::AccessPoint::Capabilities caps = 0;
    if (capStrings.contains("privacy"))
        caps |= Solid::Control::AccessPoint::Privacy;
    return caps;
}

Solid::Control::WirelessNetworkInterface::OperationMode FakeAccessPoint::mode() const
{
    QString modeName = mPropertyMap["mode"].toString();

    if (modeName == "adhoc")
        return Solid::Control::WirelessNetworkInterface::Adhoc;
    else if (modeName == "managed")
        return Solid::Control::WirelessNetworkInterface::Managed;
    else if (modeName == "master")
        return Solid::Control::WirelessNetworkInterface::Master;
    else if (modeName == "repeater")
        return Solid::Control::WirelessNetworkInterface::Repeater;
    else
        return Solid::Control::WirelessNetworkInterface::Unassociated;
}

#include "fakeaccesspoint.moc"

