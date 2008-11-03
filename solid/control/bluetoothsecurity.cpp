/*  This file is part of the KDE project
    Copyright (C) 2006 Will Stephenson <wstephenson@kde.org>
    Copyright (C) 2007 Daniel Gollub <dgollub@suse.de>
    Copyright (C) 2007 Juan González <jaguilera@opsiland.info>

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
#include <QMetaMethod>

#include <QMap>
#include <QPair>
#include <QStringList>

#include "ifaces/bluetoothsecurity.h"

#include "soliddefs_p.h"

#include "bluetoothmanager.h"
#include "bluetoothsecurity.h"
#include "bluetoothremotedevice.h"

#include "frontendobject_p.h"
#include "managerbase_p.h"

namespace Solid
{
namespace Control
{
    class BluetoothSecurityPrivate: public FrontendObjectPrivate
    {
        public:
            BluetoothSecurityPrivate(QObject *parent)
                :FrontendObjectPrivate(parent){};
        QList<BluetoothPasskeyAgent *> registeredAgents;
    };
} //Control
} //Solid

Solid::Control::BluetoothSecurity::BluetoothSecurity()
    :QObject(0),d(new BluetoothSecurityPrivate(this))
{
}

Solid::Control::BluetoothSecurity::BluetoothSecurity(QObject *backendObject)
    :QObject(backendObject),d(new BluetoothSecurityPrivate(this))
{
    d->setBackendObject(backendObject);
}

Solid::Control::BluetoothSecurity::~BluetoothSecurity()
{
}

Solid::Control::BluetoothSecurity &Solid::Control::BluetoothSecurity::operator=(const Solid::Control::BluetoothSecurity & dev)
{
    if (dev.d) {
        d->setBackendObject(dev.d->backendObject());
    }
    //FIXME We are loosing the reference to the agents here...
    return *this;
}

void Solid::Control::BluetoothSecurity::setPasskeyAgent(Solid::Control::BluetoothPasskeyAgent * agent)
{
    SOLID_CALL(Solid::Control::Ifaces::BluetoothSecurity *,d->backendObject(),setPasskeyAgent(agent))
}

void Solid::Control::BluetoothSecurity::setAuthorizationAgent(Solid::Control::BluetoothAuthorizationAgent * agent)
{
    SOLID_CALL(Solid::Control::Ifaces::BluetoothSecurity *,d->backendObject(),setAuthorizationAgent(agent))
}
/****************************** BluetoothPasskeyAgent implementation ******************************/
Solid::Control::BluetoothPasskeyAgent::BluetoothPasskeyAgent(QObject * parent,const QString &interface)
    :QObject(parent)
{
    //TODO Auto register all subclasses on BluetoothSecurity?
}

QString Solid::Control::BluetoothPasskeyAgent::remote()
{
    return m_remote;
}

QString Solid::Control::BluetoothPasskeyAgent::requestPasskey(const QString & ubi, bool isNumeric)
{
    Q_UNUSED(ubi)
    Q_UNUSED(isNumeric)
    return "";
}

bool Solid::Control::BluetoothPasskeyAgent::confirmPasskey(const QString & ubi, const QString & passkey)
{
    Q_UNUSED(ubi)
    Q_UNUSED(passkey)
    return false;
}

void Solid::Control::BluetoothPasskeyAgent::displayPasskey(const QString & ubi, const QString & passkey)
{
    Q_UNUSED(ubi)
    Q_UNUSED(passkey)
}

void Solid::Control::BluetoothPasskeyAgent::keypress(const QString & ubi)
{
    Q_UNUSED(ubi)
}

void Solid::Control::BluetoothPasskeyAgent::completedAuthentication(const QString & ubi)
{
    Q_UNUSED(ubi)
}

void Solid::Control::BluetoothPasskeyAgent::cancelAuthentication(const QString & ubi)
{
    Q_UNUSED(ubi)
}
/****************************** BluetoothAuthorizationAgent implementation ******************************/

Solid::Control::BluetoothAuthorizationAgent::BluetoothAuthorizationAgent(QObject * parent)
    :QObject(parent)
{

}

#include "bluetoothsecurity.moc"
