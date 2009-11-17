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

#include "remoteservicejob_p.h"

#include <kconfiggroup.h>
#include <kurl.h>

#include <QtCore/QBuffer>
#include <QtJolie/Client>
#include <QtJolie/Message>
#include <QtJolie/PendingCallWatcher>
#include <QtJolie/PendingReply>

#include "../servicejob.h"
#include "../remote/accessmanager.h"
#include "../remote/authorizationmanager.h"
#include "authorizationmanager_p.h"
#include "remoteservice_p.h"
#include "servicejob_p.h"
#include "joliemessagehelper_p.h"
#include <qtimer.h>

namespace Plasma
{

RemoteServiceJob::RemoteServiceJob(KUrl location,
                  const QString& destination,
                  const QString& operation,
                  QMap<QString,QVariant>& parameters,
                  QByteArray initialToken,
                  RemoteService* parent)
                : ServiceJob(destination, operation, parameters, parent),
                  m_token(initialToken),
                  m_location(location),
                  m_service(parent),
                  m_delayedDesc(0)
{
}

RemoteServiceJob::~RemoteServiceJob()
{
    delete m_delayedDesc;
    m_delayedDesc = 0;
}

void RemoteServiceJob::start()
{
    QTimer::singleShot(30000, this, SLOT(timeout()));

    if (m_service->m_busy || !m_service->m_ready) {
        //enqueue and wait
        m_service->m_queue.enqueue(this);
        kDebug() << "already busy... enqueue, queue contains " << m_service->m_queue.count();
        return;
    }

    // the service is now busy ... with us!
    m_service->m_busy = true;

    // while waiting in the queue, our validity may have changed; it's all async
    // so don't assume anything
    checkValidity();
    if (error()) {
        emitResult();
        return;
    }

    //serialize the parameters
    QByteArray params;
    QBuffer buffer(&params);
    buffer.open(QIODevice::WriteOnly);
    QDataStream out(&buffer);
    out << parameters();

    Jolie::Message message(m_location.path(KUrl::RemoveTrailingSlash).remove(0, 1).toUtf8(),
                           "startOperationCall");
    Jolie::Value data;
    data.children(JolieMessage::Field::OPERATION) << (Jolie::Value(operationName().toAscii()));
    data.children(JolieMessage::Field::PARAMETERS) << Jolie::Value(params);
    data.children(JolieMessage::Field::DESTINATION) << Jolie::Value(destination().toAscii());
    message.setData(data);

    Jolie::Client *client = m_service->m_client;
    Jolie::PendingCall pendingReply = client->asyncCall(m_service->signMessage(message));
    Jolie::PendingCallWatcher *watcher = new Jolie::PendingCallWatcher(pendingReply, this);
    connect(watcher, SIGNAL(finished(Jolie::PendingCallWatcher*)),
            this, SLOT(callCompleted(Jolie::PendingCallWatcher*)));
}

void RemoteServiceJob::setDelayedDescription(const KConfigGroup &desc)
{
    if (!m_delayedDesc) {
        m_delayedDesc = new KConfigGroup(desc);
    } else {
        *m_delayedDesc = desc;
    }
}

void RemoteServiceJob::checkValidity()
{
    if (!m_service->isOperationEnabled(operationName())) {
        setError(-1);
        setErrorText(i18n("Job no longer valid, operation is not enabled."));
    } else if (m_delayedDesc) {
        d->parameters = m_service->parametersFromDescription(*m_delayedDesc);
    } else {
        KConfigGroup description = m_service->operationDescription(operationName());
        QMapIterator<QString, QVariant> param(parameters());
        while (param.hasNext()) {
            param.next();
            if (!description.hasKey(param.key())) {
                setError(-1);
                setErrorText(i18n("Job no longer valid, invalid parameters."));
                break;
            }
        }
    }

    delete m_delayedDesc;
    m_delayedDesc = 0;
}

void RemoteServiceJob::callCompleted(Jolie::PendingCallWatcher *watcher)
{
    m_service->m_busy = false;

    Jolie::PendingReply reply = *watcher;
    Jolie::Message response = reply.reply();

    //TODO:async
    if (response.fault().isValid()) {
        kDebug() << "fault: " << response.fault().name();
        setError(-1);
        setErrorText(JolieMessage::errorMessage(response.fault().name()));
        emitResult();
        return;
    }

    m_service->m_token = JolieMessage::field(JolieMessage::Field::TOKEN, response);

    QVariant variantResult;
    QByteArray byteArrayResult = JolieMessage::field(JolieMessage::Field::RESULT, response);
    QBuffer buffer(&byteArrayResult);
    buffer.open(QIODevice::ReadOnly);
    QDataStream in(&buffer);
    in >> variantResult;

    setResult(variantResult);
}

void RemoteServiceJob::timeout()
{
    m_service->m_busy = false;
    kDebug() << "Service job timed out.";
    setError(-1);
    setErrorText(i18n("Timeout."));
    m_service->m_queue.removeAll(this);
    emitResult();
}

} // namespace Plasma
