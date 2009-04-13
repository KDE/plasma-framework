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

#include "sodepclientthread_p.h"

#include <QtCore/QTimer>
#include <QtNetwork/QTcpSocket>

#include "sodepclient_p.h"
#include "sodepmessage.h"
#include "sodephelpers_p.h"

SodepClientThread::SodepClientThread(const QString &hostName, quint16 port, SodepClientPrivate *client)
    : QThread(), m_hostName(hostName), m_port(port), m_socket(0), m_client(client)
{
    moveToThread(this);
}

SodepClientThread::~SodepClientThread()
{
    quit();
    wait();
}

void SodepClientThread::sendMessage(const SodepMessage &message)
{
  QMutexLocker locker(&m_mutex);

  m_messageQueue.enqueue(message);
  QTimer::singleShot(0, this, SLOT(writeMessageQueue()));
}

void SodepClientThread::writeMessageQueue()
{
  QMutexLocker locker(&m_mutex);

  while (!m_messageQueue.isEmpty()) {
    sodepWrite(*m_socket, m_messageQueue.dequeue());
  }
}

void SodepClientThread::readMessage()
{
    if (m_socket->bytesAvailable()==0) {
        return;
    }

    SodepMessage message = sodepReadMessage(*m_socket);
    emit messageReceived(message);

    if (m_socket->bytesAvailable()>0) {
        QTimer::singleShot(0, this, SLOT(readMessage()));
    }
}

void SodepClientThread::run()
{
    m_socket = new QTcpSocket;

    connect(m_socket, SIGNAL(readyRead()),
            this, SLOT(readMessage()), Qt::QueuedConnection);
    connect(this, SIGNAL(messageReceived(SodepMessage)),
            m_client, SLOT(messageReceived(SodepMessage)));

    m_socket->connectToHost(m_hostName, m_port);
    m_socket->waitForConnected(-1);

    exec();

    delete m_socket;
}

#include "sodepclientthread_p.moc"
