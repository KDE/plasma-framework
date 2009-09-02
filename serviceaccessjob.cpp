/*
 *   Copyright 2009 Rob Scheepmaker <r.scheepmaker@student.utwente.nl>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
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

#include "serviceaccessjob.h"
#include "private/remoteservice_p.h"

namespace Plasma
{

class ServiceAccessJobPrivate
{
public:
    ServiceAccessJobPrivate(ServiceAccessJob *owner, KUrl location)
        : q(owner),
          service(0),
          location(location)
    {
    }

    void slotStart()
    {
        q->start();
    }

    void slotServiceReady()
    {
        q->emitResult();
    }

    ServiceAccessJob *q;
    Service *service;
    KUrl location;
};

ServiceAccessJob::ServiceAccessJob(KUrl location, QObject *parent)
    : KJob(parent),
      d(new ServiceAccessJobPrivate(this, location))
{
}

ServiceAccessJob::~ServiceAccessJob()
{
    delete d;
}

Service *ServiceAccessJob::service() const
{
    return d->service;
}

void ServiceAccessJob::start()
{
    d->service = new RemoteService(parent(), d->location);
    connect(d->service, SIGNAL(ready()), this, SLOT(slotServiceReady()));
}

} // namespace Plasma

#include "serviceaccessjob.moc"

