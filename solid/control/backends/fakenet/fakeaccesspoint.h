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

#ifndef FAKE_ACCESS_POINT_H
#define FAKE_ACCESS_POINT_H

#include <solid/control/ifaces/wirelessaccesspoint.h>

#include <QString>
#include <QVariant>

#include <kdemacros.h>

class KDE_EXPORT FakeAccessPoint : public Solid::Control::Ifaces::AccessPoint
{
Q_OBJECT
Q_INTERFACES(Solid::Control::Ifaces::AccessPoint)
public:
    FakeAccessPoint(const QMap<QString,QVariant> & propsMap, QObject * parent = 0 );
    ~FakeAccessPoint();
    QString uni() const;
    Solid::Control::AccessPoint::Capabilities capabilities() const;
    Solid::Control::AccessPoint::WpaFlags wpaFlags() const;
    Solid::Control::AccessPoint::WpaFlags rsnFlags() const;
    QString ssid() const;
    uint frequency() const;
    QString hardwareAddress() const;
    uint maxBitRate() const;
    Solid::Control::WirelessNetworkInterface::OperationMode mode() const;
    int signalStrength() const;
signals:
    void ssidChanged(const QString &);
    void frequencyChanged(uint);
    void signalStrengthChanged(int strength);
    void bitRateChanged(int bitrate);
    void wpaFlagsChanged(Solid::Control::AccessPoint::WpaFlags flags);
    void rsnFlagsChanged(Solid::Control::AccessPoint::WpaFlags flags);
private:
    Solid::Control::AccessPoint::WpaFlags wpaPropsToFlags( const QString & property ) const;
    QMap<QString,QVariant> mPropertyMap;
    QString mUni;

};

#endif
