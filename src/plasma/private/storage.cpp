/*
    storage.cpp
    SPDX-FileCopyrightText: 2010 Brian Pritchett <batenkaitos@gmail.com>
    SPDX-FileCopyrightText: 2010 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "private/storage_p.h"

// Qt
#include <QSqlDriver>
#include <QSqlError>
#include <QSqlField>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QThreadStorage>

// KDE
#include <QDebug>

// Plasma
#include "applet.h"
#include "dataengine.h"
#include "storagethread_p.h"

StorageJob::StorageJob(const QString &destination, const QString &operation, const QVariantMap &parameters, QObject *parent)
    : ServiceJob(destination, operation, parameters, parent)
    , m_clientName(destination)
{
    Plasma::StorageThread::self()->start();
    connect(Plasma::StorageThread::self(), &Plasma::StorageThread::newResult, this, &StorageJob::resultSlot);
    qRegisterMetaType<StorageJob *>();
    qRegisterMetaType<QPointer<StorageJob>>();
}

StorageJob::~StorageJob()
{
}

void StorageJob::setData(const QVariantMap &data)
{
    m_data = data;
}

QVariantMap StorageJob::data() const
{
    return m_data;
}

QString StorageJob::clientName() const
{
    return m_clientName;
}

void StorageJob::start()
{
    // FIXME: QHASH
    QVariantMap params = parameters();

    QString valueGroup = params[QStringLiteral("group")].toString();
    if (valueGroup.isEmpty()) {
        valueGroup = QStringLiteral("default");
    }

    QPointer<StorageJob> me(this);
    if (operationName() == QLatin1String("save")) {
        QMetaObject::invokeMethod(Plasma::StorageThread::self(), "save", Qt::QueuedConnection, Q_ARG(QPointer<StorageJob>, me), Q_ARG(QVariantMap, params));
    } else if (operationName() == QLatin1String("retrieve")) {
        QMetaObject::invokeMethod(Plasma::StorageThread::self(), "retrieve", Qt::QueuedConnection, Q_ARG(QPointer<StorageJob>, me), Q_ARG(QVariantMap, params));
    } else if (operationName() == QLatin1String("delete")) {
        QMetaObject::invokeMethod(Plasma::StorageThread::self(),
                                  "deleteEntry",
                                  Qt::QueuedConnection,
                                  Q_ARG(QPointer<StorageJob>, me),
                                  Q_ARG(QVariantMap, params));
    } else if (operationName() == QLatin1String("expire")) {
        QMetaObject::invokeMethod(Plasma::StorageThread::self(), "expire", Qt::QueuedConnection, Q_ARG(QPointer<StorageJob>, me), Q_ARG(QVariantMap, params));
    } else {
        setError(true);
        setResult(false);
    }
}

void StorageJob::resultSlot(StorageJob *job, const QVariant &result)
{
    if (job == this) {
        if (result.type() == QVariant::Map) {
            m_data = result.toMap();
        }
        setResult(result);
    }
}

Plasma::ServiceJob *Storage::createJob(const QString &operation, QVariantMap &parameters)
{
    if (m_clientName.isEmpty()) {
        return nullptr;
    }

    return new StorageJob(m_clientName, operation, parameters, this);
}

// Storage implementation
Storage::Storage(QObject *parent)
    : Plasma::Service(parent)
    , m_clientName(QStringLiteral("data"))
{
    // search among parents for an applet or dataengine: if found call the table as its plugin name
    QObject *parentObject = this;

    while ((parentObject = parentObject->parent())) {
        Plasma::Applet *applet = qobject_cast<Plasma::Applet *>(parentObject);
        if (applet) {
            m_clientName = applet->pluginMetaData().pluginId();
            break;
        }

        Plasma::DataEngine *engine = qobject_cast<Plasma::DataEngine *>(parentObject);
        if (engine) {
            m_clientName = engine->metadata().pluginId();
            break;
        }
    }

    m_clientName.replace(QLatin1Char('.'), QLatin1Char('_'));
    m_clientName.replace(QLatin1Char('-'), QLatin1Char('_'));

    setName(QStringLiteral("storage"));
}

Storage::~Storage()
{
}

#include "moc_storage_p.cpp"
