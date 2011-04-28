/////////////////////////////////////////////////////////////////////////
// storage.cpp                                                         //
//                                                                     //
// Copyright (C)  2010  Brian Pritchett <batenkaitos@gmail.com>        //
// Copyright (C)  2010  Marco Martin <mart@kde.org>                    //
//                                                                     //
// This library is free software; you can redistribute it and/or       //
// modify it under the terms of the GNU Lesser General Public          //
// License as published by the Free Software Foundation; either        //
// version 2.1 of the License, or (at your option) any later version.  //
//                                                                     //
// This library is distributed in the hope that it will be useful,     //
// but WITHOUT ANY WARRANTY; without even the implied warranty of      //
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU   //
// Lesser General Public License for more details.                     //
//                                                                     //
// You should have received a copy of the GNU Lesser General Public    //
// License along with this library; if not, write to the Free Software //
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA       //
// 02110-1301  USA                                                     //
/////////////////////////////////////////////////////////////////////////

#include "private/storage_p.h"

//Qt
#include <QSqlDriver>
#include <QSqlError>
#include <QSqlField>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QThreadStorage>

//KDE
#include <kdebug.h>
#include <kstandarddirs.h>

//Plasma
#include "applet.h"
#include "dataengine.h"
#include "abstractrunner.h"
#include "storagethread_p.h"


static QThreadStorage<RefCountedDatabase *> s_databasePool;


//Storage Job implentation
StorageJob::StorageJob(const QString& destination,
                       const QString& operation,
                       const QMap<QString, QVariant>& parameters,
                       QObject *parent)
    : ServiceJob(destination, operation, parameters, parent),
      m_clientName(destination)
{
    Plasma::StorageThread::self()->start();
    connect(Plasma::StorageThread::self(), SIGNAL(newResult(StorageJob *, const QVariant &)), this, SLOT(resultSlot(StorageJob *, const QVariant &)));
    qRegisterMetaType<StorageJob *>();
    qRegisterMetaType<QWeakPointer<StorageJob> >();
}

StorageJob::~StorageJob()
{
}

void StorageJob::setData(const QVariantHash &data)
{
    m_data = data;
}

QVariantHash StorageJob::data() const
{
    return m_data;
}

QString StorageJob::clientName() const
{
    return m_clientName;
}


void StorageJob::start()
{
    //FIXME: QHASH
    QMap<QString, QVariant> params = parameters();

    QString valueGroup = params["group"].toString();
    if (valueGroup.isEmpty()) {
        valueGroup = "default";
    }

    QWeakPointer<StorageJob> me(this);
    if (operationName() == "save") {
        QMetaObject::invokeMethod(Plasma::StorageThread::self(), "save", Qt::QueuedConnection, Q_ARG(QWeakPointer<StorageJob>, me), Q_ARG(const QVariantMap&, params));
    } else if (operationName() == "retrieve") {
        QMetaObject::invokeMethod(Plasma::StorageThread::self(), "retrieve", Qt::QueuedConnection, Q_ARG(QWeakPointer<StorageJob>, me), Q_ARG(const QVariantMap&, params));
    } else if (operationName() == "delete") {
        QMetaObject::invokeMethod(Plasma::StorageThread::self(), "delete", Qt::QueuedConnection, Q_ARG(QWeakPointer<StorageJob>, me), Q_ARG(const QVariantMap&, params));
    } else if (operationName() == "expire") {
        QMetaObject::invokeMethod(Plasma::StorageThread::self(), "expire", Qt::QueuedConnection, Q_ARG(QWeakPointer<StorageJob>, me), Q_ARG(const QVariantMap&, params));
    } else {
        setError(true);
        setResult(false);
    }
}

void StorageJob::resultSlot(StorageJob *job, const QVariant &result)
{
    if (job == this) {
        if (result.type() == QVariant::Hash) {
            m_data = result.toHash();
        }
        setResult(result);
    }
}

Plasma::ServiceJob* Storage::createJob(const QString &operation, QMap<QString, QVariant> &parameters)
{
    if (m_clientName.isEmpty()) {
        return 0;
    }

    return new StorageJob(m_clientName, operation, parameters, this);
}

//Storage implementation
Storage::Storage(QObject* parent)
    : Plasma::Service(parent),
      m_clientName("data")
{
    //search among parents for an applet or dataengine: if found call the table as its plugin name
    QObject *parentObject = this;

    while ((parentObject = parentObject->parent())) {
        Plasma::Applet *applet = qobject_cast<Plasma::Applet *>(parentObject);
        if (applet) {
            m_clientName = applet->pluginName();
            break;
        }

        Plasma::DataEngine *engine = qobject_cast<Plasma::DataEngine *>(parentObject);
        if (engine) {
            m_clientName = engine->pluginName();
            break;
        }

        Plasma::AbstractRunner *runner = qobject_cast<Plasma::AbstractRunner *>(parentObject);
        if (runner) {
            m_clientName = runner->id();
            break;
        }
    }

    m_clientName = m_clientName.replace('.', "_");
    m_clientName = m_clientName.replace('-', "_");

    setName("storage");
}

Storage::~Storage()
{
}

#include "storage_p.moc"
