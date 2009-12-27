/*
 *   Copyright 2009 Rob Scheepmaker <r.scheepmaker@student.utwente.nl
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

#ifndef PLASMA_ACCESSAPPLETJOB_H
#define PLASMA_ACCESSAPPLETJOB_H

#include <kjob.h>

#include <plasma/plasma_export.h>

class KUrl;

namespace Plasma
{

class Applet;
class AccessAppletJobPrivate;

/**
 * @class AccessAppletJob plasma/accessappletjob.h <Plasma/AccessAppletJob>
 *
 * @short This class is used for asynchronously accessing an applet published on a remote system.
 * After calling AccessManager::accessApplet, monitor this job to track when the remote applet
 * is ready to be used, and to obtain the service when finished.
 */
class PLASMA_EXPORT AccessAppletJob : public KJob
{
    Q_OBJECT

public:
    ~AccessAppletJob();

    Applet *applet() const;

protected:
    /**
     * Default constructor
     *
     * @arg location the location of the service
     * @arg parent the parent object for this service
     */
    AccessAppletJob(const KUrl &location, QObject *parent = 0);

    void start();

private:
    AccessAppletJob();

    Q_PRIVATE_SLOT(d, void slotPackageDownloaded(Plasma::ServiceJob*))
    Q_PRIVATE_SLOT(d, void slotStart())
    Q_PRIVATE_SLOT(d, void slotServiceReady(Plasma::Service*))
    Q_PRIVATE_SLOT(d, void slotTimeout())

    AccessAppletJobPrivate * const d;
    
    friend class AccessManager;
    friend class AccessManagerPrivate;
    friend class AccessAppletJobPrivate;
};

} // namespace Plasma

#endif // PLASMA_ACCESSAPPLETJOB_H

