/*
 * Copyright 2009 by Rob Scheepmaker <r.scheepmaker@student.utwente.nl>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA  02110-1301  USA
 */

#ifndef PLASMA_PUBLICKEYFETCHER_H
#define PLASMA_PUBLICKEYFETCHER_H

#include <QtCore/QList>
#include <QtCore/QObject>
#include <QtJolie/Message>

class QString;

namespace Plasma
{

class AuthorizationManager;
class ServiceAccessJob;
class ServiceJob;

class PublicKeyFetcher : public QObject
{
    Q_OBJECT
    public:
        PublicKeyFetcher(const QString &sender, AuthorizationManager *am);
        ~PublicKeyFetcher();

        QList<Jolie::Message> pendingMessages();
        
    private Q_SLOTS:
        void slotKeyServiceReady(ServiceAccessJob *job);
        void slotKeyServiceObtained(ServiceAccessJob *job);

    private:
        QList<Jolie::Message> m_messageList;
        QString m_sender;
};
} // Plasma namespace

#endif

