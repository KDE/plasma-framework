/////////////////////////////////////////////////////////////////////////
// storage.cpp                                                         //
//                                                                     //
// Copyright (C)  2010  Brian Pritchett <batenkaitos@gmail.com>        //
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
#include <kdebug.h>

//Storage Job implentation
StorageJob::StorageJob(const QString& destination,
                       const QString& operation,
                       const QMap<QString, QVariant>& parameters,
                       QObject *parent)
            : ServiceJob(destination, operation, parameters, parent)
{
}

void StorageJob::start()
{
    static QStringList* keyList = NULL;
    QMap<QString, QVariant> params = parameters();
    QString source = params["source"].toString();
    KConfig config("plasma-storagerc");
    KConfigGroup destinationGroup(&config, destination());
    KConfigGroup sourceGroup(&destinationGroup, source);
    KConfigGroup data(&sourceGroup, "Data");

    if (operationName() == "save") {
    //    KConfigGroup metaData(&sourceGroup, "MetaData");
    //    metaData.writeEntry("TimeStamp", QDateTime::currentDateTime());
        data.writeEntry(params["key"].toString(), params["data"]);
        setResult(true);
    } else if (operationName() == "retrieve") {
        if (keyList == NULL) {
            kDebug() << "NULL";
            keyList = new QStringList;
            *keyList = data.keyList();
        }
        if (keyList->isEmpty()) {
           setError(1);
           delete keyList;
           keyList = NULL;
        } else {
           QString key = keyList->first();
           QHash<QString, QVariant> h;
           QVariant v(data.readEntry(key));
           h["key"] = key;
           h["data"] = v;
           keyList->pop_front();
           setResult(h);
       }
    }
    setError(1);
}

Plasma::ServiceJob* Storage::createJob(const QString &operation, QMap<QString, QVariant> &parameters)
{
    return new StorageJob(m_serviceName, operation, parameters);
}

//Storage implementation
Storage::Storage(const QString& destination, QObject* parent) : Plasma::Service(parent)
{
    m_serviceName = destination;
    setName("storage");
}

Storage::~Storage()
{
}

#include "storage_p.moc"
