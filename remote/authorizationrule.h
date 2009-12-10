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

#ifndef PLASMA_AUTHORIZATIONRULE_H
#define PLASMA_AUTHORIZATIONRULE_H

#include "plasma_export.h"

#include <QtCore/QObject>

class QString;
class KUrl;

namespace QCA
{
    class PublicKey;
}

namespace Plasma
{

class AuthorizationManager;
class AuthorizationRulePrivate;
class Credentials;

/**
 * @class AuthorizationRule plasma/authorizationrule.h <Plasma/AuthorizationRule>
 *
 * @short Defines a rule indicating whether or not a certain service can be accessed by a certain
 * machine.
 *
 * Rules allow you to have control over which computers are allowed to access which
 * services. Every time a message get's in, AuthorizationManager validates it's sender, and then
 * checks it's list of rules for rules matching the sender and/or the service. If no rules match,
 * or all matching rules have the value Unspecified, AuthorizationManager will create a new rule
 * for this message, and invoke authorize on your shells implementation of AuthorizationInterface.
 * Here, you can change that rule to either allow or deny that request.
 * This class can be used to specify different types of rules:
 * - Rules matching only a user
 * - Rules matching only a service
 * - Rules matching both a service, and a user.
 * A more specific rule always takes precedence over a more global rule: so if for example you have
 * a rule for "myAwesomeService" specifying Deny, and a rule for "myAwesomeService" in combination
 * with "130.42.120.146" as caller specifying Allow, only 130.42.120.146 can access
 * myAwesomeService.
 * By setting the PinRequired flag in setRules in an AuthorizationInterface implementation, you
 * trigger Pin pairing (user will be asked to enter the same password on both machines).
 *
 * @since 4.4?
 */
class PLASMA_EXPORT AuthorizationRule : public QObject
{
    Q_OBJECT
    public:
        ~AuthorizationRule();

        enum Policy {
            Deny = 0,           /**< access for messages matching this rule is denied. */
            Allow = 1,          /**< access for messages matching this rule is allowed. */
            PinRequired = 2     /**< specify that the user will need to enter a pin at both sides */
        };

        enum Persistence {
            Transient = 0,      /**< specify that this rule is just valid for this session. */
            Persistent = 1      /**< specify that this rule will be saved between sessions. */
        };

        enum Target {
            Default = 0,
            AllUsers = 1,       /**< specify that this rule is valid for all users */
            AllServices = 2     /**< specify that this rule is valid for all services */
        };
        Q_DECLARE_FLAGS(Targets, Target)

        /**
         * @returns a friendly and i18n'd description of the current rule, useful for creating a
         * GUI to allow editing rules, or asking permission for an access attempt.
         */
        QString description() const;

        /**
         * @param rules the flags describing this rule.
         */
        void setPolicy(Policy policy);

        /**
         * @returns the flags describing this rule.
         */
        Policy policy();

        /**
         * @param rules the flags describing this rule.
         */
        void setPersistence(Persistence persistence);

        /**
         * @returns the flags describing this rule.
         */
        Persistence persistence();

        /**
         * @param rules the flags describing this rule.
         */
        void setTargets(Targets targets);

        /**
         * @returns the flags describing this rule.
         */
        Targets targets();

        /**
         * @param pin set pin for pin pairing. You'll need to call this bevore setting the rule.
         */
        void setPin(const QString &pin);

        /**
         * @returns the pin for pin pairing.
         */
        QString pin() const;

        /**
         * @returns the identity of the caller.
         */
        Credentials credentials() const;

        /**
         * @returns the name of the service this rule applies to.
         */
        QString serviceName() const;

    Q_SIGNALS:
        void changed(Plasma::AuthorizationRule *);

    private:
        AuthorizationRule();
        AuthorizationRule(const QString &serviceName, const QString &identityID);

        AuthorizationRulePrivate * const d;

        Q_PRIVATE_SLOT(d, void fireChangedSignal())

        friend class AppletPrivate;
        friend class AuthorizationManager;
        friend class AuthorizationManagerPrivate;
        friend class AuthorizationRulePrivate;
        friend class GetSource;
        friend class PlasmoidServiceJob;
        friend class ServiceProvider;
};
} // Plasma namespace

Q_DECLARE_OPERATORS_FOR_FLAGS(Plasma::AuthorizationRule::Targets)

#endif

