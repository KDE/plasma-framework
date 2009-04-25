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

#ifndef QTJOLIE_CLIENTTHREAD_P_H
#define QTJOLIE_CLIENTTHREAD_P_H

#include <QtCore/QThread>
#include <QtCore/QMutex>
#include <QtCore/QQueue>

class QAbstractSocket;

namespace Jolie
{
class Message;
class ClientPrivate;

class ClientThread : public QThread
{
    Q_OBJECT

public:
    explicit ClientThread(const QString &hostName, quint16 port, ClientPrivate *client);
    ~ClientThread();

    void run();

    void sendMessage(const Message &message);

signals:
    void messageReceived(const Jolie::Message &message);

private slots:
    void readMessage();
    void writeMessageQueue();

private:
    QString m_hostName;
    quint16 m_port;

    QAbstractSocket *m_socket;
    ClientPrivate *m_client;

    QQueue<Message> m_messageQueue;

    QMutex m_mutex;
};

} // namespace Jolie

#endif

