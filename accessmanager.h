/*
 * Copyright 2009 by Rob Scheepmaker <r.scheepmaker@student.utwente.nl>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA  02110-1301  USA
 */

#ifndef PLASMA_ACCESSMANAGER_H
#define PLASMA_ACCESSMANAGER_H

#include "plasma_export.h"

#include <QtCore/QList>
#include <QtCore/QObject>
#include <QtCore/QString>

#include <kdebug.h>
#include "accessappletjob.h"
#include "packagemetadata.h"

class QString;
class KUrl;

namespace Plasma
{

class AccessManagerPrivate;
class ServiceAccessJob;

/**
 * @class AccessManager plasma/accessmanager.h <Plasma/AccessManager>
 *
 * @short Allows access to remote Plasma::Applet classes.
 *
 * This manager provides a way to access a plasmoid that is hosted on another machine. It also
 * provides a mechanism to discover services announced to the network through zeroconf or
 * bluetooth.
 *
 * @since 4.4
 */

class PLASMA_EXPORT AccessManager : public QObject
{
    Q_OBJECT

    public:
        /**
         * Singleton pattern accessor.
         */
        static AccessManager *self();

        /**
         * Access a native plasmoid hosted on another machine.
         * @param location the location of the remote plasmoids. Exmples of valid urls:
         * plasma://ip:port/resourceName
         * zeroconf://PlasmoidName
         * @returns a job that can be used to track when a remote plasmoid is ready for use, and to
         * obtain the applet when the package is sent over.
         */
        AccessAppletJob *accessRemoteApplet(const KUrl &location) const;

        /**
         * @returns a map mapping service names to the plasmoid's metadata.
         */
        QList<PackageMetadata> remoteApplets() const;

    Q_SIGNALS:
        /**
         * fires when a AccessAppletJob is finished.
         */
        void finished(Plasma::AccessAppletJob*);

        /**
         * fires when a new plasmoid is announced on the network.
         */
        void remoteAppletAnnounced(Plasma::PackageMetadata metadata);

        /**
         * fires when an announced plasmoid disappears from the network.
         */
        void remoteAppletUnannounced(Plasma::PackageMetadata metadata);

    private:
        AccessManager();
        ~AccessManager();

        AccessManagerPrivate * const d;

        Q_PRIVATE_SLOT(d, void slotJobFinished(KJob*))
        Q_PRIVATE_SLOT(d, void slotAddService(DNSSD::RemoteService::Ptr service))
        Q_PRIVATE_SLOT(d, void slotRemoveService(DNSSD::RemoteService::Ptr service))

        friend class AccessManagerPrivate;
        friend class AccessManagerSingleton;
};
} // Plasma namespace

#endif

