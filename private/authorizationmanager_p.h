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

#ifndef AUTHORIZATIONMANAGER_P_H
#define AUTHORIZATIONMANAGER_P_H

#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCrypto/QtCrypto>

#include <plasma/authorizationmanager.h>
#include <plasma/credentials.h>
#include <kconfiggroup.h>

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

        void loadRules();
        
        /**
         * @returns the rule matching the parameters with the highest priority.
         */
        AuthorizationRule *matchingRule(const QString &serviceName, const Credentials &key) const;

        /**
         * @returns the identity with @p id. Or 0 well there's no identity with that ID.
         */
        Credentials getCredentials(const QString &id = QString());

        /**
         * @param identity the identity to be added to the identity ring.
         */
        void addCredentials(const Credentials &identity);

        void slotWalletOpened();

        QCA::Initializer            initializer;

        AuthorizationManager        *q;
        Jolie::Server               *server;
        AuthorizationManager::AuthorizationPolicy
                                    authorizationPolicy;
        AuthorizationInterface      *authorizationInterface;
        AuthorizationInterface      *customAuthorizationInterface;
        KWallet::Wallet             *wallet;

        Credentials                    myCredentials;
        QMap<QString, Credentials>     identities;
        QList<AuthorizationRule*>   rules;
        KConfigGroup                identitiesConfig;
        KConfigGroup                rulesConfig;
        bool                        locked;
};

}

#endif
