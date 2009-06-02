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

#include "pendingcall.h"
#include "pendingcall_p.h"
#include "pendingcallwatcher.h"

using namespace Jolie;

PendingCall::PendingCall(const PendingCall &other)
    : d(other.d)
{
}

PendingCall::PendingCall(QExplicitlySharedDataPointer<PendingCallPrivate> dd)
    : d(dd)
{
}

PendingCall::~PendingCall()
{
}

PendingCall &PendingCall::operator=(const PendingCall &other)
{
    d = other.d;

    return *this;
}

void PendingCallPrivate::setReply(const Message &message)
{
    Q_ASSERT(message.id()==id);
    isFinished = true;
    reply = message;

    foreach (PendingCallWaiter *waiter, waiters) {
        waiter->eventLoop.quit();
    }
    waiters.clear();

    foreach (PendingCallWatcher *watcher, watchers) {
        emit watcher->finished(watcher);
    }
    watchers.clear();
}

void PendingCallWaiter::waitForFinished(PendingCallPrivate *pendingCall)
{
    if (pendingCall==0) return;
    pendingCall->waiters << this;
    eventLoop.exec();
}

