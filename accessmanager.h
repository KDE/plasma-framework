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

#include <QtCore/QObject>

class QString;
class QMap;
class KUrl;

namespace Plasma
{

class ServiceAccessJob;

/**
 * @class AccessManager plasma/accessmanager.h <Plasma/AccessManager>
 *
 * @short Allows access to remote Plasma::Service, Plasma::DataEngine and Plasma::Applet classes.
 *
 * This manager provides a way to access a Plasma::Service, Plasma::DataEngine or Plasma::Applet
 * that is hosted on another machine. Besides functions to access those resources, this class is
 * also the place that tracks which remote computers are allowed to access each published resource,
 * and it provides a mechanism to discover services announced to the network through zeroconf or
 * bluetooth.
 * All url's passed to the access functions need to be valid JOLIE urls, that have this format:
 * plasma://<hostname/ip>:<port>/!/<servicename>
 * All access function are asynchronous. The services need to be accessed over the network, and
 * might even have to be authorized by the user of that machine first. All access functions
 * therefore return a job that can be monitored to see when the service is ready for use.
 *
 * @since 4.4?
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
         * Access a native Plasma::Service hosted on another machine.
         *
         * @param location a valid JOLIE url
         * @returns a job that can be monitored to see when access to the remote service is
         * obtained, or if it failed.
         */
        ServiceAccessJob* accessService(KUrl location) const;

        /**
         * TODO: I think there should be a more elegant way to access SOAP services right? Plus if
         * we want this to work with RemoteService, the JOLIE script is required to have the exact
         * native plasma service interface.... which means, amonst others: provide an 
         * operationsDescription operation which returns valid ConfigXml. This way of accessing is
         * easy enough, but I fear the creation of the jolieScript will become more complicated then
         * it needs to be.... or we need to have some utility in the feature that automagically
         * creates JOLIE script from WSDL... which would be totally awesome.
         * Create a Plasma::Service that accesses a not native Plasma::Service like a SOAP service. 
         * To accomplish this you'll need to provide the name of a JOLIE script that accesses this 
         * service and has a valid interface (TODO: I'll need to provide a include for jolie scripts
         * for this) and optionally a map that will be passed to the JOLIE script's init function,
         * and can contain things like username/password, url etc.
         * @param jolieScript filename of the jolie script. TODO: which path's to look?
         * @param initValues map of strings>variants that will get passed to the jolie script's init
         * function.
         * @returns a job that can be monitored to see when access to the remote service is
         * obtained, or if it failed.
         */
        ServiceAccessJob* accessService(const QString &jolieScript, 
                                        const QMap<QString, QVariant> &initValues) const;

        Jolie::Message signMessage(Jolie::Message message) const;

        //TODO: access functions for engines and applets... which are higher level things built on
        //top of Plasma::Service, which means I'll first need to get services working and
        //everything.

        //TODO: functions for service discovery through bluetooth and zeroconf

    private:
        AccessManager();
        ~AccessManager();

        AccessManagerPrivate * const d;

        friend class AccessManagerSingleton;
};
} // Plasma namespace

#endif

