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

#include "serverthread_p.h"

#include <QtNetwork/QTcpSocket>
#include <QtNetwork/QTcpServer>

#include "server_p.h"
#include "message.h"
#include "sodephelpers_p.h"

using namespace Jolie;

Q_DECLARE_METATYPE(QAbstractSocket*)
static int _jolie_socketDataTypeId = qRegisterMetaType<QAbstractSocket*>();

ServerThread::ServerThread(quint16 port, ServerPrivate *server)
    : QThread(), m_port(port), m_serverSocket(0), m_server(server)
{
    moveToThread(this);
}

ServerThread::~ServerThread()
{
    quit();
    wait();
}

void ServerThread::sendMessage(int descriptor, const Message &message)
{
    QMutexLocker locker(&m_mutex);

    m_messageQueue.enqueue(QPair<int, Message>(descriptor, message));
    QMetaObject::invokeMethod(this, "writeMessageQueue", Qt::QueuedConnection);
}

void ServerThread::writeMessageQueue()
{
    QMutexLocker locker(&m_mutex);

    while (!m_messageQueue.isEmpty()) {
        QPair<int, Message> pair = m_messageQueue.dequeue();
        sodepWrite(*m_sockets[pair.first], pair.second);
    }
}

void ServerThread::onIncomingConnection()
{
    while (m_serverSocket->hasPendingConnections()) {
        QAbstractSocket *socket = m_serverSocket->nextPendingConnection();
        m_sockets[socket->socketDescriptor()] = socket;

        connect(socket, SIGNAL(readyRead()),
                this, SLOT(onReadyRead()), Qt::QueuedConnection);
    }
}

void ServerThread::onReadyRead()
{
    QAbstractSocket *socket = static_cast<QAbstractSocket*>(sender());
    readMessage(socket);
}

void ServerThread::readMessage(QAbstractSocket *socket)
{
    if (socket->bytesAvailable()==0) {
        return;
    }

    Message message = sodepReadMessage(*socket);
    emit messageReceived(socket->socketDescriptor(), message);

    if (socket->bytesAvailable()>0) {
        QMetaObject::invokeMethod(this, "readMessage", Qt::QueuedConnection, Q_ARG(QAbstractSocket*, socket));
    }
}

void ServerThread::run()
{
    m_serverSocket = new QTcpServer;

    connect(this, SIGNAL(messageReceived(int, Jolie::Message)),
            m_server, SLOT(messageReceived(int, Jolie::Message)));
    connect(m_serverSocket, SIGNAL(newConnection()),
            this, SLOT(onIncomingConnection()), Qt::QueuedConnection);

    m_serverSocket->listen(QHostAddress::Any, m_port);

    exec();

    delete m_serverSocket;
}

#include "serverthread_p.moc"
