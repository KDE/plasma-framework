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

#ifndef PLASMA_AUTHORIZATIONMANAGER_H
#define PLASMA_AUTHORIZATIONMANAGER_H

#include "plasma_export.h"

#include <QtCore/QObject>

class QString;
class KUrl;

namespace Plasma
{

class AuthorizationInterface;
class AuthorizationManagerPrivate;
class ServiceAccessJob;
class ServiceJob;

/**
 * @class AuthorizationManager plasma/authorizationmanager.h <Plasma/AccessManager>
 *
 * @short Allows authorization of access to plasma services.
 *
 * This is the class where every message to or from another machine passes through.
 * It's responsibilities are:
 * - creating/keeping a credentials used for message signing.
 * - verifying credentials of incoming messages.
 * - testing whether or not the sender is allowed to access the requested resource by testing the
 *   request to a set of rules.
 * - allowing the shell to respond to a remote request that doesn't match any of the rules that
 *   are in effect.
 * Besides internal use in libplasma, the only moment you'll need to access this class is when you
 * implement a plasma shell.
 *
 * @since 4.4
 */
class PLASMA_EXPORT AuthorizationManager : public QObject
{
    Q_OBJECT

    public:
        enum AuthorizationPolicy {
            DenyAll= 0,  /** < Don't allow any incoming connections */
            TrustedOnly= 1, /**< Standard PIN pairing for untrusted connections */
            PinPairing= 2, /** < Only allow connections from trusted machines */
            Custom= 256 /** < Specify a custom AuthorizationInterface */
        };

        /**
         * Singleton pattern accessor.
         */
        static AuthorizationManager *self();

        /**
         * Set a policy used for authorizing incoming connections. You can either use one of the
         * included policies, Default is to deny all incoming connections. This can only be set
         * once to avoid that malicious plugins can change this. This means that you should ALWAYS
         * call this function in any plasma shell, even if you like to use the default DenyAll
         * policy.
         */
        void setAuthorizationPolicy(AuthorizationPolicy policy);

        /**
         * Register an implementation of AuthorizationInterface. Use this to make your shell
         * handle authorization requests. This can only be set once to avoid that malicious plugins
         * can change this.
         */
        void setAuthorizationInterface(AuthorizationInterface *interface);

    Q_SIGNALS:
        /**
         * fires when the AuthorizationManager is ready for accesssing remote plasmoids, meaning the
         * private key has been unlocked by the user.
         */
        void readyForRemoteAccess();

    private:
        AuthorizationManager();
        ~AuthorizationManager();

        AuthorizationManagerPrivate *const d;

        Q_PRIVATE_SLOT(d, void slotLoadRules())
        Q_PRIVATE_SLOT(d, void slotWalletOpened())

        friend class AccessManager;
        friend class AuthorizationManagerPrivate;
        friend class AuthorizationManagerSingleton;
        friend class AuthorizationRule;
        friend class AuthorizationRulePrivate;
        friend class Applet;
        friend class AppletPrivate;
        friend class Credentials;
        friend class DataEngine;
        friend class GetSource;
        friend class PackagePrivate;
        friend class PlasmoidServiceJob;
        friend class RemoteService;
        friend class RemoteServiceJob;
        friend class ServicePrivate;
        friend class ServiceProvider;
};
} // Plasma namespace

#endif

