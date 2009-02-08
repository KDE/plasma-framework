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

#include "sodepmessage.h"
#include "sodephelpers_p.h"

SodepClientThread::SodepClientThread(QIODevice *device, QObject *parent)
    : QThread(parent), m_device(device), m_quit(false)
{

}

SodepClientThread::~SodepClientThread()
{
    m_quit = true;
    m_cond.wakeOne();
    wait();
}

void SodepClientThread::requestMessage()
{
    if (!isRunning()) {
        start();
    } else {
        m_cond.wakeOne();
    }
}

void SodepClientThread::run()
{
    while (!m_quit) {
        QMutexLocker locker(&m_mutex);

        SodepMessage message = sodepReadMessage(*m_device);
        emit messageLoaded(message);

        m_cond.wait(&m_mutex);
    }
}

#include "sodepclientthread_p.moc"
