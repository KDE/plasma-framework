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
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>

//KDE
#include <kdebug.h>
#include <kstandarddirs.h>

static uint connectionId = 0;

//Storage Job implentation
StorageJob::StorageJob(const QString& destination,
                       const QString& operation,
                       const QMap<QString, QVariant>& parameters,
                       QObject *parent)
            : ServiceJob(destination, operation, parameters, parent)
{
    m_db = QSqlDatabase::addDatabase("QSQLITE", QString("plasma-storage-%1").arg(++connectionId));
    m_db.setDatabaseName(KStandardDirs::locateLocal("appdata", "plasma-storage.db"));

    if (!m_db.open()) {
        kWarning() << "Unable to open the plasma storage cache database: " << m_db.lastError();
    } else if (!m_db.tables().contains("data")) {
        QSqlQuery query(m_db);
        query.exec("create table data (id varchar(100) primary key, source varchar(100), data clob, date datetime)");
    }
}

void StorageJob::start()
{
    if (!m_db.isOpen()) {
        return;
    }

    QMap<QString, QVariant> params = parameters();

    if (operationName() == "save") {
      QSqlQuery query(m_db);
      query.prepare("delete from data where key = :id");
      query.bindValue(":id", params["key"].toString());
      query.exec();

      query.prepare("insert into data values(:id, :source, :datavalue, 'now')");
      query.bindValue(":id", params["key"].toString());
      query.bindValue(":source", params["source"].toString());
      query.bindValue(":datavalue", params["data"]);
      const bool success = query.exec();
      setResult(success);
      return;

    } else if (operationName() == "retrieve") {
        QSqlQuery query(m_db);
        query.prepare("delete from data where date < :date");
        QDateTime time(QDateTime::currentDateTime());
        time.addDays(-2);
        query.bindValue(":date", time.toTime_t());
        query.exec();

        query.prepare("select * from data where source=:source");
        query.bindValue(":source", params["source"].toString());
        const bool success = query.exec();

        QHash<QString, QVariant> h;
        if (success) {
            QSqlRecord rec = query.record();
            const int keyColumn = rec.indexOf("id");
            const int dataColumn = rec.indexOf("data");

            while (query.next()) {
                h[query.value(keyColumn).toString()] = query.value(dataColumn).toString();
            }

            setResult(h);
            return;
        } else {
            return;
        }

    } else {
        setError(true);
    }
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
