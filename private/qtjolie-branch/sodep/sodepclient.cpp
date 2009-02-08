/**
  * This file is part of the KDE project
  * Copyright (C) 2009 Kevin Ottens <ervin@kde.org>
  *
  * This library is free software; you can redistribute it and/or
  * modify it under the terms of the GNU Library General Public
  * License as published by the Free Software Foundation; either
  * version 2 of the License, or (at your option) any later version.
  *
  * This library is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  * Library General Public License for more details.
  *
  * You should have received a copy of the GNU Library General Public License
  * along with this library; see the file COPYING.LIB.  If not, write to
  * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  * Boston, MA 02110-1301, USA.
  */

#include "sodepclient.h"

#include <QtCore/QHash>
#include <QtCore/QIODevice>
#include <QtCore/QTimer>

#include "sodepmessage.h"
#include "sodephelpers_p.h"
#include "sodepclientthread_p.h"

struct SodepRequest
{
    enum { Send, Receive } type;
    SodepMessage message;
};

class SodepClientPrivate
{
public:
    SodepClientPrivate(SodepClient *client)
        : q(client), device(0),
          error(SodepClient::NoError),
          lastAllocatedRequestId(0),
          currentRequestId(-1) {}

    void startRequest(int id);
    void emitCurrentRequestFinished();

    void _k_messageLoaded(const SodepMessage &message);
    void _k_bytesWritten();

    SodepClient * const q;

    QIODevice *device;
    SodepClientThread *readerThread;

    SodepClient::Error error;
    QString errorString;

    int lastAllocatedRequestId;
    int currentRequestId;
    QHash<int, SodepRequest> queuedRequests;
};

SodepClient::SodepClient(QIODevice *device)
    : d(new SodepClientPrivate(this))
{
    d->device = device;
    d->readerThread = new SodepClientThread(device, this);
}

SodepClient::~SodepClient()
{
    delete d;
}

int SodepClient::requestMessage()
{
    int id = ++d->lastAllocatedRequestId;

    SodepRequest request;
    request.type = SodepRequest::Receive;
    d->queuedRequests[id] = request;

    if (d->currentRequestId==-1) {
        d->startRequest(id);
    }

    return id;
}

int SodepClient::postMessage(const SodepMessage &message)
{
    int id = ++d->lastAllocatedRequestId;

    SodepRequest request;
    request.type = SodepRequest::Send;
    request.message = message;
    d->queuedRequests[id] = request;

    if (d->currentRequestId==-1) {
        d->startRequest(id);
    }

    return id;
}

SodepClient::Error SodepClient::error() const
{
    return d->error;
}

QString SodepClient::errorString() const
{
    return d->errorString;
}

void SodepClientPrivate::startRequest(int id)
{
    currentRequestId = id;

    SodepRequest &request = queuedRequests[id];

    error = SodepClient::NoError;
    errorString = QString();

    emit q->requestStarted(id);

    if (request.type == SodepRequest::Receive) {
        QObject::connect(readerThread, SIGNAL(messageLoaded(const SodepMessage&)),
                         q, SLOT(_k_messageLoaded(const SodepMessage&)));
        readerThread->requestMessage();
    } else {
        QObject::connect(device, SIGNAL(bytesWritten(qint64)),
                         q, SLOT(_k_bytesWritten()));
        SodepMessage &message = request.message;
        sodepWrite(*device, message);
    }
}

void SodepClientPrivate::emitCurrentRequestFinished()
{
    SodepRequest request = queuedRequests.take(currentRequestId);

    if (request.type == SodepRequest::Receive) {
        QObject::disconnect(readerThread, SIGNAL(messageLoaded(const SodepMessage&)),
                            q, SLOT(_k_messageLoaded(const SodepMessage&)));
    } else {
        QObject::disconnect(device, SIGNAL(bytesWritten(qint64)),
                            q, SLOT(_k_bytesWritten()));
    }

    emit q->requestFinished(currentRequestId, request.message, false);

    if (currentRequestId<lastAllocatedRequestId) {
        startRequest(currentRequestId+1);
    } else {
        currentRequestId = -1;
    }
}

void SodepClientPrivate::_k_messageLoaded(const SodepMessage &message)
{
    queuedRequests[currentRequestId].message = message;
    emitCurrentRequestFinished();
}

void SodepClientPrivate::_k_bytesWritten()
{
    if (device->bytesToWrite()>0) {
        return;
    }

    emitCurrentRequestFinished();
}

#include "sodepclient.moc"
