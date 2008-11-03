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

#ifndef FAKE_WIRELESS_NETWORK_INTERFACE_H
#define FAKE_WIRELESS_NETWORK_INTERFACE_H

#include "fakenetworkinterface.h"
#include <solid/control/ifaces/wirelessnetworkinterface.h>
#include <QString>
#include <QMap>
#include <QVariant>

class FakeAccessPoint;
#include <kdemacros.h>

class KDE_EXPORT FakeWirelessNetworkInterface : public FakeNetworkInterface, virtual public Solid::Control::Ifaces::WirelessNetworkInterface
{
Q_OBJECT
Q_INTERFACES(Solid::Control::Ifaces::WirelessNetworkInterface)
friend class FakeNetworkManager;
public:
    FakeWirelessNetworkInterface(const QMap<QString, QVariant> & propertyMap, QObject * parent = 0);
    ~FakeWirelessNetworkInterface();
    MacAddressList accessPoints() const;
    QString activeAccessPoint() const;
    QString hardwareAddress() const;
    Solid::Control::WirelessNetworkInterface::OperationMode mode() const;
    int bitRate() const;
    Solid::Control::WirelessNetworkInterface::Capabilities wirelessCapabilities() const;
    QObject * createAccessPoint(const QString & uni);
signals:
    void bitRateChanged(int bitrate);
    void activeAccessPointChanged(const QString &);
    void modeChanged(Solid::Control::WirelessNetworkInterface::OperationMode);
    void accessPointAppeared(const QString &);
    void accessPointDisappeared(const QString &);
protected:
    void injectAccessPoint( FakeAccessPoint * ap );
private:
    QMap<QString, FakeAccessPoint *> mAccessPoints;

};

#endif

