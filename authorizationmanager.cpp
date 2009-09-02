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

#include "authorizationmanager.h"
#include "private/authorizationmanager_p.h"

#include "authorizationinterface.h"
#include "authorizationrule.h"
#include "credentials.h"
#include "service.h"
#include "servicejob.h"

#include "private/authorizationrule_p.h"
#include "private/denyallauthorization.h"
#include "private/pinpairingauthorization.h"
#include "private/trustedonlyauthorization.h"
#include "private/joliemessagehelper_p.h"

#include <QtCore/QBuffer>
#include <QtCore/QMap>
#include <QNetworkInterface>
#include <QHostInfo>
#include <QtCore/QTimer>
#include <QtCore/QMetaType>

#include <QtCrypto/QtCrypto>
#include <QtJolie/Message>
#include <QtJolie/Server>

#include <KDebug>
#include <KUrl>
#include <KWallet/Wallet>
#include <KConfigGroup>
#include <kstandarddirs.h>

namespace Plasma
{

class AuthorizationManagerSingleton
{
    public:
        AuthorizationManager self;
};

K_GLOBAL_STATIC(AuthorizationManagerSingleton, privateAuthorizationManagerSelf)

AuthorizationManager *AuthorizationManager::self()
{
    return &privateAuthorizationManagerSelf->self;
}

AuthorizationManager::AuthorizationManager()
    : QObject(),
      d(new AuthorizationManagerPrivate(this))
{
    qRegisterMetaTypeStreamOperators<Plasma::Credentials>("Plasma::Credentials");
}

AuthorizationManager::~AuthorizationManager()
{
    delete d;
}

void AuthorizationManager::setAuthorizationPolicy(AuthorizationPolicy policy)
{
    if (d->locked) {
        kDebug() << "Can't change AuthorizationPolicy: interface locked.";
        return;
    }

    if (policy == d->authorizationPolicy) {
        return;
    }

    d->authorizationPolicy = policy;

    if (d->authorizationInterface != d->customAuthorizationInterface) {
        delete d->authorizationInterface;
    }

    switch (policy) {
        case DenyAll:
            d->authorizationInterface = new DenyAllAuthorization();
            break;
        case PinPairing:
            d->authorizationInterface = new PinPairingAuthorization();
            break;
        case TrustedOnly:
            d->authorizationInterface = new TrustedOnlyAuthorization();
            break;
        case Custom:
            d->authorizationInterface = d->customAuthorizationInterface;
            break;
    }

    d->locked = true;
}

void AuthorizationManager::setAuthorizationInterface(AuthorizationInterface *interface)
{
    if (d->authorizationInterface) {
        kDebug() << "Can't change AuthorizationInterface: interface locked.";
        return;
    }

    delete d->customAuthorizationInterface;
    d->customAuthorizationInterface = interface;

    if (d->authorizationPolicy == Custom) {
        d->authorizationInterface = interface;
    }
}

AuthorizationManagerPrivate::AuthorizationManagerPrivate(AuthorizationManager *manager) 
    : q(manager),
      authorizationPolicy(AuthorizationManager::DenyAll),
      authorizationInterface(new DenyAllAuthorization()),
      customAuthorizationInterface(0),
      identitiesConfig(KSharedConfig::openConfig("plasma-identityrc")->group("Identitites")),
      rulesConfig(KSharedConfig::openConfig("plasma-rulesrc")->group("Rules")),
      locked(false)
{
    wallet = KWallet::Wallet::openWallet("Plasma", 0, KWallet::Wallet::Asynchronous);
    q->connect(wallet, SIGNAL(walletOpened(bool)), q, SLOT(slotWalletOpened()));

    //Let's set up plasma for remote service support. Since most of the set up involves crypto,
    //AuthorizationManager seems the sensible place.
    //First, let's start the JOLIE server:
    server = new Jolie::Server(4000);

    /**
    //Actually: storing identities isn't really necesarry. TODO: remove this and see if stuff
    //breaks.
    foreach (const QString &groupName, identitiesConfig.groupList()) {
        QByteArray identityByteArray =
            identitiesConfig.group(groupName).readEntry("Credentials", QByteArray());
        QDataStream stream(&identityByteArray, QIODevice::ReadOnly);
        Credentials storedCredentials;
        stream >> storedCredentials;
        //Credentials storedCredentials = identityVariant.value<Credentials>();
        if (storedCredentials.isNull()) {
            kDebug() << "stored identity is null";
        } else {
            identities[storedCredentials.id()] = storedCredentials;
        }
    }
    */

    QTimer::singleShot(0, q, SLOT(loadRules()));
}

AuthorizationManagerPrivate::~AuthorizationManagerPrivate()
{
    int i = 0;
    foreach (AuthorizationRule *rule, rules) {
        kDebug() << "adding rule " << i;
        rulesConfig.group(QString::number(i)).writeEntry("CredentialsID", rule->credentials().id());
        rulesConfig.group(QString::number(i)).writeEntry("serviceName", rule->serviceName());
        rulesConfig.group(QString::number(i)).writeEntry("Policy", (uint)rule->policy());
        rulesConfig.group(QString::number(i)).writeEntry("Targets", (uint)rule->targets());
        rulesConfig.group(QString::number(i)).writeEntry("Persistence", (uint)rule->persistence());
        i++;
    }
    rulesConfig.sync();

    delete authorizationInterface;
    delete customAuthorizationInterface;
    delete server;
    delete wallet;
}

void AuthorizationManagerPrivate::slotWalletOpened()
{
    QByteArray identity;

    if (!wallet->readEntry("Credentials", identity)) {
        kDebug() << "Existing identity found";
        QDataStream stream(&identity, QIODevice::ReadOnly);
        stream >> myCredentials;
    }

    if (!myCredentials.isValid()) {
        kDebug() << "Creating a new identity";
        myCredentials = Credentials::createCredentials(QHostInfo::localHostName());
        QDataStream stream(&identity, QIODevice::WriteOnly);
        stream << myCredentials;
        wallet->writeEntry("Credentials", identity);
    }

    emit q->readyForRemoteAccess();
}

void AuthorizationManagerPrivate::loadRules()
{
    //TODO: at some point we're probably going to use kauth for this stuff, by which time this will
    //become kind of obsolete.
    foreach (const QString &groupName, rulesConfig.groupList()) {
        QString identityID = rulesConfig.group(groupName).readEntry("CredentialsID", "");
        QString serviceName = rulesConfig.group(groupName).readEntry("serviceName", "");
        uint policy = rulesConfig.group(groupName).readEntry("Policy", 0);
        uint targets = rulesConfig.group(groupName).readEntry("Targets", 0);
        uint persistence = rulesConfig.group(groupName).readEntry("Persistence", 0);
        //Credentials storedCredentials = identities[identityID];
        if (serviceName.isEmpty()) {
            kDebug() << "Invalid rule";
        } else {
            AuthorizationRule *rule = new AuthorizationRule(serviceName, identityID);
            rule->setPolicy(static_cast<AuthorizationRule::Policy>(policy));
            rule->setTargets(static_cast<AuthorizationRule::Targets>(targets));
            rule->setPersistence(static_cast<AuthorizationRule::Persistence>(persistence));
            rules.append(rule);
        }
    }
}

AuthorizationRule *AuthorizationManagerPrivate::matchingRule(const QString &serviceName,
                                                             const Credentials &identity) const
{
    AuthorizationRule *matchingRule = 0;
    foreach (AuthorizationRule *rule, rules) {
        if (rule->d->matches(serviceName, identity.id())) {
            //a message can have multiple matching rules, consider priorities: the more specific the
            //rule is, the higher it's priority
            if (!matchingRule) {
                matchingRule = rule;
            } else {
                if (!matchingRule->targets().testFlag(AuthorizationRule::AllServices) &&
                    !matchingRule->targets().testFlag(AuthorizationRule::AllUsers)) {
                    matchingRule = rule;
                }
            }
        }
    }

    if (!matchingRule) {
        kDebug() << "no matching rule";
    } else {
        kDebug() << "matching rule found: " << matchingRule->description();
    }
    return matchingRule;
}

Credentials AuthorizationManagerPrivate::getCredentials(const QString &id)
{
    if (identities.contains(id)) {
        return identities[id];
    } else {
        return Credentials();
    }
}

void AuthorizationManagerPrivate::addCredentials(const Credentials &identity)
{
    if (identities.contains(identity.id())) {
        return;
    } else if (identity.isValid()) {
        kDebug() << "Adding a new identity for " << identity.id();
        identities[identity.id()] = identity;
        /**
        QByteArray identityByteArray;
        QDataStream stream(&identityByteArray, QIODevice::WriteOnly);
        stream << identity;

        identitiesConfig.group(identity.id()).writeEntry("Credentials", identityByteArray);
        identitiesConfig.sync();
        */
    }
}

} // Plasma namespace

#include "authorizationmanager.moc"
