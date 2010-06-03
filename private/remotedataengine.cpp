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

#include "remotedataengine_p.h"

#include "remoteservice_p.h"

#include "../remote/accessmanager.h"
#include "../remote/serviceaccessjob.h"
#include "../service.h"
#include "../servicejob.h"

#include <QTimer>
#include <QUuid>

#include <kurl.h>

namespace Plasma
{

RemoteDataEngine::RemoteDataEngine(KUrl location, QObject* parent, const QVariantList& args)
    : Plasma::DataEngine(parent, args),
      m_service(0),
      m_location(location),
      m_uuid("")
{
    if (!location.isEmpty()) {
        setLocation(location);
    } else {
        kDebug() << "LOCATION IS EMPTY";
    }
}

RemoteDataEngine::~RemoteDataEngine()
{
}

void RemoteDataEngine::setLocation(KUrl location)
{
    m_location = location;
    setMinimumPollingInterval(1000);
    setPollingInterval(5000);
    m_uuid = QUuid::createUuid();
    Service *service = Service::access(location);
    connect(service, SIGNAL(serviceReady(Plasma::Service*)),
            this, SLOT(serviceReady(Plasma::Service*)));
}

Plasma::Service* RemoteDataEngine::serviceForSource(const QString& source)
{
    if (m_serviceForSource.contains(source)) {
        return m_serviceForSource[source];
    } else {
        RemoteService *service = new RemoteService(this);
        initRemoteService(source, service);
        return service;
    }
}

void RemoteDataEngine::initRemoteService(const QString &source, RemoteService *service)
{
    if (m_service) {
        KConfigGroup op = m_service->operationDescription("ServiceForSource");
        op.writeEntry("SourceName", source);
        m_service->startOperationCall(op);
        m_serviceForSource[source] = service;
    } else {
        m_pendingServices[source] = service;
    }
}

void RemoteDataEngine::init()
{
}

void RemoteDataEngine::serviceReady(Plasma::Service *service)
{
    m_service = service;

    KConfigGroup op = m_service->operationDescription("GetSourceNames");
    m_service->startOperationCall(op);
    connect(m_service, SIGNAL(finished(Plasma::ServiceJob*)),
            this, SLOT(remoteCallFinished(Plasma::ServiceJob*)));
    //FIXME: every 5s? this MUST become push rather than pull
    QTimer *timer = new QTimer(this);
    timer->setInterval(5000);
    connect(timer, SIGNAL(timeout()), this, SLOT(updateSources()));
    timer->start();
}

QStringList RemoteDataEngine::sources() const
{
    return m_sources.toList();
}

void RemoteDataEngine::remoteCallFinished(Plasma::ServiceJob *job)
{
    if (job->operationName() == "GetSourceNames") {
        kDebug() << "get source names";
        const QSet<QString> oldsources = m_sources;
        m_sources = QSet<QString>::fromList(job->result().toStringList());

        //first check if there are sources that have to be removed:
        foreach (const QString &source, oldsources) {
            if (!m_sources.contains(source)) {
                kDebug() << "source no longer exists... remove that data.";
                removeSource(source);
                emit sourceRemoved(source);
            }
        }

        //are there sources that have to be added?
        {
            // we have to check the current container dict, because we may have added
            // an empty data set in a call to sourceRequestEvent before the service was
            // ready; in this case, it will already exist in the engine's container
            // collection and be listed in pendingSources.
            // we also have to check oldSources since it may be in there, but never
            // actually connected to from the local side
            const SourceDict s = containerDict();
            foreach (const QString &source, m_sources) {
                if (!oldsources.contains(source) && !s.contains(source)) {
                    kDebug() << "new source = " << source;
                    emit sourceAdded(source);
                }
            }
        }

        //and now check and update any pending resources
        foreach (const QString &pendingSource, m_pendingSources) {
            createSource(pendingSource);
        }
        m_pendingSources.clear();
    } else if (job->operationName() == "GetSource") {
        QString source = job->parameters().value("SourceName").toString();
        kDebug() << "setting data for " << source;
        bool newSource = !m_sources.contains(source);
        if (job->result().type() == QVariant::Bool && job->result().toBool() == false) {
            kDebug() << "there is no update";
            if (newSource) {
                // the source doesn't exist on the remote side!
                removeSource(source);
                emit sourceRemoved(source);
                m_pendingServices.remove(source);
            }
        } else {
            if (newSource) {
                m_sources.insert(source);
                emit sourceAdded(source);

                RemoteService *rs = m_pendingServices.value(source);
                if (rs) {
                    m_pendingServices.remove(source);
                    initRemoteService(source, rs);
                }
            }

            setData(source, static_cast<Plasma::DataEngine::Data>(job->result().toHash()));
        }
    } else {
        QString source = job->parameters().value("SourceName").toString();
        kDebug() << "setting serviceForSource for " << source;
        QString resource = job->result().toString();
        KUrl loc = m_location;
        loc.setFileName(resource);
        RemoteService *rs = m_serviceForSource.value(source);
        if (rs) {
            rs->setLocation(loc);
        } else {
            kDebug() << "no such service?" << source;
        }
    }
}

bool RemoteDataEngine::updateSourceEvent(const QString &source)
{
    if (!m_service) {
        return false;
    }

    KConfigGroup op = m_service->operationDescription("GetSource");
    op.writeEntry("SourceName", source);
    op.writeEntry("UUID", m_uuid);
    m_service->startOperationCall(op);
    return false;
}

bool RemoteDataEngine::sourceRequestEvent(const QString &source)
{
    setData(source, Data());

    if (m_service) {
        createSource(source);
    } else {
        m_pendingSources.append(source);
    }

    return true;
}

void RemoteDataEngine::createSource(const QString &source)
{
    KConfigGroup op = m_service->operationDescription("GetSource");
    op.writeEntry("SourceName", source);
    op.writeEntry("UUID", m_uuid);
    m_service->startOperationCall(op);
}

void RemoteDataEngine::updateSources()
{
    if (m_service) {
        KConfigGroup op = m_service->operationDescription("GetSourceNames");
        m_service->startOperationCall(op);
    }
}

}

#include "remotedataengine_p.moc"

