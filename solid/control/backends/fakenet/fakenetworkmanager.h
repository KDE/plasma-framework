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

#ifndef FAKE_NETWORK_MANAGER_H
#define FAKE_NETWORK_MANAGER_H

#include <QObject>
#include <QStringList>

#include <kdemacros.h>

#include <solid/control/ifaces/networkmanager.h>

#include "fakenetworkinterface.h"


class FakeAuthenticationValidator;
class QDomElement;
//class FakeNetworkInterface;

namespace Solid {
    class Authentication;
}

class KDE_EXPORT FakeNetworkManager : public Solid::Control::Ifaces::NetworkManager
{
Q_OBJECT
    public:
        FakeNetworkManager(QObject * parent, const QStringList  & args);
        FakeNetworkManager(QObject * parent, const QStringList  & args, const QString  & xmlFile);
        virtual ~FakeNetworkManager();
        Solid::Networking::Status status() const;
        QStringList networkInterfaces() const;
        QStringList activeNetworkInterfaces() const;
        QObject * createNetworkInterface(const QString &);

        bool isNetworkingEnabled() const;
        bool isWirelessEnabled() const;
        bool isWirelessHardwareEnabled() const;

        void activateConnection(const QString & interfaceUni, const QString & connectionUni, const QVariantMap & connectionParameters);

        void deactivateConnection(const QString & activeConnection);
        QStringList activeConnections() const;

    public Q_SLOTS:
        void setWirelessEnabled(bool);
        void setNetworkingEnabled(bool);

    private:
        void parseNetworkingFile();
        FakeNetworkInterface *parseDeviceElement(const QDomElement &deviceElement);
        QMap<QString,QVariant> parseAPElement(const QDomElement &deviceElement);

        bool mUserNetworkingEnabled;
        bool mUserWirelessEnabled;
        bool mRfKillEnabled;
        QMap<QString, FakeNetworkInterface *> mNetworkInterfaces;
        QStringList mActiveConnections;
        QString mXmlFile;
};

#endif
