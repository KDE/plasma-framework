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
#include "private/service_p.h"

#include "config-plasma.h"

#include <QFile>
#include <QTimer>

#include <QDebug>
#include <kservice.h>
#include <ksharedconfig.h>
#include <KConfigLoader>
#include <KConfigSkeleton>

#include <qstandardpaths.h>

#include "version.h"
#include "pluginloader.h"
#include "debug_p.h"

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
    delete d;
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
    if (d->operationsMap.isEmpty()) {
#ifndef NDEBUG
        // qCDebug(LOG_PLASMA) << "No valid operations scheme has been registered";
#endif
        return QStringList();
    }

    return d->operationsMap.keys();
}

QVariantMap Service::operationDescription(const QString &operationName)
{
    if (d->operationsMap.isEmpty()) {
#ifndef NDEBUG
        // qCDebug(LOG_PLASMA) << "No valid operations scheme has been registered";
#endif
        return QVariantMap();
    }

    //qCDebug(LOG_PLASMA) << "operation" << operationName
    //         << "requested, has keys" << d->operationsMap.keys();
    return d->operationsMap.value(operationName);
}

ServiceJob *Service::startOperationCall(const QVariantMap &description, QObject *parent)
{
    // TODO: nested groups?
    ServiceJob *job = nullptr;
    const QString op = !description.isEmpty() ? description.value(QStringLiteral("_name")).toString() : QString();

    if (d->operationsMap.isEmpty()) {
#ifndef NDEBUG
        // qCDebug(LOG_PLASMA) << "No valid operations scheme has been registered";
#endif
    } else if (!op.isEmpty() && d->operationsMap.contains(op)) {
        if (d->disabledOperations.contains(op)) {
#ifndef NDEBUG
            // qCDebug(LOG_PLASMA) << "Operation" << op << "is disabled";
#endif
        } else {
            QVariantMap map = description;
            job = createJob(op, map);
        }
    } else {
#ifndef NDEBUG
        // qCDebug(LOG_PLASMA) << op << "is not a valid group; valid groups are:" << d->operationsMap.keys();
#endif
    }

    if (!job) {
        job = new NullServiceJob(d->destination, op, this);
    }

    job->setParent(parent ? parent : this);
    QTimer::singleShot(0, job, SLOT(autoStart()));
    return job;
}

QString Service::name() const
{
    return d->name;
}

void Service::setName(const QString &name)
{
    d->name = name;

    // now reset the config, which may be based on our name
    d->operationsMap.clear();

    registerOperationsScheme();

    emit serviceReady(this);
}

void Service::setOperationEnabled(const QString &operation, bool enable)
{
    if (d->operationsMap.isEmpty() || !d->operationsMap.contains(operation)) {
        return;
    }

    if (enable) {
        d->disabledOperations.remove(operation);
    } else {
        d->disabledOperations.insert(operation);
    }

    emit operationEnabledChanged(operation, enable);
}

bool Service::isOperationEnabled(const QString &operation) const
{
    return d->operationsMap.contains(operation) && !d->disabledOperations.contains(operation);
}

void Service::setOperationsScheme(QIODevice *xml)
{
    d->operationsMap.clear();

    // /dev/null is because I need to pass a filename argument to construct a
    //  KSharedConfig. We need a config object for the config loader even
    //  though we dont' actually want to use any config parts from it,
    //  we just want to share the KConfigLoader XML parsing.
    KSharedConfigPtr config = KSharedConfig::openConfig(QStringLiteral("/dev/null"), KConfig::SimpleConfig);
    KConfigLoader loader(config, xml);

    const auto groupList = loader.groupList();
    for (const QString &group : groupList) {
        d->operationsMap[group][QStringLiteral("_name")] = group;
    }
    const auto itemsList = loader.items();
    for (KConfigSkeletonItem *item : itemsList) {
        d->operationsMap[item->group()][item->key()] = item->property();
    }
}

void Service::registerOperationsScheme()
{
    if (!d->operationsMap.isEmpty()) {
        // we've already done our job. let's go home.
        return;
    }

    if (d->name.isEmpty()) {
#ifndef NDEBUG
        // qCDebug(LOG_PLASMA) << "No name found";
#endif
        return;
    }

    const QString path = QStandardPaths::locate(QStandardPaths::GenericDataLocation, QStringLiteral(PLASMA_RELATIVE_DATA_INSTALL_DIR "/services/") + d->name + QStringLiteral(".operations"));

    if (path.isEmpty()) {
#ifndef NDEBUG
        // qCDebug(LOG_PLASMA) << "Cannot find operations description:" << d->name << ".operations";
#endif
        return;
    }

    QFile file(path);
    setOperationsScheme(&file);
}

} // namespace Plasma

#include "moc_service.cpp"
