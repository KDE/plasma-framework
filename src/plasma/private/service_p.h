/*
    SPDX-FileCopyrightText: 2008 Aaron Seigo <aseigo@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef SERVICE_P_H
#define SERVICE_P_H

#include "service.h"
#include "servicejob.h"

#include <QMap>
#include <QMultiHash>
#include <QSet>
#include <QWidget>

#include <KLocalizedString>

namespace Plasma
{
class NullServiceJob : public ServiceJob
{
public:
    NullServiceJob(const QString &destination, const QString &operation, QObject *parent)
        : ServiceJob(destination, operation, QVariantMap(), parent)
    {
    }

    void start() override
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
        setName(QStringLiteral("NullService"));
    }

    ServiceJob *createJob(const QString &operation, QVariantMap &) override
    {
        return new NullServiceJob(destination(), operation, this);
    }
};

class ServicePrivate
{
public:
    ServicePrivate(Service *service)
        : q(service)
    {
    }

    ~ServicePrivate()
    {
    }

    void associatedWidgetDestroyed(QObject *obj);

    void associatedItemDestroyed(QObject *obj);

    Service *q;
    QString destination;
    QString name;
    QString resourcename;
    QMap<QString, QVariantMap> operationsMap;
    QSet<QString> disabledOperations;
};

} // namespace Plasma

#endif
