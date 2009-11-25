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

#include "getsource_p.h"

#include "authorizationmanager_p.h"
#include "dataengineservice_p.h"
#include "service_p.h"

#include "../remote/authorizationmanager.h"
#include "../remote/authorizationrule.h"
#include "../dataengine.h"
#include "../service.h"

#include <kdebug.h>

namespace Plasma
{

GetSource::GetSource(DataEngine *engine, const QString& operation,
                    QMap<QString,QVariant>& parameters,
                    DataEngineService *service)
    : ServiceJob(QString("publickey"), operation, parameters, service),
      m_engine(engine),
      m_service(service)
{
}

void GetSource::start()
{
    kDebug() << "Trying to perform the action" << operationName();

    //TODO: check with capabilities before performing actions.
    if (operationName() == "GetSource") {
        QString source  = parameters().value("SourceName").toString();
        QString UUID    = parameters().value("UUID").toString();
        if (!m_service->m_peersAlreadyUpdated[source].contains(UUID)) {
            m_service->m_peersAlreadyUpdated[source].append(UUID);
            setResult(m_service->m_data[source]);
        } else {
            setResult(false); //no update needed
        }
    } else if (operationName() == "GetSourceNames") {
        setResult(m_engine->sources());
    } else if (operationName() == "ServiceForSource") {
        QString source      = parameters().value("SourceName").toString();
        Service *service    = m_engine->serviceForSource(source);
        QString serviceName = "plasma-service-" + service->name();

        kDebug() << "serviceForSource: getting source " << serviceName;
        service->d->publish(Plasma::NoAnnouncement, serviceName);
        if (!AuthorizationManager::self()->d->matchingRule(serviceName, identity())) {
            AuthorizationRule *rule = new AuthorizationRule(serviceName, identity().id());
            rule->setPolicy(AuthorizationRule::Allow);
            AuthorizationManager::self()->d->rules.append(rule);
        }
        setResult(serviceName);
    }
}

}

#include "getsource_p.moc"

