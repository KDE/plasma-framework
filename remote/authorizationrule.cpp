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

#include "authorizationrule.h"

#include "authorizationmanager.h"
#include "credentials.h"
#include "private/authorizationmanager_p.h"
#include "private/authorizationrule_p.h"

#include <QtCore/QObject>
#include <QtCore/QTimer>

#include <kurl.h>
#include <klocalizedstring.h>

namespace Plasma
{

AuthorizationRulePrivate::AuthorizationRulePrivate(const QString &serviceName, const QString &credentialID,
                                                   AuthorizationRule *rule)
    : q(rule),
      serviceName(serviceName),
      credentialID(credentialID),
      policy(AuthorizationRule::Deny),
      targets(AuthorizationRule::Default),
      persistence(AuthorizationRule::Transient)
{
}

AuthorizationRulePrivate::~AuthorizationRulePrivate()
{
}

bool AuthorizationRulePrivate::matches(const QString &name, const QString &id) const
{
    if (serviceName == name && (credentialID == id)) {
        return true;
    }

    if (targets.testFlag(AuthorizationRule::AllUsers) && (serviceName == name)) {
        return true;
    }

    if (targets.testFlag(AuthorizationRule::AllServices) && (credentialID == id)) {
        return true;
    }

    return false;
}

void AuthorizationRulePrivate::scheduleChangedSignal()
{
    QTimer::singleShot(0, q, SLOT(fireChangedSignal()));
}

void AuthorizationRulePrivate::fireChangedSignal()
{
    if ((persistence == AuthorizationRule::Persistent) &&
        (policy != AuthorizationRule::PinRequired)) {
        AuthorizationManager::self()->d->saveRules();
    }

    emit q->changed(q);
}

AuthorizationRule::AuthorizationRule(const QString &serviceName, const QString &credentialID)
    : QObject(AuthorizationManager::self()),
      d(new AuthorizationRulePrivate(serviceName, credentialID, this))
{
}

AuthorizationRule::~AuthorizationRule()
{
    delete d;
}

QString AuthorizationRule::description() const
{
    //i18n megafest :p
    if (d->targets.testFlag(AllUsers) && d->policy == Allow) {
        return i18n("Allow everybody access to %1.", d->serviceName);
    } else if (d->targets.testFlag(AllUsers) && d->policy == Deny) {
        return i18n("Deny everybody access to %1", d->serviceName);
    } else if (d->targets.testFlag(AllServices) && d->policy == Allow) {
        return i18n("Allow %1 access to all services.", credentials().name());
    } else if (d->targets.testFlag(AllServices) && d->policy == Deny) {
        return i18n("Deny %1 access to all services.", credentials().name());
    } else if (d->policy == Allow) {
        return i18n("Allow access to %1, by %2.", d->serviceName, credentials().name());
    } else if (d->policy == Deny) {
        return i18n("Deny access to %1, by %2.", d->serviceName, credentials().name());
    } else {
        return i18n("Allow access to %1, by %2?", d->serviceName, credentials().name());
    }
}


void AuthorizationRule::setPolicy(Policy policy)
{
    d->policy = policy;
    d->scheduleChangedSignal();
}

AuthorizationRule::Policy AuthorizationRule::policy()
{
    return d->policy;
}

void AuthorizationRule::setTargets(Targets targets)
{
    d->targets = targets;
    d->scheduleChangedSignal();
}

AuthorizationRule::Targets AuthorizationRule::targets()
{
    return d->targets;
}

void AuthorizationRule::setPersistence(Persistence persistence)
{
    d->persistence = persistence;
    d->scheduleChangedSignal();
}

AuthorizationRule::Persistence AuthorizationRule::persistence()
{
    return d->persistence;
}

void AuthorizationRule::setPin(const QString &pin)
{
    d->pin = pin;
    d->scheduleChangedSignal();
}

QString AuthorizationRule::pin() const
{
    return d->pin;
}

Credentials AuthorizationRule::credentials() const
{
    return AuthorizationManager::self()->d->getCredentials(d->credentialID);
}

QString AuthorizationRule::serviceName() const
{
    return d->serviceName;
}

} // Plasma namespace

#include "authorizationrule.moc"
