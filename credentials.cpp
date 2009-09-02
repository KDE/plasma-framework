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

#include "credentials.h"

#include <QObject>
#include <QtCrypto/QtCrypto>

#include <KDebug>
#include "authorizationmanager.h"
#include <kstandarddirs.h>

namespace Plasma {

class CredentialsPrivate {
public:
    CredentialsPrivate()
    {
    }

    CredentialsPrivate(const QString &id, const QString &name,
                    const QString &pemKey, bool isPrivateKey)
        : id(id),
          name(name)
    {
        if (isPrivateKey) {
            privateKey = QCA::PrivateKey::fromPEM(pemKey);
            publicKey = privateKey.toPublicKey();
        } else {
            publicKey = QCA::PublicKey::fromPEM(pemKey);
        }
    }

    ~CredentialsPrivate()
    {
    }

    QString id;
    QString name;
    QCA::PublicKey publicKey;
    QCA::PrivateKey privateKey;
};

Credentials::Credentials(const QString &id, const QString &name,
                   const QString &key, bool isPrivateKey)
         : d(new CredentialsPrivate(id, name, key, isPrivateKey))
{
}

Credentials::Credentials()
         : d(new CredentialsPrivate())
{
}

Credentials::Credentials(const Credentials &other)
        : d(new CredentialsPrivate())
{
    *d = *other.d;
}

Credentials::~Credentials()
{
    delete d;
}

Credentials &Credentials::operator=(const Credentials &other)
{
    *d = *other.d;
    return *this;
}

Credentials Credentials::createCredentials(const QString &name)
{
    QCA::KeyGenerator generator;
    QCA::PrivateKey key = generator.createRSA(2048);
    QString pemKey(key.toPublicKey().toPEM());
    //TODO: is using a md5 hash for the id a good idea?
    QString id = QCA::Hash("sha1").hashToString(pemKey.toAscii());
    return Credentials(id, name, key.toPEM(), true);
}

TrustLevel Credentials::trustLevel() const
{
    /**
    QString pemFile = KStandardDirs::locate("trustedkeys", id());

    if (!pemFile.isEmpty()) {
        QCA::PublicKey pubKey = QCA::PublicKey::fromPEMFile(pemFile);
        if (pubKey == d->publicKey) {
            return true;
        }
    }
    */
    //Trust noone ;)
    return ValidCredentials;
}

bool Credentials::isValid() const
{
    if (d->publicKey.isNull()) {
        return false;
    } else {
        QString id = QCA::Hash("sha1").hashToString(d->publicKey.toPEM().toAscii());
        return (id == d->id);
    }
}

QString Credentials::name() const
{
    return d->name;
}

QString Credentials::id() const
{
    return d->id;
}

bool Credentials::isValidSignature(const QByteArray &signature, const QByteArray &payload) 
{
    if (d->publicKey.canVerify()) {
        if (!isValid()) {
            kDebug() << "Key is null?";
        }
        QCA::PublicKey publicKey = QCA::PublicKey::fromPEM(d->publicKey.toPEM());
        publicKey.startVerify( QCA::EMSA3_MD5 );
        publicKey.update(payload);
        return ( publicKey.validSignature( signature ) );
    } else {
        kDebug() << "Can't verify?";
        return false;
    }
}

bool Credentials::canSign() const
{
    return d->privateKey.canSign();
}

QByteArray Credentials::signMessage(const QByteArray &message)
{
    if(!QCA::isSupported("pkey") ||
       !QCA::PKey::supportedIOTypes().contains(QCA::PKey::RSA)) {
        kDebug() << "RSA not supported";
        return QByteArray();
    } else if (canSign()) {
        //QCA::PrivateKey privateKey = QCA::PrivateKey::fromPEM(d->privateKey.toPEM());
        d->privateKey.startSign( QCA::EMSA3_MD5 );
        d->privateKey.update( message );
        QByteArray signature = d->privateKey.signature();
        return signature;
    } else {
        kDebug() << "can't sign?";
        return QByteArray();
    }
}

Credentials Credentials::toPublicCredentials() const
{
    kDebug();
    Credentials result(*this);
    result.d->privateKey = QCA::PrivateKey();
    return result;
}

QDataStream &operator<<(QDataStream &out, const Credentials &myObj)
{
    QString privateKeyPem;
    QString publicKeyPem;

    if (!myObj.d->privateKey.isNull()) {
        privateKeyPem = myObj.d->privateKey.toPEM();
    }
    if (!myObj.d->publicKey.isNull()) {
        publicKeyPem = myObj.d->publicKey.toPEM();
    }

    out << 1 << myObj.d->id << myObj.d->name << privateKeyPem << publicKeyPem;

    return out;
}

QDataStream &operator>>(QDataStream &in, Credentials &myObj)
{
    QString privateKeyString;
    QString publicKeyString;
    uint version;

    in >> version >> myObj.d->id >> myObj.d->name >> privateKeyString >> publicKeyString;
    QCA::ConvertResult conversionResult;

    if (!privateKeyString.isEmpty()) {
        myObj.d->privateKey = QCA::PrivateKey::fromPEM(privateKeyString,
                                                    QByteArray(), &conversionResult);
    }
    if (!publicKeyString.isEmpty()) {
        myObj.d->publicKey = QCA::PublicKey::fromPEM(publicKeyString, &conversionResult);
    }

    if (conversionResult != QCA::ConvertGood) {
        kDebug() << "Unsuccessfull conversion of key?";
    }

    return in;
}

}
