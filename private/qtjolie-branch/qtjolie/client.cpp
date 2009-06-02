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

#include "client.h"
#include "client_p.h"

#include "clientthread_p.h"
#include "message.h"
#include "pendingcall.h"
#include "pendingreply.h"

using namespace Jolie;

Client::Client(const QString &hostName, quint16 port)
    : d(new ClientPrivate(this))
{
    d->readerThread = new ClientThread(hostName, port, d);
    d->readerThread->start();
}

Client::~Client()
{
    delete d->readerThread;
    delete d;
}

Client::Error Client::error() const
{
    return d->error;
}

QString Client::errorString() const
{
    return d->errorString;
}

PendingCall Client::asyncCall(const Message &message)
{
    Q_ASSERT(!d->pendingCalls.contains(message.id()));
    d->pendingCalls[message.id()] = new PendingCallPrivate(message);
    d->readerThread->sendMessage(message);
    return PendingCall(d->pendingCalls[message.id()]);
}

Message Client::call(const Message &message)
{
    PendingReply pending = asyncCall(message);
    pending.waitForFinished();
    return pending.reply();
}

void Client::callNoReply(const Message &message)
{
    d->readerThread->sendMessage(message);
}

void ClientPrivate::messageReceived(const Message &message)
{
    QExplicitlySharedDataPointer<PendingCallPrivate> pending = pendingCalls.take(message.id());
    pending->setReply(message);
}

