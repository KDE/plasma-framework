/*  This file is part of the KDE project
    Copyright (C) 2006 Kevin Ottens <ervin@kde.org>
    Copyright (C) 2007 Will Stephenson <wstephenson@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.

*/

#ifndef SOLID_CONTROL_AUTHENTICATION_H
#define SOLID_CONTROL_AUTHENTICATION_H

#include <solid/control/ifaces/authentication.h>

namespace Solid
{
namespace Control
{
    /**
     * Base class for wireless authentication schemes.  No need to instantiate this
     */
    class SOLIDCONTROL_EXPORT Authentication
    {
    public:
        typedef QMap<QString, QString> SecretMap;

        Authentication();
        virtual ~Authentication();

        /**
         * All the authentication's secrets are stored in this map.
         * These can be plaintext passwords, hashed passwords, certificate passphrases
         */
        void setSecrets(const SecretMap &);
        /**
         * retrieve the map containing secrets.
         */
        SecretMap secrets() const;

    private:
        class Private;
        Private * const d;
    };

    /**
     * This Authentication is a null authentication.  Used for open networks
     */
    class SOLIDCONTROL_EXPORT AuthenticationNone : public Authentication
    {
    public:
        AuthenticationNone();
        virtual ~AuthenticationNone();

    private:
        class Private;
        Private * const d;
    };

    /**
     * WEP (Wired Equivalent Privacy) Authentication.
     * Better than prayer for protecting your data, but not much.
     */
    class SOLIDCONTROL_EXPORT AuthenticationWep : public Authentication
    {
    public:
        /**
         * Wep password type.  WepAscii and WepPassphrase are both hashed to WepHex using
         * standard algorithms, but are easier to remember.
         */
        enum WepType { WepAscii, WepHex, WepPassphrase };
        /**
         * Authentication schemes
         * Open System has no authentication, if you have the encryption key, you are able to use the network
         * Shared Key means that the station must know a secret key to authenticate to the network.
         * Not sure if the same key is used for both Auth and Encryption though.
         */
        enum WepMethod { WepOpenSystem, WepSharedKey };

        AuthenticationWep();
        virtual ~AuthenticationWep();

        /**
         * Set the auth scheme in use
         */
        void setMethod(WepMethod);
        /**
         * Get the auth scheme in use
         */
        WepMethod method() const;
        /**
         * Set the password scheme in use
         */
        void setType(WepType);
        /**
         * Get the password scheme in use
         */
        WepType type() const;
        /**
         * Set the key length in bits
         * Valid values are 40 or 64 (equivalent)
         *                  104 or 128
         *                  192
         *                  256
         *                  other values (rare)
         */
        void setKeyLength(int);
        /**
         * Get the key length, in bits
         */
        int keyLength() const;

    private:
        class Private;
        Private * const d;
    };

    /**
     * AuthenticationWpa contains functionality shared by both Personal and Enterprise
     * authentication flavors
     */
    class SOLIDCONTROL_EXPORT AuthenticationWpa : public Authentication
    {
    public:
        /**
         * Possible Authentication schemes
         */
        enum WpaProtocol { WpaAuto, WpaTkip, WpaCcmpAes, // WPA Personal only
                           WpaEap /* WPA Enterprise only */ };
        /**
         * WPA Versions
         */
        enum WpaVersion { Wpa1, Wpa2 };

        /**
         * WPA key management schemes
         */
        enum WpaKeyManagement { WpaPsk, Wpa8021x };

        AuthenticationWpa();
        virtual ~AuthenticationWpa();

        /**
         * Set the protocol in use
         */
        void setProtocol(WpaProtocol);
        /**
         * Set the protocol in use
         */
        WpaProtocol protocol() const;

        /**
         * Set the WPA version
         */
        void setVersion(WpaVersion);
        /**
         * Get the WPA version
         */
        WpaVersion version() const;

        /**
         * Set the key management scheme
         */
        void setKeyManagement(WpaKeyManagement);

        /**
         * Get the key management scheme
         */
        WpaKeyManagement keyManagement() const;

    private:
        class Private;
        Private * const d;
    };

    /**
     * WPA Personal authentication.
     */
    class SOLIDCONTROL_EXPORT AuthenticationWpaPersonal : public AuthenticationWpa
    {
    public:
        AuthenticationWpaPersonal();
        virtual ~AuthenticationWpaPersonal();

    private:
        class Private;
        Private * const d;
    };

    /**
     * WPA Enterprise
     */
    class SOLIDCONTROL_EXPORT AuthenticationWpaEnterprise : public AuthenticationWpa
    {
    public:
        /**
         * Subtypes of Enterprise Authentication Protocol
         */
        enum EapMethod { EapPeap, EapTls, EapTtls, EapMd5, EapMsChap, EapOtp, EapGtc };
        AuthenticationWpaEnterprise();
        virtual ~AuthenticationWpaEnterprise();

        /**
         * TODO: check with thoenig what this means - probably identity off one of the certs
         */
        void setIdentity(const QString  &);
        /**
         * TODO: check with thoenig what this means - probably identity off one of the certs
         */
        QString identity() const;

        /**
         * TODO: check with thoenig what this means - probably identity off one of the certs
         */
        void setAnonIdentity(const QString  &);
        /**
         * TODO: check with thoenig what this means - probably identity off one of the certs
         */
        QString anonIdentity() const;

        /**
         * Set path to the client certificate
         */
        void setCertClient(const QString  &);
        /**
         * Get path to the client certificate
         */
        QString certClient() const;
        /**
         * Set path to the certification authority certificate
         */
        void setCertCA(const QString  &);
        /**
         * Get path to the certification authority certificate
         */
        QString certCA() const;

        /**
         * Set path to the private certificate
         */
        void setCertPrivate(const QString  &);
        /**
         * Get path to the private certificate
         */
        QString certPrivate() const;
        /**
         * Set the EAP method
         */
        void setMethod(EapMethod);
        /**
         * Get the EAP method
         */
        EapMethod method() const;
        /**
         * Set the ID password key (helper method)
         */
        void setIdPasswordKey(const QString  &);
        /**
         * Set the ID password key (helper method)
         */
        QString idPasswordKey() const;

        /**
         * Set the private certificate password key (helper method)
         */
        void setCertPrivatePasswordKey(const QString  &);
        /**
         * Get the private certificate password key (helper method)
         */
        QString certPrivatePasswordKey() const;

    private:
        class Private;
        Private * const d;
    };

    /**
     * Utility class
     * Contains a backend specific validator instance to validate authentication
     * Can be used for example to authenticate user input as they type
     */
    class SOLIDCONTROL_EXPORT AuthenticationValidator
    {
        public:
            AuthenticationValidator();
            virtual ~AuthenticationValidator();
            /**
             * Call this to check if an authentication is valid
             * (All secrets present, passphrase length correct
             */
            bool validate(const Authentication *);
        private:
            class Private;
            Private * const d;
    };
}
}

#endif
