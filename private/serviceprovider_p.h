/*
 *   Copyright © 2009 Rob Scheepmaker <r.scheepmaker@student.utwente.nl>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License version 2 as
 *   published by the Free Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef PLASMA_SERVICEPROVIDER_P_H
#define PLASMA_SERVICEPROVIDER_P_H

#include <QtCore/QMap>

#include <QtJolie/AbstractAdaptor>
#include <QtJolie/Message>

class QSignalMapper;

namespace Plasma 
{

class AuthorizationRule;
class Service;
class ServiceJob;

class ServiceProvider : public Jolie::AbstractAdaptor
{
    Q_OBJECT

    public:
        ServiceProvider(const QString &name, Service *service);
        ~ServiceProvider();

        void startOperationCall(Jolie::Message message);
        void sendOperations(Jolie::Message message);
        void sendEnabledOperations(Jolie::Message message);
        QString resourceName() const;

    protected:
        void relay(Jolie::Server *server, int descriptor, const Jolie::Message &message);

    private Q_SLOTS:
        void operationCompleted(Plasma::ServiceJob *job);
        void ruleChanged(Plasma::AuthorizationRule *rule);

    private:
        Jolie::Message appendToken(Jolie::Message message, const QByteArray &caller,
                                   const QByteArray &uuid);
        void authorize(const Jolie::Message &message, const QByteArray &validToken);
        void authorizationSuccess(const Jolie::Message &message);
        void authorizationFailed(const Jolie::Message &message, const QByteArray &error);

        Service                           *m_service;
        int                               m_descriptor;
        QString                           m_providerName;

        QMap<ServiceJob*, Jolie::Message> m_messageMap;
        QMap<QString, QByteArray>         m_tokens;
        QMap<QByteArray, int>             m_descriptorMap;
        QList<Jolie::Message>             m_messagesPendingAuthorization;

};

} //namespace Plasma

#endif //PLASMA_SERVICEPROVIDER_P_H
