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
#include "sodepclient_p.h"

#include "sodepclientthread_p.h"
#include "sodepmessage.h"
#include "sodeppendingcall.h"


SodepClient::SodepClient(const QString &hostName, quint16 port)
    : d(new SodepClientPrivate(this))
{
    d->readerThread = new SodepClientThread(hostName, port, d);
    d->readerThread->start();
}

SodepClient::~SodepClient()
{
    delete d->readerThread;
    delete d;
}

SodepClient::Error SodepClient::error() const
{
    return d->error;
}

QString SodepClient::errorString() const
{
    return d->errorString;
}

SodepPendingCall SodepClient::asyncCall(const SodepMessage &message)
{
    Q_ASSERT(!d->pendingCalls.contains(message.id()));
    d->pendingCalls[message.id()] = new SodepPendingCallPrivate(message);
    d->readerThread->sendMessage(message);
    return SodepPendingCall(d->pendingCalls[message.id()]);
}

SodepMessage SodepClient::call(const SodepMessage &message)
{
    SodepPendingCall pending = asyncCall(message);
    pending.waitForFinished();
    return pending.reply();
}

void SodepClientPrivate::messageReceived(const SodepMessage &message)
{
    QExplicitlySharedDataPointer<SodepPendingCallPrivate> pending = pendingCalls.take(message.id());
    pending->setReply(message);
}
