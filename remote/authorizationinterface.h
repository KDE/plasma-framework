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

#ifndef PLASMA_AUTHORIZATIONINTERFACE_H
#define PLASMA_AUTHORIZATIONINTERFACE_H

#include "plasma_export.h"

#include <QtCore/QObject>

class QString;
class KUrl;

namespace Plasma
{

class AuthorizationRule;
class ClientPinRequest;

/**
 * @class AuthorizationInterface plasma/authorizationinterface.h <Plasma/AuthorizationInterface>
 *
 * @short Allows authorization of access to plasma services.
 *
 * This class is only needed when you create a plasma shell. When you implement it and register it
 * with the AuthorizationManager class, it allows you to respond to incoming service access
 * attempts. Whenever a message is received that does not match any of the AuthorizationRules,
 * AuthorizationManager creates a new rule matching it, and passes it to the authorize function.
 * Change the rule from Unspecified to something else like Allow or Deny to continue processing the
 * message.
 * It also allows you to outgoing access attempts that require pin pairing, to allow your shell to
 * show a dialog to ask the user for a password.
 *
 * @since 4.4
 */
class PLASMA_EXPORT AuthorizationInterface
{
    public:
        virtual ~AuthorizationInterface();

        /**
         * implement this function to respond to an incoming request that doesn't match any rule.
         * @param rule a new AuthorizationRule matching an incoming operation. Call setRules on this
         * rule to allow/deny the operation.
         */
        virtual void authorizationRequest(AuthorizationRule &rule) = 0;

        /**
         * Implement this function to respond to an outgoing connection that needs a password to
         * connect successfully. As a response to this you'll probably want to show a dialog.
         * @param request a ClientPinRequest where you can call setPin on to set the pin for the
         * outgoing connection.
         */
        virtual void clientPinRequest(ClientPinRequest &request) = 0;

    protected:
        AuthorizationInterface();
};

} // Plasma namespace

#endif

