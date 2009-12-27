/*
 * Copyright 2009 by Rob Scheepmaker <r.scheepmaker@student.utwente.nl>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA  02110-1301  USA
 */

#ifndef PLASMA_AUTHORIZATIONMANAGER_P_H
#define PLASMA_AUTHORIZATIONMANAGER_P_H

#include "config-plasma.h"

#include <QtCore/QMap>
#include <QtCore/QString>

#ifdef ENABLE_REMOTE_WIDGETS
#include <QtCrypto>
#endif

#include <kconfiggroup.h>

#include <authorizationmanager.h>
#include <credentials.h>

class QByteArray;

namespace KWallet
{
    class Wallet;
} // namespace KWallet

namespace Jolie
{
    class Server;
} // namespace Jolie

namespace Plasma
{

class AuthorizationInterface;
class AuthorizationRule;
class Credentials;

class AuthorizationManagerPrivate
{
    public:
        AuthorizationManagerPrivate(AuthorizationManager *manager);
        ~AuthorizationManagerPrivate();

        void prepareForServiceAccess();
        void prepareForServicePublication();
        void slotWalletOpened();
        void slotLoadRules();
        AuthorizationRule *matchingRule(const QString &serviceName, const Credentials &key) const;
        Credentials getCredentials(const QString &id = QString());
        void addCredentials(const Credentials &identity);
        void saveRules();

#ifdef ENABLE_REMOTE_WIDGETS
        QCA::Initializer            initializer;
#endif

        AuthorizationManager        *q;
        Jolie::Server               *server;
        AuthorizationManager::AuthorizationPolicy
                                    authorizationPolicy;
        AuthorizationInterface      *authorizationInterface;
        AuthorizationInterface      *customAuthorizationInterface;
        KWallet::Wallet             *wallet;

        Credentials                 myCredentials;
        QMap<QString, Credentials>  identities;
        QList<AuthorizationRule*>   rules;
        KConfigGroup                identitiesConfig;
        KConfigGroup                rulesConfig;
        bool                        locked;
};

}

#endif // PLASMA_AUTHORIZATIONMANAGER_P_H
