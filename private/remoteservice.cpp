/*
 *   Copyright © 2009 Rob Scheepmaker <r.scheepmaker@student.utwente.nl>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License version 2 as
 *   published by the Free Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "remoteservice_p.h"

#include "../remote/accessmanager.h"
#include "../remote/authorizationinterface.h"
#include "../remote/authorizationmanager.h"
#include "../remote/clientpinrequest.h"
#include "../service.h"

#include "authorizationmanager_p.h"
#include "joliemessagehelper_p.h"
#include "remoteservicejob_p.h"
#include "service_p.h"

#include <QtCore/QBuffer>
#include <QtCore/QObject>
#include <QtCore/QTimer>
#include <QtCore/QUuid>

#include <QtJolie/Client>
#include <QtJolie/Message>
#include <QtJolie/PendingReply>
#include <QtJolie/PendingCall>
#include <QtJolie/PendingCallWatcher>
#include <QtJolie/Value>

#include <kdebug.h>
#include <kurl.h>

namespace Plasma 
{

RemoteService::RemoteService(QObject* parent)
    : Service(parent),
      m_client(0),
      m_ready(false),
      m_busy(false)
{
}

RemoteService::RemoteService(QObject* parent, KUrl location)
    : Service(parent),
      m_location(location),
      m_client(0),
      m_ready(false),
      m_busy(false)
{
    if (AuthorizationManager::self()->d->myCredentials.isValid()) {
        setLocation(location);
    } else {
        connect(AuthorizationManager::self(), SIGNAL(readyForRemoteAccess()),
                this, SLOT(slotReadyForRemoteAccess()));
    }
}

RemoteService::~RemoteService()
{
    delete m_client;
}

void RemoteService::slotReadyForRemoteAccess()
{
    kDebug() << "AuthorizationManager is now ready for remote access!";
    setLocation(m_location);
}

void RemoteService::setLocation(const KUrl &location)
{
    kDebug() << "Setting RemoteService location to " << location.prettyUrl();

    m_uuid = QUuid::createUuid().toString();
    Credentials identity = AuthorizationManager::self()->d->myCredentials;
    if (!identity.canSign()) {
        kDebug() << "we can't sign? how did we get here?";
        return;
    }

    if (m_client && (m_location != location)) {
        delete m_client;
        m_client = new Jolie::Client(location.host(), location.port());
    }

    if (!m_client) {
        m_client = new Jolie::Client(location.host(), location.port());
    }

    m_location = location;

    QByteArray identityByteArray;
    QDataStream stream(&identityByteArray, QIODevice::WriteOnly);
    stream << identity.toPublicCredentials();

    Jolie::Message getOpDesc(location.path(KUrl::RemoveTrailingSlash).remove(0, 1).toUtf8(),
                             "startConnection");
    Jolie::Value data;
    data.children(JolieMessage::Field::IDENTITY) << Jolie::Value(identityByteArray);
    data.children(JolieMessage::Field::UUID) << Jolie::Value(m_uuid.toAscii());
    getOpDesc.setData(data);

    Jolie::PendingCall pendingReply = m_client->asyncCall(getOpDesc);
    Jolie::PendingCallWatcher *watcher = new Jolie::PendingCallWatcher(pendingReply, this);
    connect(watcher, SIGNAL(finished(Jolie::PendingCallWatcher*)),
            this, SLOT(callCompleted(Jolie::PendingCallWatcher*)));
}

QString RemoteService::location() const
{
    return m_location.prettyUrl();
}

bool RemoteService::isReady() const
{
    return m_ready;
}

bool RemoteService::isBusy() const
{
    return m_busy;
}

void RemoteService::callCompleted(Jolie::PendingCallWatcher *watcher)
{
    Jolie::PendingReply reply = *watcher;
    Jolie::Message response = reply.reply();

    if (response.operationName() == "startConnection") {
        kDebug() << "Started connection: fetching .operations";
        m_token = JolieMessage::field(JolieMessage::Field::TOKEN, response);
        Jolie::Message getOpDesc(m_location.path(KUrl::RemoveTrailingSlash).remove(0, 1).toUtf8(),
                                 "getOperations");
        //TODO: async
        Jolie::PendingCall pendingReply = m_client->asyncCall(signMessage(getOpDesc));
        Jolie::PendingCallWatcher *watcher = new Jolie::PendingCallWatcher(pendingReply, this);
        connect(watcher, SIGNAL(finished(Jolie::PendingCallWatcher*)),
                this, SLOT(callCompleted(Jolie::PendingCallWatcher*)));
    } else if (response.operationName() == "getOperations") {
        if (response.fault().name() == JolieMessage::Error::REQUIREPIN) {
            kDebug() << "pin required, request auth interface";
            ClientPinRequest *request = new ClientPinRequest(this);
            connect(request, SIGNAL(changed(Plasma::ClientPinRequest*)),
                    this, SLOT(slotGotPin(Plasma::ClientPinRequest*)));
            AuthorizationManager::self()->d->authorizationInterface->clientPinRequest(*request);
        } else {
            kDebug() << "RemoteService is now ready for use!";
            m_operationsScheme = JolieMessage::field(JolieMessage::Field::OPERATIONSDESCRIPTION, response);
            m_token = JolieMessage::field(JolieMessage::Field::TOKEN, response);
            m_ready = true;
            setName(m_location.prettyUrl());
            //if there's stuff in the queue, let it continue.
            slotFinished();
        }
    } else if (response.operationName() == "getEnabledOperations") {
        //TODO: optimize.
        m_token = JolieMessage::field(JolieMessage::Field::TOKEN, response);
        QByteArray enabledOperations = JolieMessage::field(JolieMessage::Field::ENABLEDOPERATIONS, response);
        QDataStream in(&enabledOperations, QIODevice::ReadOnly);
        QStringList enabledOperationsList;
        in >> enabledOperationsList;

        foreach (const QString &operation, operationNames()) {
            if (enabledOperationsList.contains(operation) && !isOperationEnabled(operation)) {
                kDebug() << "yeah, we're enabling the operation with the name " << operation;
                setOperationEnabled(operation, true);
            } else if (!enabledOperationsList.contains(operation) && isOperationEnabled(operation)) {
                kDebug() << "we're disabling the operation with the name " << operation;
                setOperationEnabled(operation, false);
            }
        }

        //if there's stuff in the queue, let it continue.
        m_busy = false;
        slotFinished();
    } else {
        kDebug() << "How did we end up here?";
    }
}

void RemoteService::slotGotPin(Plasma::ClientPinRequest *request)
{
    Jolie::Message getOpDesc(m_location.path(KUrl::RemoveTrailingSlash).remove(0, 1).toUtf8(),
                             "getOperations");
    Jolie::Value value;
    value.children(JolieMessage::Field::PARAMETERS) << Jolie::Value(QByteArray());
    if (!request->pin().isEmpty()) {
        value.children(JolieMessage::Field::PIN) << Jolie::Value(request->pin().toAscii());
    }
    getOpDesc.setData(value);
    //TODO: async
    Jolie::PendingCall pendingReply = m_client->asyncCall(signMessage(getOpDesc));
    Jolie::PendingCallWatcher *watcher = new Jolie::PendingCallWatcher(pendingReply, this);
    connect(watcher, SIGNAL(finished(Jolie::PendingCallWatcher*)),
            this, SLOT(callCompleted(Jolie::PendingCallWatcher*)));
}

void RemoteService::registerOperationsScheme()
{
    QBuffer buffer(&m_operationsScheme);
    buffer.open(QBuffer::ReadWrite);
    setOperationsScheme(&buffer);
    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(slotUpdateEnabledOperations()));
    //FIXME: get some sensible interval depending on the connection speed. This is kind of stupid.
    timer->start(2000);
}

void RemoteService::slotUpdateEnabledOperations()
{
    //TODO: maybe push the get enabled operations also on the queue?
    if (!m_busy) {
        m_busy = true;
        Jolie::Message getOpDesc(m_location.path(KUrl::RemoveTrailingSlash).remove(0, 1).toUtf8(),
                                 "getEnabledOperations");

        Jolie::PendingCall pendingReply = m_client->asyncCall(signMessage(getOpDesc));
        Jolie::PendingCallWatcher *watcher = new Jolie::PendingCallWatcher(pendingReply, this);
        connect(watcher, SIGNAL(finished(Jolie::PendingCallWatcher*)),
                this, SLOT(callCompleted(Jolie::PendingCallWatcher*)));
    } else {
        kDebug() << "We would like to update enabled operations, but are still busy so let's wait for now.";
    }
}

ServiceJob* RemoteService::createJob(const QString& operation,
                                     QMap<QString,QVariant>& parameters)
{
    if (!m_ready) {
        kDebug() << "Use of this service hasn't checked for the serviceReady signal, which it should.";
    }

    ServiceJob *job = new RemoteServiceJob(m_location, destination(), operation, parameters, m_token, this);
    connect(job, SIGNAL(finished(KJob *)), this, SLOT(slotFinished()));
    return job;
}

void RemoteService::slotFinished()
{
    if (!m_queue.isEmpty()) {
        kDebug() << "Job finished, there are still service jobs in queue, starting next in queue.";
        ServiceJob *job = m_queue.dequeue();
        QTimer::singleShot(0, job, SLOT(slotStart()));
    }
}

Jolie::Message RemoteService::signMessage(const Jolie::Message &message) const
{
    Jolie::Message response(message);

    Credentials identity = AuthorizationManager::self()->d->myCredentials;
    if (!identity.isValid()) {
        kDebug() << "We don't have our identity yet, just drop this message";
        return response;
    }

    Jolie::Value data = response.data();
    data.children(JolieMessage::Field::IDENTITYID) << Jolie::Value(identity.id().toAscii());
    data.children(JolieMessage::Field::TOKEN) << Jolie::Value(m_token);
    data.children(JolieMessage::Field::UUID) << Jolie::Value(m_uuid.toAscii());
    response.setData(data);
    data.children(JolieMessage::Field::SIGNATURE) <<
                Jolie::Value(identity.signMessage(JolieMessage::payload(response)));
    response.setData(data);
    return response;
}

} //namespace Plasma

#include "remoteservice_p.moc"
