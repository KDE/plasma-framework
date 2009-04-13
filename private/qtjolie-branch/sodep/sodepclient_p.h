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

#ifndef SODEPCLIENT_P_H
#define SODEPCLIENT_P_H

#include "sodepclient.h"
#include "sodeppendingcall_p.h"

#include <QtCore/QMap>
#include <QtCore/QObject>

class SodepClientThread;

class SodepClientPrivate : public QObject
{
    Q_OBJECT
public:
    SodepClientPrivate(SodepClient *client)
        : q(client),
          error(SodepClient::NoError) {}

public slots:
    void messageReceived(const SodepMessage &message);

private:
    friend class SodepClient;
    SodepClient * const q;
    SodepClientThread *readerThread;

    SodepClient::Error error;
    QString errorString;
    QMap<int, QExplicitlySharedDataPointer<SodepPendingCallPrivate> > pendingCalls;
};

#endif
