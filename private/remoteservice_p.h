/*
 *   Copyright © 2008 Rob Scheepmaker <r.scheepmaker@student.utwente.nl>
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

#ifndef PLASMA_REMOTESERVICE_P_H
#define PLASMA_REMOTESERVICE_P_H

#include <QtCore/QQueue>
#include <QtJolie/Message>

#include "../service.h"

namespace Jolie
{
    class Client;
    class PendingCallWatcher;
}

namespace Plasma 
{

class ClientPinRequest;
class RemoteServiceJob;

class RemoteService : public Plasma::Service
{
    Q_OBJECT

    public:
        RemoteService(QObject* parent);
        RemoteService(QObject* parent, KUrl location);
        ~RemoteService();

        void setLocation(const KUrl &location);
        QString location() const;

        bool isReady() const;
        bool isBusy() const;

    protected:
        ServiceJob* createJob(const QString& operation,
                              QMap<QString,QVariant>& parameters);
        void registerOperationsScheme();

    private Q_SLOTS:
        void callCompleted(Jolie::PendingCallWatcher *watcher);
        void slotFinished();
        void slotGotPin(Plasma::ClientPinRequest *request);
        void slotUpdateEnabledOperations();
        void slotReadyForRemoteAccess();

    private:
        Jolie::Message signMessage(const Jolie::Message &message) const;

    private:
        KUrl            m_location;
        Jolie::Client   *m_client;
        QByteArray      m_token;
        QByteArray      m_operationsScheme;
        bool            m_ready;
        QQueue<RemoteServiceJob*> m_queue;
        bool            m_busy;
        QString         m_pin;
        QString         m_uuid;

        friend class RemoteServiceJob;

};

} //namespace Plasma

#endif // PLASMA_REMOTESERVICE_P_H
