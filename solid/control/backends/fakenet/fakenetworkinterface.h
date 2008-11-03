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

#ifndef FAKE_NETWORK_INTERFACE_H
#define FAKE_NETWORK_INTERFACE_H

#include <QString>
#include <QVariant>

#include <kdemacros.h>

#include <solid/control/ifaces/networkinterface.h>
#include <solid/control/networkipv4config.h>

using namespace Solid::Control::Ifaces;

class KDE_EXPORT FakeNetworkInterface : public QObject, virtual public Solid::Control::Ifaces::NetworkInterface
{
    friend class FakeNetworkManager;
    Q_OBJECT
    Q_INTERFACES(Solid::Control::Ifaces::NetworkInterface)
public:
        FakeNetworkInterface(const QMap<QString, QVariant>  & propertyMap,
                          QObject *parent = 0);
        virtual ~FakeNetworkInterface();
        QString uni() const;
        QString interfaceName() const;
        QString driver() const;
        bool isActive() const;
        Solid::Control::IPv4Config ipV4Config() const;
        Solid::Control::NetworkInterface::ConnectionState connectionState() const;
        int designSpeed() const;
        Solid::Control::NetworkInterface::Capabilities capabilities() const;
        void activate(const QString & connectionUni, const QString & extra_connection_parameter = QString() );
        void deactivate();
    Q_SIGNALS:
        void ipDetailsChanged();
        void activeChanged(bool);
        void linkUpChanged(bool);
        void connectionStateChanged(int /*NetworkStatus::ConnectionState */);

    protected:
        /* These methods are operations that are carried out by the manager
           In a real backend they would be events coming up from the network layer */
        QString mActiveConnection;
        QMap<QString, QVariant> mPropertyMap;
};

#endif
