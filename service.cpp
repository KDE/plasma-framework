/*
 *   Copyright 2008 Aaron Seigo <aseigo@kde.org>
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

#include "service.h"
#include "private/authorizationmanager_p.h"
#include "private/service_p.h"
#include "private/serviceprovider_p.h"

#include "config-plasma.h"

#include <QFile>
#include <QTimer>

#include <kdebug.h>
#include <kservice.h>
#include <kservicetypetrader.h>
#include <ksharedconfig.h>
#include <kstandarddirs.h>
#include <dnssd/publicservice.h>
#include <dnssd/servicebrowser.h>

#include "configloader.h"
#include "version.h"
#include "private/configloader_p.h"
#include "private/remoteservice_p.h"
#include "private/remoteservicejob_p.h"
#include "pluginloader.h"

namespace Plasma
{

Service::Service(QObject *parent)
    : QObject(parent),
      d(new ServicePrivate(this))
{
}

Service::Service(QObject *parent, const QVariantList &args)
    : QObject(parent),
      d(new ServicePrivate(this))
{
    Q_UNUSED(args)
}

Service::~Service()
{
    d->unpublish();
    delete d;
}

Service *Service::load(const QString &name, QObject *parent)
{
    QVariantList args;
    return load(name, args, parent);
}

Service *Service::load(const QString &name, const QVariantList &args, QObject *parent)
{
    return PluginLoader::pluginLoader()->loadService(name, args, parent);
}

Service *Service::access(const KUrl &url, QObject *parent)
{
    return new RemoteService(parent, url);
}

void ServicePrivate::jobFinished(KJob *job)
{
    emit q->finished(static_cast<ServiceJob*>(job));
}

void ServicePrivate::associatedWidgetDestroyed(QObject *obj)
{
    associatedWidgets.remove(static_cast<QWidget*>(obj));
}

void ServicePrivate::associatedGraphicsWidgetDestroyed(QObject *obj)
{
    associatedGraphicsWidgets.remove(static_cast<QGraphicsWidget*>(obj));
}

void ServicePrivate::publish(AnnouncementMethods methods, const QString &name, PackageMetadata metadata)
{
#ifdef ENABLE_REMOTE_WIDGETS
    if (!serviceProvider) {
        AuthorizationManager::self()->d->prepareForServicePublication();

        serviceProvider = new ServiceProvider(name, q);

        if (methods.testFlag(ZeroconfAnnouncement) &&
                (DNSSD::ServiceBrowser::isAvailable() == DNSSD::ServiceBrowser::Working)) {
            //TODO: dynamically pick a free port number.
            publicService = new DNSSD::PublicService(name, "_plasma._tcp", 4000);

            QMap<QString, QByteArray> textData;
            textData["name"] = name.toUtf8();
            textData["plasmoidname"] = metadata.name().toUtf8();
            textData["description"] = metadata.description().toUtf8();
            publicService->setTextData(textData);
            kDebug() << "about to publish";

            publicService->publishAsync();
        } else if (methods.testFlag(ZeroconfAnnouncement) &&
                (DNSSD::ServiceBrowser::isAvailable() != DNSSD::ServiceBrowser::Working)) {
            kDebug() << "sorry, but your zeroconf daemon doesn't seem to be running.";
        }
    } else {
        kDebug() << "already published!";
    }
#else
    kWarning() << "libplasma is compiled without support for remote widgets. not publishing.";
#endif
}

void ServicePrivate::unpublish()
{
    if (serviceProvider) {
        delete serviceProvider;
        serviceProvider = 0;
    }

    if (publicService) {
        delete publicService;
        publicService = 0;
    }
}

bool ServicePrivate::isPublished() const
{
    if (serviceProvider) {
        return true;
    } else {
        return false;
    }
}

KConfigGroup ServicePrivate::dummyGroup()
{
    if (!dummyConfig) {
        dummyConfig = new KConfig(QString(), KConfig::SimpleConfig);
    }

    return KConfigGroup(dummyConfig, "DummyGroup");
}

void Service::setDestination(const QString &destination)
{
    d->destination = destination;
}

QString Service::destination() const
{
    return d->destination;
}

QStringList Service::operationNames() const
{
    if (!d->config) {
        kDebug() << "No valid operations scheme has been registered";
        return QStringList();
    }

    return d->config->groupList();
}

KConfigGroup Service::operationDescription(const QString &operationName)
{
    if (!d->config) {
        kDebug() << "No valid operations scheme has been registered";
        return d->dummyGroup();
    }

    d->config->writeConfig();
    KConfigGroup params(d->config->config(), operationName);
    //kDebug() << "operation" << operationName
    //         << "requested, has keys" << params.keyList() << "from"
    //         << d->config->config()->name();
    return params;
}

QMap<QString, QVariant> Service::parametersFromDescription(const KConfigGroup &description)
{
    QMap<QString, QVariant> params;

    if (!d->config || !description.isValid()) {
        return params;
    }

    const QString op = description.name();
    foreach (const QString &key, description.keyList()) {
        KConfigSkeletonItem *item = d->config->findItem(op, key);
        if (item) {
            params.insert(key, description.readEntry(key, item->property()));
        }
    }

    return params;
}

ServiceJob *Service::startOperationCall(const KConfigGroup &description, QObject *parent)
{
    // TODO: nested groups?
    ServiceJob *job = 0;
    const QString op = description.isValid() ? description.name() : QString();

    RemoteService *rs = qobject_cast<RemoteService *>(this);
    if (!op.isEmpty() && rs && !rs->isReady()) {
        // if we have an operation, but a non-ready remote service, just let it through
        kDebug() << "Remote service is not ready; queueing operation";
        QMap<QString, QVariant> params;
        job = createJob(op, params);
        RemoteServiceJob *rsj = qobject_cast<RemoteServiceJob *>(job);
        if (rsj) {
            rsj->setDelayedDescription(description);
        }
    } else if (!d->config) {
        kDebug() << "No valid operations scheme has been registered";
    } else if (!op.isEmpty() && d->config->hasGroup(op)) {
        if (d->disabledOperations.contains(op)) {
            kDebug() << "Operation" << op << "is disabled";
        } else {
            QMap<QString, QVariant> params = parametersFromDescription(description);
            job = createJob(op, params);
        }
    } else {
        kDebug() << "Not a valid group!";
    }

    if (!job) {
        job = new NullServiceJob(destination(), op, this);
    }

    job->setParent(parent ? parent : this);
    connect(job, SIGNAL(finished(KJob*)), this, SLOT(jobFinished(KJob*)));
    QTimer::singleShot(0, job, SLOT(slotStart()));
    return job;
}

void Service::associateWidget(QWidget *widget, const QString &operation)
{
    disassociateWidget(widget);
    d->associatedWidgets.insert(widget, operation);
    connect(widget, SIGNAL(destroyed(QObject*)), this, SLOT(associatedWidgetDestroyed(QObject*)));

    widget->setEnabled(!d->disabledOperations.contains(operation));
}

void Service::disassociateWidget(QWidget *widget)
{
    disconnect(widget, SIGNAL(destroyed(QObject*)),
               this, SLOT(associatedWidgetDestroyed(QObject*)));
    d->associatedWidgets.remove(widget);
}

void Service::associateWidget(QGraphicsWidget *widget, const QString &operation)
{
    disassociateWidget(widget);
    d->associatedGraphicsWidgets.insert(widget, operation);
    connect(widget, SIGNAL(destroyed(QObject*)),
            this, SLOT(associatedGraphicsWidgetDestroyed(QObject*)));

    widget->setEnabled(!d->disabledOperations.contains(operation));
}

void Service::disassociateWidget(QGraphicsWidget *widget)
{
    disconnect(widget, SIGNAL(destroyed(QObject*)),
               this, SLOT(associatedGraphicsWidgetDestroyed(QObject*)));
    d->associatedGraphicsWidgets.remove(widget);
}

QString Service::name() const
{
    return d->name;
}

void Service::setName(const QString &name)
{
    d->name = name;

    // now reset the config, which may be based on our name
    delete d->config;
    d->config = 0;

    delete d->dummyConfig;
    d->dummyConfig = 0;

    registerOperationsScheme();

    emit serviceReady(this);
}

void Service::setOperationEnabled(const QString &operation, bool enable)
{
    if (!d->config || !d->config->hasGroup(operation)) {
        return;
    }

    if (enable) {
        d->disabledOperations.remove(operation);
    } else {
        d->disabledOperations.insert(operation);
    }

    {
        QHashIterator<QWidget *, QString> it(d->associatedWidgets);
        while (it.hasNext()) {
            it.next();
            if (it.value() == operation) {
                it.key()->setEnabled(enable);
            }
        }
    }

    {
        QHashIterator<QGraphicsWidget *, QString> it(d->associatedGraphicsWidgets);
        while (it.hasNext()) {
            it.next();
            if (it.value() == operation) {
                it.key()->setEnabled(enable);
            }
        }
    }
}

bool Service::isOperationEnabled(const QString &operation) const
{
    return d->config && d->config->hasGroup(operation) && !d->disabledOperations.contains(operation);
}

void Service::setOperationsScheme(QIODevice *xml)
{
    delete d->config;

    delete d->dummyConfig;
    d->dummyConfig = 0;

    KSharedConfigPtr c = KSharedConfig::openConfig(QString(), KConfig::SimpleConfig);
    d->config = new ConfigLoader(c, xml, this);
    d->config->d->setWriteDefaults(true);

    emit operationsChanged();

    {
        QHashIterator<QWidget *, QString> it(d->associatedWidgets);
        while (it.hasNext()) {
            it.next();
            it.key()->setEnabled(d->config->hasGroup(it.value()));
        }
    }

    {
        QHashIterator<QGraphicsWidget *, QString> it(d->associatedGraphicsWidgets);
        while (it.hasNext()) {
            it.next();
            it.key()->setEnabled(d->config->hasGroup(it.value()));
        }
    }
}

void Service::registerOperationsScheme()
{
    if (d->config) {
        // we've already done our job. let's go home.
        return;
    }

    if (d->name.isEmpty()) {
        kDebug() << "No name found";
        return;
    }

    QString path = KStandardDirs::locate("data", "plasma/services/" + d->name + ".operations");

    if (path.isEmpty()) {
        kDebug() << "Cannot find operations description:" << d->name << ".operations";
        return;
    }

    QFile file(path);
    setOperationsScheme(&file);
}

} // namespace Plasma

#include "service.moc"

