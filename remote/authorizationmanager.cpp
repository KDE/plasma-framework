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
#include "private/denyallauthorization_p.h"
#include "private/joliemessagehelper_p.h"
#include "private/pinpairingauthorization_p.h"
#include "private/trustedonlyauthorization_p.h"

#include <QtCore/QBuffer>
#include <QtCore/QMap>
#include <QtCore/QMetaType>
#include <QtCore/QTimer>

#include <QtNetwork/QHostInfo>

#include <QtJolie/Message>
#include <QtJolie/Server>

#include <kauthaction.h>
#include <kconfiggroup.h>
#include <kdebug.h>
#include <kstandarddirs.h>
#include <ktemporaryfile.h>
#include <kurl.h>
#include <kwallet.h>

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
      server(0),
      authorizationPolicy(AuthorizationManager::DenyAll),
      authorizationInterface(new DenyAllAuthorization()),
      customAuthorizationInterface(0),
      rulesConfig(KSharedConfig::openConfig("/etc/plasma-remotewidgets.conf")->group("Rules")),
      locked(false)
{
}

AuthorizationManagerPrivate::~AuthorizationManagerPrivate()
{
    delete authorizationInterface;
    delete customAuthorizationInterface;
    delete server;
}

void AuthorizationManagerPrivate::prepareForServiceAccess()
{
    if (myCredentials.isValid()) {
        return;
    }

    wallet = KWallet::Wallet::openWallet("kdewallet", 0, KWallet::Wallet::Asynchronous);
    q->connect(wallet, SIGNAL(walletOpened(bool)), q, SLOT(slotWalletOpened()));
    QTimer::singleShot(0, q, SLOT(slotLoadRules()));
}

void AuthorizationManagerPrivate::prepareForServicePublication()
{
    if (!server) {
        server = new Jolie::Server(4000);
    }
}

void AuthorizationManagerPrivate::saveRules()
{
    kDebug() << "SAVE RULES";

    KTemporaryFile tempFile;
    tempFile.open();
    tempFile.setAutoRemove(false);
    KConfigGroup rulesGroup = KSharedConfig::openConfig(tempFile.fileName())->group("Rules");

    int i = 0;
    foreach (AuthorizationRule *rule, rules) {
        if (rule->persistence() == AuthorizationRule::Persistent) {
            kDebug() << "adding rule " << i;
            rulesGroup.group(QString::number(i)).writeEntry("CredentialsID", rule->credentials().id());
            rulesGroup.group(QString::number(i)).writeEntry("serviceName", rule->serviceName());
            rulesGroup.group(QString::number(i)).writeEntry("Policy", (uint)rule->policy());
            rulesGroup.group(QString::number(i)).writeEntry("Targets", (uint)rule->targets());
            rulesGroup.group(QString::number(i)).writeEntry("Persistence", (uint)rule->persistence());
            i++;
        }
    }
    rulesGroup.sync();
    tempFile.close();

    kDebug() << "tempfile = " << tempFile.fileName();

    KAuth::Action action("org.kde.kcontrol.kcmremotewidgets.save");
    action.addArgument("source", tempFile.fileName());
    action.addArgument("filename", "/etc/plasma-remotewidgets.conf");
    KAuth::ActionReply reply = action.execute();

    if (reply.failed()) {
        kDebug() << "KAuth failed.... YOU SUCK!";
    }
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

void AuthorizationManagerPrivate::slotLoadRules()
{
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
    }
}

} // Plasma namespace

#include "authorizationmanager.moc"
