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

#include "publickeyfetcher.h"
#include "authorizationmanager_p.h"
#include "joliemessagehelper_p.h"
#include "../authorizationmanager.h"
#include "../accessmanager.h"
#include "../servicejob.h"
#include "../serviceaccessjob.h"

#include <QtCore/QString>
#include <KDebug>
#include <KConfigGroup>

namespace Plasma
{

PublicKeyFetcher::PublicKeyFetcher(const QString &sender)
    : QObject(AuthorizationManager::self()),
      m_sender(sender)
{
    connect(AccessManager::self(), SIGNAL(serviceAccessFinished(Plasma::ServiceAccessJob*)),
            this, SLOT(slotKeyServiceReady(Plasma::ServiceAccessJob*)));
    AccessManager::self()->accessService(KUrl(sender));
}

PublicKeyFetcher::~PublicKeyFetcher()
{
}

QList<Jolie::Message> & PublicKeyFetcher::pendingMessages()
{
    return m_messageList;
}

void PublicKeyFetcher::slotKeyServiceReady(Plasma::ServiceAccessJob *job)
{
    if (job->service()) {
        KConfigGroup op = job->service()->operationDescription("GetPublicKey");
        job->service()->startOperationCall(op);
        connect(job->service(), SIGNAL(finished(Plasma::ServiceJob*)),
                this, SLOT(slotKeyObtained(Plasma::ServiceJob*)));
    }
}

void PublicKeyFetcher::slotKeyObtained(Plasma::ServiceJob *job)
{
    QByteArray reply = job->result().toByteArray();
    QCA::PublicKey key = QCA::PublicKey::fromPEM(reply);
    AuthorizationManager::self()->d->publicKeys[m_sender] = key;
    
    KConfig c("plasmapublickeys");
    c.group(m_sender).writeEntry("PublicKey", key.toPEM());

    kDebug() << "Obtained public key from: " << m_sender << ", public key: " << reply;
    
    foreach (Jolie::Message message, m_messageList) {
        kDebug() << "process pending message: " << printJolieMessage(message);
        AuthorizationManager::self()->d->startAuthorization(message, "");
    }
}

} // Plasma namespace

#include "publickeyfetcher.moc"
