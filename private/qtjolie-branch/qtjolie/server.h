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

#ifndef QTJOLIE_SERVER_H
#define QTJOLIE_SERVER_H

#include <QtCore/QtGlobal>

namespace Jolie
{
class AbstractAdaptor;
class ServerPrivate;
class Message;

class Q_DECL_EXPORT Server
{
public:
    enum Error
    {
        NoError,
        UnexpectedClose,
        UnkownError
    };

    explicit Server(quint16 port);
    ~Server();

    Error error() const;
    QString errorString() const;

    void sendReply(int clientId, const Message &reply);

    bool registerAdaptor(const QByteArray &path, AbstractAdaptor *adaptor);
    bool unregisterAdaptor(const QByteArray &path);

private:
    friend class ServerPrivate;
    ServerPrivate * const d;
};

} // namespace Jolie

#endif

