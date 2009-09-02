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

namespace Plasma
{

class ServiceAccessJobPrivate
{
public:
    ServiceAccessJobPrivate(ServiceJob *owner, KUrl location)
        : q(owner),
          location(location)
    {
    }

    void slotStart()
    {
        q->start();
    }

    ServiceAccessJob *q;
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
    return d->destination;
}

void ServiceAccessJob::start()
{
    //TODO: implement
}

} // namespace Plasma

#include "servicejob.moc"

