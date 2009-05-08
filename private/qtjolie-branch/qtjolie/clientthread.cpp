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

#include "clientthread_p.h"

#include <QtNetwork/QTcpSocket>

#include "client_p.h"
#include "message.h"
#include "sodephelpers_p.h"

using namespace Jolie;

ClientThread::ClientThread(const QString &hostName, quint16 port, ClientPrivate *client)
    : QThread(), m_hostName(hostName), m_port(port), m_socket(0), m_client(client)
{
    moveToThread(this);
}

ClientThread::~ClientThread()
{
    quit();
    wait();
}

void ClientThread::sendMessage(const Message &message)
{
    QMutexLocker locker(&m_mutex);

    m_messageQueue.enqueue(message);
    QMetaObject::invokeMethod(this, "writeMessageQueue", Qt::QueuedConnection);
}

void ClientThread::writeMessageQueue()
{
    QMutexLocker locker(&m_mutex);

    while (!m_messageQueue.isEmpty()) {
        sodepWrite(*m_socket, m_messageQueue.dequeue());
    }
}

void ClientThread::readMessage()
{
    if (m_socket->bytesAvailable()==0) {
        return;
    }

    Message message = sodepReadMessage(*m_socket);
    emit messageReceived(message);

    if (m_socket->bytesAvailable()>0) {
        QMetaObject::invokeMethod(this, "readMessage", Qt::QueuedConnection);
    }
}

void ClientThread::run()
{
    m_socket = new QTcpSocket;

    connect(m_socket, SIGNAL(readyRead()),
            this, SLOT(readMessage()), Qt::QueuedConnection);
    connect(this, SIGNAL(messageReceived(Jolie::Message)),
            m_client, SLOT(messageReceived(Jolie::Message)));

    m_socket->connectToHost(m_hostName, m_port);
    m_socket->waitForConnected(30000);

    exec();

    delete m_socket;
}

#include "clientthread_p.moc"
