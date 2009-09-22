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

#include "accessmanager.h"
#include "private/accessmanager_p.h"

#include "authorizationmanager.h"
#include "service.h"
#include "serviceaccessjob.h"
#include "private/authorizationmanager_p.h"

#include "config-plasma.h"

#include <QtCore/QMap>
#include <QtCore/QTimer>

#include <dnssd/remoteservice.h>
#include <dnssd/servicebrowser.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <ktemporaryfile.h>
#include <kurl.h>

#include <QtJolie/Message>

namespace Plasma
{

class AccessManagerSingleton
{
    public:
        AccessManager self;
};

K_GLOBAL_STATIC(AccessManagerSingleton, privateAccessManagerSelf)

AccessManager *AccessManager::self()
{
    return &privateAccessManagerSelf->self;
}

AccessManager::AccessManager()
    : QObject(),
      d(new AccessManagerPrivate(this))
{
    KGlobal::dirs()->addResourceType("trustedkeys", "config", "trustedkeys/");
}

AccessManager::~AccessManager()
{
    delete d;
}

AccessAppletJob *AccessManager::accessRemoteApplet(const KUrl &location) const
{
    AuthorizationManager::self()->d->prepareForServiceAccess();

    KUrl resolvedLocation;
    if (location.protocol() == "plasma+zeroconf") {
        if (d->zeroconfServices.contains(location.host())) {
            resolvedLocation = d->services[location.host()].remoteLocation();
        } else {
            kDebug() << "There's no zeroconf service with this name.";
        }
    } else {
        resolvedLocation = location;
    }

    AccessAppletJob *job = new AccessAppletJob(resolvedLocation);
    connect(job, SIGNAL(finished(KJob*)), this, SLOT(slotJobFinished(KJob*)));
    QTimer::singleShot(0, job, SLOT(slotStart()));
    return job;
}

QList<PackageMetadata> AccessManager::remoteApplets() const
{
    return d->services.values();
}

QStringList AccessManager::supportedProtocols()
{
    QStringList list;
    list << "plasma" << "plasma+zeroconf";
    return list;
}

AccessManagerPrivate::AccessManagerPrivate(AccessManager *manager)
    : q(manager),
      browser(new DNSSD::ServiceBrowser("_plasma._tcp"))
{
#ifdef ENABLE_REMOTE_WIDGETS
    q->connect(browser, SIGNAL(serviceAdded(DNSSD::RemoteService::Ptr)),
               q, SLOT(slotAddService(DNSSD::RemoteService::Ptr)));
    q->connect(browser, SIGNAL(serviceRemoved(DNSSD::RemoteService::Ptr)),
               q, SLOT(slotRemoveService(DNSSD::RemoteService::Ptr)));
    browser->startBrowse();
#else
    kWarning() << "libplasma is compiled without support for remote widgets. not monitoring remote widgets on the network";
#endif
}

AccessManagerPrivate::~AccessManagerPrivate()
{
    delete browser;
}

void AccessManagerPrivate::slotJobFinished(KJob *job)
{
    emit q->finished(static_cast<AccessAppletJob*>(job));
}

void AccessManagerPrivate::slotAddService(DNSSD::RemoteService::Ptr service)
{
    kDebug();
    if (!service->resolve()) {
        kDebug() << "Zeroconf service can't be resolved";
        return;
    }

    if (!services.contains(service->serviceName())) {
        PackageMetadata metadata;
        kDebug() << "textdata = " << service->textData();
        kDebug() << "hostname: " << service->hostName();
        QHostAddress address = DNSSD::ServiceBrowser::resolveHostName(service->hostName());
        QString ip = address.toString();
        kDebug() << "result for resolve = " << ip;

        KUrl url(QString("plasma://%1:%2/%3").arg(ip)
                                             .arg(service->port())
                                             .arg(service->serviceName()));

        if (!service->textData().isEmpty()) {
            kDebug() << "service has got textdata";
            QMap<QString, QByteArray> textData = service->textData();
            metadata.setName(textData["name"]);
            metadata.setDescription(textData["description"]);
            metadata.setRemoteLocation(url.prettyUrl());
        } else {
            kDebug() << "no textdata?";
            metadata.setName(service->serviceName());
            metadata.setRemoteLocation(url.prettyUrl());
        }

        kDebug() << "location = " << metadata.remoteLocation();
        kDebug() << "name = " << metadata.name();
        kDebug() << "description = " << metadata.name();

        services[service->serviceName()] = metadata;
        zeroconfServices[service->serviceName()] = service;
        emit q->remoteAppletAnnounced(metadata);
    }
}

void AccessManagerPrivate::slotRemoveService(DNSSD::RemoteService::Ptr service)
{
    kDebug();
    emit q->remoteAppletUnannounced(services[service->serviceName()]);
    services.remove(service->serviceName());
    zeroconfServices.remove(service->serviceName());
}

} // Plasma namespace

#include "accessmanager.moc"
