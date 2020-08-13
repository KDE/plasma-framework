/*
    SPDX-FileCopyrightText: 2009 Rob Scheepmaker <r.scheepmaker@student.utwente.nl>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef PLASMA_AUTHORIZATIONMANAGER_P_H
#define PLASMA_AUTHORIZATIONMANAGER_P_H

#include "config-plasma.h"

#include <QMap>
#include <QString>

#include <KConfigGroup>

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

    AuthorizationManager        *q;
    Jolie::Server               *server;
    AuthorizationManager::AuthorizationPolicy
    authorizationPolicy;
    AuthorizationInterface      *authorizationInterface;
    AuthorizationInterface      *customAuthorizationInterface;
    KWallet::Wallet             *wallet;

    Credentials                 myCredentials;
    QMap<QString, Credentials>  identities;
    QList<AuthorizationRule *>   rules;
    KConfigGroup                identitiesConfig;
    KConfigGroup                rulesConfig;
    bool                        locked;
};

}

#endif // PLASMA_AUTHORIZATIONMANAGER_P_H
