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

#ifndef QTJOLIE_PENDINGCALL_P_H
#define QTJOLIE_PENDINGCALL_P_H

#include <QtCore/QEventLoop>
#include <QtCore/QObject>
#include <QtCore/QSharedData>

#include "message.h"

namespace Jolie
{

class PendingCallPrivate;
class PendingCallWatcher;

class PendingCallWaiter
{
public:
    void waitForFinished(PendingCallPrivate *pendingCall);

private:
    friend class PendingCallPrivate;
    QEventLoop eventLoop;
};

class PendingCallPrivate : public QSharedData
{
public:
    PendingCallPrivate(const Message &message)
        : id(message.id()), isFinished(false) {}

    void setReply(const Message &message);

private:
    friend class PendingCall;
    friend class PendingCallWatcher;
    friend class PendingCallWaiter;
    friend class PendingReply;

    qint64 id;
    bool isFinished;
    Message reply;
    QList<PendingCallWaiter*> waiters;
    QList<PendingCallWatcher*> watchers;
};

} // namespace Jolie

#endif

