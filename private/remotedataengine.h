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

#ifndef REMOTEDATAENGINE_H
#define REMOTEDATAENGINE_H

#include "../dataengine.h"
#include "remoteservice_p.h"

class JobView;

namespace Plasma
{
    class Service;
    class ServiceAccessJob;

class RemoteDataEngine : public Plasma::DataEngine
{
    Q_OBJECT

public:
    RemoteDataEngine(KUrl location, QObject* parent = 0, const QVariantList &args = QVariantList());
    ~RemoteDataEngine();
    Plasma::Service* serviceForSource(const QString& source);
    void setLocation(KUrl location);

protected:
    void init();
    QStringList sources() const;
    bool updateSourceEvent(const QString &source);
    bool sourceRequestEvent(const QString &source);

private Q_SLOTS:
    void serviceReady(Plasma::Service *service);
    void remoteCallFinished(Plasma::ServiceJob *job);
    void updateSources();
    void createSource(const QString &source);
    void initRemoteService(const QString &source, RemoteService *service);

private:
    Service         *m_service;
    QSet<QString>   m_sources;
    QHash<QString, RemoteService *> m_serviceForSource;
    QHash<QString, RemoteService *> m_pendingServices;
    QStringList     m_pendingSources;
    KUrl            m_location;
    QString         m_uuid;
};

} // namespace Plasma

#endif
