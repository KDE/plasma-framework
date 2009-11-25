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

#include "dataengineservice_p.h"

#include "../dataengine.h"
#include "getsource_p.h"

#include <kdebug.h>

namespace Plasma
{

DataEngineService::DataEngineService(DataEngine *engine)
    : Plasma::Service(engine),
      m_engine(engine)
{
    setName("dataengineservice");
    engine->connectAllSources(this, 1000);
    connect(engine, SIGNAL(sourceAdded(QString)), this, SLOT(sourceAdded(QString)));
}

Plasma::ServiceJob* DataEngineService::createJob(const QString& operation,
                                          QMap<QString,QVariant>& parameters)
{
    return new GetSource(m_engine, operation, parameters, this);
}

void DataEngineService::dataUpdated(QString source, Plasma::DataEngine::Data data)
{
    if (data != m_data[source]) {
        m_data[source] = data;
        m_peersAlreadyUpdated[source] = QStringList();
    }
}

void DataEngineService::sourceAdded(QString source)
{
    m_engine->connectSource(source, this, 1000);
}

}

#include "dataengineservice_p.moc"

