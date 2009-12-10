/*
    Copyright (C) 2009  Rob Scheepmaker <r.scheepmaker@student.utwente.nl>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef CREDENTIALS_H
#define CREDENTIALS_H

#include "plasma.h"

#include <QtCore/QObject>
#include <QtCore/QDataStream>
#include <QtCore/QMetaType>

namespace Plasma {

class CredentialsPrivate;

/**
 * @class Credentials plasma/credentials.h <Plasma/Credentials>
 *
 * This class encapsules someone's identity.
 * It contains a unique id that identifies the machine an incoming connection is coming from, it's
 * name (which is not necesarily unique and/or trusted), a public key used to validate messages
 * coming from the machine with this identity, and in the future the possibility to determine
 * whether or not this identity can be trusted based on mechanisms different then pin pairing, e.g.
 * a signature of the key that can be verified by a gpg trusted key.
 */
class Credentials
{
public:
    /**
     * Default constructor.
     */
    Credentials();

    /**
     * Copy constructor.
     */
    Credentials(const Credentials &other);

    ~Credentials();

    Credentials &operator=(const Credentials &other);

    /**
     * Create a new identity with a new set of random public/private keys.
     */
    static Credentials createCredentials(const QString &name);

    /**
     * @return whether or not this identity can be trusted based on e.g. having the key signed with
     * a trusted GPG key (not yet implemented) or having the key in a designated folder on disk
     * (about to be impl.). If this function returns false, your shell should always instatiate
     * pin pairing before allowing a connection from an untrusted source
     * (AuthorizationRule::PinRequired flag should be set on the rule with setRules).
     */
    TrustLevel trustLevel() const;

    /**
     * @return whether or not this is a null identity or an invalid one (hash of key doesn't match
     * id). Maybe isValid() is a better name?
     */
    bool isValid() const;

    /**
     * @return the name of this identity. There's however no guarantee that if the name returns e.g.
     * "Santa Claus", this message is actually from Mr. Claus, except if trustLevel returns a
     * sufficiently high trust level.
     */
    QString name() const;

    /**
     * @return an id to identify this identity. I use a Hash of the public key as ID. This way we
     * don't have to send the complete public key with every message.
     */
    QString id() const;

    /**
     * @return whether or not @p signature is correct for @p message.
     */
    bool isValidSignature(const QByteArray &signature, const QByteArray &message);

    /**
     * @return whether or not this identity can be used for signing a message (whether or not it
     * includes a public key)
     */
    bool canSign() const;

    /**
     * @return the signature for the message.
     */
    QByteArray signMessage(const QByteArray &message);

    /**
     * @return a Credentials stripped from any private key, so you can be sure it is save to send to
     * somebody.
     */
    Credentials toPublicCredentials() const;

    friend QDataStream &operator<<(QDataStream &, const Credentials &);
    friend QDataStream &operator>>(QDataStream &, Credentials &);

private:
    Credentials(const QString &id, const QString &name, const QString &key,
             bool privateKey = false);

    CredentialsPrivate *const d;

    friend class AuthorizationManagerPrivate;
    friend class CredentialsPrivate;
};

/**
 * Streaming operators for sending/storing identities.
 */
QDataStream &operator<<(QDataStream &, const Credentials &);
QDataStream &operator>>(QDataStream &, Credentials &);

}

#endif // IDENTITY_H
