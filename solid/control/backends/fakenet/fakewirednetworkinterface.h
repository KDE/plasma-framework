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

#ifndef FAKE_WIRED_NETWORK_INTERFACE_H
#define FAKE_WIRED_NETWORK_INTERFACE_H

#include <QString>
#include <QVariant>

#include <kdemacros.h>

#include <solid/control/ifaces/wirednetworkinterface.h>

#include "fakenetworkinterface.h"

using namespace Solid::Control::Ifaces;

class KDE_EXPORT FakeWiredNetworkInterface : public FakeNetworkInterface, virtual public Solid::Control::Ifaces::WiredNetworkInterface
{
Q_OBJECT
Q_INTERFACES(Solid::Control::Ifaces::WiredNetworkInterface)
public:
    FakeWiredNetworkInterface(const QMap<QString, QVariant> & propertyMap, QObject * parent = 0);
    ~FakeWiredNetworkInterface();
    QString hardwareAddress() const;
    int bitRate() const;
    bool carrier() const;
signals:
    void bitRateChanged(int bitrate);
    void carrierChanged(bool plugged);
};

#endif

