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

#include "accessmanager.h"

#include "service.h"
#include "serviceaccessjob.h"

#include <QtCore/QMap>
#include <QtCore/QTimer>

#include <QtCrypto>
#include <QtJolie/Message>

#include <KUrl>

namespace Plasma
{

class AccessManagerPrivate
{
    public:
        AccessManagerPrivate() 
        {
            //TODO: store at a sensible place... kwallet keyring for private key maybe?
            privateKey = QCA::PrivateKey::fromPemFile("/home/rob/plasma_private_key.pem");
            publicKey = QCA::PrivateKey::fromPemFile("/home/rob/plasma_public_key.pem");
            if (privateKey.isNull()) {
                //generate public/private key pair
                QCA::KeyGenerator generator;
                privateKey = generator.createRSA(2048);
                publicKey = privateKey.toPublicKey();
                privateKey.toPEMFile("/home/rob/plasma_private_key.pem");
                publicKey.toPEMFile("/home/rob/plasma_public_key.pem");
            }
        };

        ~AccessManagerPrivate() {};

        //TODO: is this bool based authorisation management detailed enough?
        QMap<KUrl caller, QMap<Service *, bool> >;
        //Needed for QCA support
        QCA::Initializer initializer;
        QCA:PrivateKey privateKey;
        QCA:PrivateKey publicKey;
};

class AccessManagerSingleton
{
    public:
        AccessManager self;
};

K_GLOBAL_STATIC(AccessManagerSingleton, privateAccessManagerSelf)

AccessManager *AccessManager::self()
{
    return &privateAccessManagerSelf->self;
}

AccessManager::AccessManager()
    : d(new AccessManagerPrivate)
{
}

AccessManager::~AccessManager()
{
    delete d;
}

Jolie::Message AccessManager::signMessage(Jolie::Message message) const
{
    //TODO:what about multiple interfaces?
    QString host = QNetworkInterface::allAddresses().first();
    message.children("host") << Jolie::Value(host);
    QByteArray serializedMessage = //serialize sodep message
    QByteArray signature = d->privateKey->signMessage(MemoryRegion(serializedMessage));
    message.children("signature") << Jolie::Value(signature);
    return message;
}

ServiceAccessJob* AccessManager::accessService(KUrl location) const
{
    ServiceAccesJob *job = new ServiceAccessJob(location);
    QTimer::singleShot(0, job, SLOT(slotStart()));
    return job;
}

ServiceAccessJob* AccessManager::accessService(const QString &jolieScript, 
                                               const QMap<QString, QVariant> &initValues) const
{
    ServiceAccesJob *job = new ServiceAccessJob(jolieScript, initValues);
    QTimer::singleShot(0, job, SLOT(slotStart()));
    return job;
}

} // Plasma namespace

#include "accessmanager.moc"
