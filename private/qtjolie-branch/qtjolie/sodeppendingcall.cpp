/**
  * This file is part of the KDE project
  * Copyright (C) 2008 Kevin Ottens <ervin@kde.org>
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

#include "sodeppendingcall.h"
#include "sodeppendingcall_p.h"


SodepPendingCall::SodepPendingCall(const SodepPendingCall &other)
    : d(other.d)
{
}

SodepPendingCall::SodepPendingCall(QExplicitlySharedDataPointer<SodepPendingCallPrivate> dd)
    : d(dd)
{
}

SodepPendingCall::~SodepPendingCall()
{
}

SodepPendingCall &SodepPendingCall::operator=(const SodepPendingCall &other)
{
    d = other.d;

    return *this;
}

bool SodepPendingCall::isFinished() const
{
    return d->isFinished;
}

SodepMessage SodepPendingCall::reply() const
{
    return d->reply;
}

void SodepPendingCall::waitForFinished()
{
    SodepPendingCallWaiter waiter;
    waiter.waitForFinished(d.data());
}

void SodepPendingCallPrivate::setReply(const SodepMessage &message)
{
    Q_ASSERT(message.id()==id);
    isFinished = true;
    reply = message;

    foreach (SodepPendingCallWaiter *waiter, waiters) {
        waiter->eventLoop.quit();
    }

    waiters.clear();
}

void SodepPendingCallWaiter::waitForFinished(SodepPendingCallPrivate *pendingCall)
{
    pendingCall->waiters << this;
    eventLoop.exec();
}
