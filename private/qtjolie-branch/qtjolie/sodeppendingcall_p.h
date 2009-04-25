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

#ifndef SODEPPENDINGCALL_P_H
#define SODEPPENDINGCALL_P_H

#include <QtCore/QEventLoop>
#include <QtCore/QObject>
#include <QtCore/QSharedData>

#include "sodepmessage.h"

class SodepPendingCallPrivate;

class SodepPendingCallWaiter
{
public:
    void waitForFinished(SodepPendingCallPrivate *pendingCall);

private:
    friend class SodepPendingCallPrivate;
    QEventLoop eventLoop;
};

class SodepPendingCallPrivate : public QSharedData
{
public:
    SodepPendingCallPrivate(const SodepMessage &message)
        : id(message.id()), isFinished(false) {}

    void setReply(const SodepMessage &message);

private:
    friend class SodepPendingCall;
    friend class SodepPendingCallWaiter;

    qint64 id;
    bool isFinished;
    SodepMessage reply;
    QList<SodepPendingCallWaiter*> waiters;
};


#endif
