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
#include "service_p.h"

#include <QFile>

#include <KDebug>
#include <KService>
#include <KServiceTypeTrader>
#include <KSharedConfig>
#include <KStandardDirs>
#include <KTemporaryFile>

#include "configxml.h"

#include "version.h"

namespace Plasma
{

Service::Service(QObject *parent)
    : QObject(parent),
      d(new ServicePrivate(this))
{
    registerOperationsScheme();
}

Service::Service(QObject *parent, const QVariantList &args)
    : QObject(parent),
      d(new ServicePrivate(this))
{
    Q_UNUSED(args);
    registerOperationsScheme();
}

Service::~Service()
{
    delete d;
}

Service* Service::load(const QString &name, QObject *parent)
{
    //TODO: scripting API support
    if (name.isEmpty()) {
        return new NullService(parent);
    }

    QString constraint = QString("[X-KDE-PluginInfo-Name] == '%1'").arg(name);
    KService::List offers = KServiceTypeTrader::self()->query("Plasma/Service", constraint);

    if (offers.isEmpty()) {
        kDebug() << "offers is empty for " << name;
        return new NullService(parent);
    }

    KService::Ptr offer = offers.first();
    QString error;
    QVariantList args;
    Service* service = 0;

    if (Plasma::isPluginVersionCompatible(KPluginLoader(*offer).pluginVersion()))
        service = offer->createInstance<Plasma::Service>(parent, args, &error);

    if (!service) {
        kDebug() << "Couldn't load Service \"" << name << "\"! reason given: " << error;
        return new NullService(parent);
    }

    return service;
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

    return d->config->config()->groupList();
}

KConfigGroup Service::operationDescription(const QString &operationName)
{
    if (!d->config) {
        kDebug() << "No valid operations scheme has been registered";
        return KConfigGroup();
    }

    KConfigGroup params(d->config->config(), operationName);
    return params;
}

ServiceJob* Service::startOperationCall(const KConfigGroup &description)
{
    // TODO: nested groups?
    if (!d->config) {
        kDebug() << "No valid operations scheme has been registered";
        return new NullServiceJob(parent());
    }

    d->config->writeConfig();
    QMap<QString, QVariant> params;
    QString op = description.name();
    foreach (const QString &key, description.keyList()) {
        KConfigSkeletonItem *item = d->config->findItem(op, key);
        if (item) {
            params.insert(key, item->property());
        }
    }

    ServiceJob *job = createJob(description.name(), params);
    connect(job, SIGNAL(finished(KJob*)), this, SLOT(jobFinished(KJob*)));
    job->start();
    return job;
}

QString Service::name() const
{
    return d->name;
}

void Service::setName(const QString &name)
{
    d->name = name;
}

void Service::setOperationsScheme(QIODevice *xml)
{
    delete d->config;
    delete d->tempFile;

    //FIXME: make KSharedConfig and KConfigSkeleton not braindamaged in 4.2 and then get rid of the
    //       temp file object here
    d->tempFile = new KTemporaryFile;
    d->config = new ConfigXml(KSharedConfig::openConfig(d->tempFile->fileName()), xml, this);
}

void Service::registerOperationsScheme()
{
    if (d->name.isEmpty()) {
        kDebug() << "No name found";
        return;
    }

    QString path = KStandardDirs::locate("data", "plasma/services/" + d->name + ".operations");

    if (path.isEmpty()) {
        kDebug() << "Cannot find operations description";
        return;
    }

    QFile file(path);
    setOperationsScheme(&file);
}

} // namespace Plasma

#include "service.moc"

