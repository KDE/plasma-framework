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

#ifndef SODEPCLIENTTHREAD_P_H
#define SODEPCLIENTTHREAD_P_H

#include <QtCore/QThread>
#include <QtCore/QMutex>
#include <QtCore/QQueue>

class QAbstractSocket;

class SodepMessage;
class SodepClientPrivate;

class SodepClientThread : public QThread
{
    Q_OBJECT

public:
    explicit SodepClientThread(const QString &hostName, quint16 port, SodepClientPrivate *client);
    ~SodepClientThread();

    void run();

    void sendMessage(const SodepMessage &message);

signals:
    void messageReceived(const SodepMessage &message);

private slots:
    void readMessage();
    void writeMessageQueue();

private:
    QString m_hostName;
    quint16 m_port;

    QAbstractSocket *m_socket;
    SodepClientPrivate *m_client;

    QQueue<SodepMessage> m_messageQueue;

    QMutex m_mutex;
};

#endif
