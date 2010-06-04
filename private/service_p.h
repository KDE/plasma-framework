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

#ifndef SERVICE_P_H
#define SERVICE_P_H

#include "servicejob.h"
#include "service.h"

#include <QGraphicsWidget>
#include <QMap>
#include <QMultiHash>
#include <QWidget>
#include <QSet>

#include <dnssd/publicservice.h>
#include <dnssd/servicebrowser.h>

#include "plasma/configloader.h"
#include "accessmanager_p.h"

namespace Plasma
{

class ConfigLoader;
class ServiceProvider;

class NullServiceJob : public ServiceJob
{
public:
    NullServiceJob(const QString &destination, const QString &operation, QObject *parent)
        : ServiceJob(destination, operation, QMap<QString, QVariant>(), parent)
    {
    }

    void start()
    {
        setErrorText(i18nc("Error message, tried to start an invalid service", "Invalid (null) service, can not perform any operations."));
        emitResult();
    }
};

class NullService : public Service
{
public:
    NullService(const QString &target, QObject *parent)
        : Service(parent)
    {
        setDestination(target);
        setName("NullService");
    }

    ServiceJob *createJob(const QString &operation, QMap<QString, QVariant> &)
    {
        return new NullServiceJob(destination(), operation, this);
    }
};

class ServicePrivate
{
public:
    ServicePrivate(Service *service)
        : q(service),
          config(0),
          dummyConfig(0),
          publicService(0),
          serviceProvider(0)
    {
    }

    ~ServicePrivate()
    {
        delete config;
        delete dummyConfig;
    }

    void jobFinished(KJob *job);

    void associatedWidgetDestroyed(QObject *obj);

    void associatedGraphicsWidgetDestroyed(QObject *obj);

    void publish(AnnouncementMethods methods, const QString &name,
                 PackageMetadata metadata = PackageMetadata());

    void unpublish();

    bool isPublished() const;

    KConfigGroup dummyGroup();

    Service *q;
    QString destination;
    QString name;
    QString resourcename;
    ConfigLoader *config;
    KConfig *dummyConfig;
    DNSSD::PublicService *publicService;
    ServiceProvider *serviceProvider;
    QMultiHash<QWidget *, QString> associatedWidgets;
    QMultiHash<QGraphicsWidget *, QString> associatedGraphicsWidgets;
    QSet<QString> disabledOperations;
};

} // namespace Plasma

#endif

