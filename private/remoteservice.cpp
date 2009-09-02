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

#ifndef REMOTESERVICE_H
#define REMOTESERVICE_H

#include <QtJolie/Client>

#include "remoteservice_p.h"
#include "remoteservicejob_p.h"

namespace Plasma 
{

RemoteService::RemoteService(QObject* parent, KUrl location)
    : m_location(location),
      m_client(Jolie::Client(location->host(), location->port()))
{
    Jolie::Message getOpDesc(location->path(), "getOperationNames");
    //TODO: async
    Jolie::Message response = m_client->call(AccessManager::self()->signMessage(getOpDesc));
    QBuffer buffer(response.data().toByteArray());
    buffer.open(QBuffer::ReadWrite);
    setOperationsScheme(&buffer);
}

RemoteService::ServiceJob* createJob(const QString& operation,
                                     QMap<QString,QVariant>& parameters)
{
    return new RemoteServiceJob(m_location, operation, parameters, parent());
}

} //namespace Plasma

#include "remoteservice_p.moc"

#endif // REMOTESERVICE_H
