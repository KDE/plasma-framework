/*  This file is part of the KDE project
    Copyright (C) 2006 Kevin Ottens <ervin@kde.org>
    Copyright (C) 2006,2007 Will Stephenson <wstephenson@kde.org>

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

#include "soliddefs_p.h"
#include "authentication.h"
#include "networkmanager_p.h"
#include <solid/control/networkmanager.h>
#include <solid/control/ifaces/networkmanager.h>
#include <solid/control/ifaces/authentication.h>

namespace Solid
{
namespace Control
{
    class AuthenticationValidator::Private
    {
    public:
        QObject *backendObject;
    };

    class Authentication::Private
    {
    public:
        SecretMap secrets;
    };

    class AuthenticationWep::Private
    {
    public:
        WepMethod method;
        WepType type;
        int keyLength;
    };

    class AuthenticationWpa::Private
    {
    public:
        WpaProtocol protocol;
        WpaVersion version;
        WpaKeyManagement mgmt;
    };

    class AuthenticationWpaEnterprise::Private
    {
    public:
        QString identity;
        QString anonIdentity;
        QString certClient;
        QString certCA;
        QString certPrivate;
        EapMethod method;
        QString idPasswordKey;
        QString certPrivatePasswordKey;
    };
}
}


/******************************************************************************/

Solid::Control::Authentication::Authentication()
    : d(new Private)
{

}

Solid::Control::Authentication::~Authentication()
{
    delete d;
}

void Solid::Control::Authentication::setSecrets(const SecretMap &secrets)
{
    d->secrets = secrets;
}

Solid::Control::Authentication::SecretMap Solid::Control::Authentication::secrets() const
{
    return d->secrets;
}



/******************************************************************************/

Solid::Control::AuthenticationNone::AuthenticationNone()
    : d(0)
{

}

Solid::Control::AuthenticationNone::~AuthenticationNone()
{

}



/******************************************************************************/

Solid::Control::AuthenticationWep::AuthenticationWep()
    : d(new Private)
{

}

Solid::Control::AuthenticationWep::~AuthenticationWep()
{
    delete d;
}

void Solid::Control::AuthenticationWep::setMethod(WepMethod method)
{
    d->method = method;
}

Solid::Control::AuthenticationWep::WepMethod Solid::Control::AuthenticationWep::method() const
{
    return d->method;
}

void Solid::Control::AuthenticationWep::setType(WepType type)
{
    d->type = type;
}

Solid::Control::AuthenticationWep::WepType Solid::Control::AuthenticationWep::type() const
{
    return d->type;
}

void Solid::Control::AuthenticationWep::setKeyLength(int length)
{
    d->keyLength = length;
}

int Solid::Control::AuthenticationWep::keyLength() const
{
    return d->keyLength;
}

/******************************************************************************/

Solid::Control::AuthenticationWpa::AuthenticationWpa()
    : d(new Private)
{

}

Solid::Control::AuthenticationWpa::~AuthenticationWpa()
{
    delete d;
}

void Solid::Control::AuthenticationWpa::setProtocol(WpaProtocol protocol)
{
    d->protocol = protocol;
}

Solid::Control::AuthenticationWpa::WpaProtocol Solid::Control::AuthenticationWpa::protocol() const
{
    return d->protocol;
}

void Solid::Control::AuthenticationWpa::setVersion(WpaVersion version)
{
    d->version = version;
}

Solid::Control::AuthenticationWpa::WpaVersion Solid::Control::AuthenticationWpa::version() const
{
    return d->version;
}

void Solid::Control::AuthenticationWpa::setKeyManagement(WpaKeyManagement mgmt)
{
    d->mgmt = mgmt;
}

Solid::Control::AuthenticationWpa::WpaKeyManagement Solid::Control::AuthenticationWpa::keyManagement() const
{
    return d->mgmt;
}

/******************************************************************************/

Solid::Control::AuthenticationWpaPersonal::AuthenticationWpaPersonal()
    : d(0)
{

}

Solid::Control::AuthenticationWpaPersonal::~AuthenticationWpaPersonal()
{

}



/******************************************************************************/

Solid::Control::AuthenticationWpaEnterprise::AuthenticationWpaEnterprise()
    : d(new Private)
{

}

Solid::Control::AuthenticationWpaEnterprise::~AuthenticationWpaEnterprise()
{
    delete d;
}

void Solid::Control::AuthenticationWpaEnterprise::setIdentity(const QString &identity)
{
    d->identity = identity;
}

QString Solid::Control::AuthenticationWpaEnterprise::identity() const
{
    return d->identity;
}

void Solid::Control::AuthenticationWpaEnterprise::setAnonIdentity(const QString &anonIdentity)
{
    d->anonIdentity = anonIdentity;
}

QString Solid::Control::AuthenticationWpaEnterprise::anonIdentity() const
{
    return d->anonIdentity;
}

void Solid::Control::AuthenticationWpaEnterprise::setCertClient(const QString &certClient)
{
    d->certClient = certClient;
}

QString Solid::Control::AuthenticationWpaEnterprise::certClient() const
{
    return d->certClient;
}

void Solid::Control::AuthenticationWpaEnterprise::setCertCA(const QString &certCA)
{
    d->certCA = certCA;
}

QString Solid::Control::AuthenticationWpaEnterprise::certCA() const
{
    return d->certCA;
}

void Solid::Control::AuthenticationWpaEnterprise::setCertPrivate(const QString &certPrivate)
{
    d->certPrivate = certPrivate;
}

QString Solid::Control::AuthenticationWpaEnterprise::certPrivate() const
{
    return d->certPrivate;
}


void Solid::Control::AuthenticationWpaEnterprise::setMethod(EapMethod method)
{
    d->method = method;
}

Solid::Control::AuthenticationWpaEnterprise::EapMethod Solid::Control::AuthenticationWpaEnterprise::method() const
{
    return d->method;
}

void Solid::Control::AuthenticationWpaEnterprise::setIdPasswordKey(const QString &idPasswordKey)
{
    d->idPasswordKey = idPasswordKey;
}

QString Solid::Control::AuthenticationWpaEnterprise::idPasswordKey() const
{
    return d->idPasswordKey;
}

void Solid::Control::AuthenticationWpaEnterprise::setCertPrivatePasswordKey(const QString &certPrivatePasswordKey)
{
    d->certPrivatePasswordKey = certPrivatePasswordKey;
}

QString Solid::Control::AuthenticationWpaEnterprise::certPrivatePasswordKey() const
{
    return d->certPrivatePasswordKey;
}



Solid::Control::AuthenticationValidator::AuthenticationValidator()
    : d(new Private)
{
#if 0
    Ifaces::NetworkManager *backend = qobject_cast<Ifaces::NetworkManager *>(NetworkManager::self().d->managerBackend());

    if (backend)
    {
        d->backendObject = backend->createAuthenticationValidator();
    }
#endif
}

Solid::Control::AuthenticationValidator::~AuthenticationValidator()
{
    delete d->backendObject;
    delete d;
}

bool Solid::Control::AuthenticationValidator::validate(const Solid::Control::Authentication *authentication)
{
    return_SOLID_CALL(Ifaces::AuthenticationValidator *, d->backendObject, false, validate(authentication));
}

