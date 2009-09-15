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

#include "pendingreply.h"

#include "pendingcall_p.h"

using namespace Jolie;

PendingReply::PendingReply()
    : PendingCall(QExplicitlySharedDataPointer<PendingCallPrivate>())
{
}

PendingReply::PendingReply(const PendingReply &other)
    : PendingCall(other.d)
{
}

PendingReply::PendingReply(const PendingCall &call)
    : PendingCall(call.d)
{
}

PendingReply::~PendingReply()
{
}

PendingReply &PendingReply::operator=(const PendingReply &other)
{
    d = other.d;

    return *this;
}

PendingReply &PendingReply::operator=(const PendingCall &call)
{
    d = call.d;

    return *this;
}

bool PendingReply::isFinished() const
{
    return d ? d->isFinished : false;
}

Message PendingReply::reply() const
{
    return d ? d->reply : Message();
}

void PendingReply::waitForFinished()
{
    PendingCallWaiter waiter;
    waiter.waitForFinished(d.data());
}

