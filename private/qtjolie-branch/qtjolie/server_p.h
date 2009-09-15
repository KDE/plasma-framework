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

#ifndef QTJOLIE_SERVER_P_H
#define QTJOLIE_SERVER_P_H

#include "server.h"

#include <QtCore/QMap>
#include <QtCore/QObject>

class QIODevice;

namespace Jolie
{
class ServerThread;
class AbstractAdaptor;

class ServerPrivate : public QObject
{
    Q_OBJECT
public:
    ServerPrivate(Server *server)
        : q(server),
          error(Server::NoError) {}

public slots:
    void messageReceived(int descriptor, const Jolie::Message &message);

private:
    friend class Server;
    Server * const q;
    ServerThread *serverThread;

    Server::Error error;
    QString errorString;
    QMap<QByteArray, AbstractAdaptor*> adaptors;
};

} // namespace Jolie

#endif

