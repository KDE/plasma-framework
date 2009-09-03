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

#ifndef PRIVATE_AUTHORIZATIONRULE_P_H
#define PRIVATE_AUTHORIZATIONRULE_P_H

#include "../remote/authorizationrule.h"

namespace Plasma
{

class AuthorizationRulePrivate {
public:
    AuthorizationRulePrivate(const QString &serviceName, const QString &credentialID,
                             AuthorizationRule *rule);
    ~AuthorizationRulePrivate();
    bool matches(const QString &serviceName, const QString &credentialID) const;
    void scheduleChangedSignal();
    void fireChangedSignal();

    AuthorizationRule *q;
    QString serviceName;
    QString credentialID;
    bool PINvalidated;
    QString pin;

    AuthorizationRule::Policy policy;
    AuthorizationRule::Targets targets;
    AuthorizationRule::Persistence persistence;
};

}
#endif // PRIVATE_AUTHORIZATIONRULE_P_H
