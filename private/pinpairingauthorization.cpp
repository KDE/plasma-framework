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

#include "pinpairingauthorization_p.h"

#include "authorizationrule.h"
#include "clientpinrequest.h"
#include "pinpairingdialog_p.h"

#include "../remote/credentials.h"

#include <kdebug.h>


namespace Plasma
{

PinPairingAuthorization::PinPairingAuthorization()
{
}

PinPairingAuthorization::~PinPairingAuthorization()
{
    //TODO: cleanup
}

void PinPairingAuthorization::clientPinRequest(ClientPinRequest &request)
{
    kDebug();
    new PinPairingDialog(request);
}

void PinPairingAuthorization::authorizationRequest(AuthorizationRule &rule)
{
    kDebug();
    if (rule.credentials().trustLevel() > TrustedCredentials) {
        rule.setPolicy(AuthorizationRule::Allow);
        rule.setTargets(AuthorizationRule::AllServices);
    } else {
        rule.setPolicy(AuthorizationRule::PinRequired);
        new PinPairingDialog(rule);
    }
}

} // Plasma namespace

