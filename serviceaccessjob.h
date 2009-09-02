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

#ifndef PLASMA_SERVICEACCESSJOB_H
#define PLASMA_SERVICEACCESSJOB_H

#include <kjob.h>
#include <kservice.h>

#include <plasma/plasma_export.h>

class KUrl;

namespace Plasma
{

class ServiceAccessJobPrivate;

/**
 * @class ServiceAccessJob plasma/serviceaccessjob.h <Plasma/ServiceAccessJob>
 *
 * @short This class is used for asynchronously accessing a job published on a remote system. After
 * calling AccessManager::accessService, monitor this job to track when the remote service is ready
 * to be used, and to obtain the service when finished.
 */
class PLASMA_EXPORT ServiceAccessJob : public KJob
{
    Q_OBJECT

public:
    ~ServiceJob();

    Service *service() const;

protected:
    /**
     * Default constructor
     *
     * @arg location the location of the service
     * @arg parent the parent object for this service
     */
    ServiceJob(KUrl location, QObject *parent = 0);

private:
    Q_PRIVATE_SLOT(d, void slotStart())

    ServiceAccessJobPrivate * const d;
};

} // namespace Plasma

#endif // multiple inclusion guard

