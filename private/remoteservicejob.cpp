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

#include <KUrl>

#include <QtJolie/Client>

#include "../servicejob.h"

namespace Plasma
{

RemoteServiceJob::RemoteServiceJob(KUrl location,
                  const QString& operation,
                  QMap<QString,QVariant>& parameters,
                  QObject* parent = 0);
                : ServiceJob(location, operation, parameters, parent),
                  m_location(location)
{
}

void RemoteServiceJob::start()
{
    Jolie::Client client(location.host(), location.port());
    Jolie::Message message(location.path(), "startOperation");
    Jolie::Message response = client.call(AccessManager::self()->signMessage(message));
    //TODO:async
    if (response.fault().isValid()) {
        setErrorText(response.fault().data().toByteArray());
        return;
    }
    if (response.data().isDouble()) {
        setResult(response.data().toDouble());
    } else if (response.data().isInt()) {
        setResult(response.data().toInt());
    } else {
        setResult(respoonse.data().toByteArray());
    }
}

} // namespace Plasma

#endif //REMOTESERVICEJOB_H
