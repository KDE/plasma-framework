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

//Plasma
#include "applet.h"
#include "dataengine.h"
#include "abstractrunner.h"

static uint connectionId = 0;

//Storage Job implentation
StorageJob::StorageJob(const QString& destination,
                       const QString& operation,
                       const QMap<QString, QVariant>& parameters,
                       QObject *parent)
            : ServiceJob(destination, operation, parameters, parent),
              m_clientName(destination)
{
    m_db = QSqlDatabase::addDatabase("QSQLITE", QString("plasma-storage-%1").arg(++connectionId));
    m_db.setDatabaseName(KStandardDirs::locateLocal("appdata", "plasma-storage.db"));

    if (!m_db.open()) {
        kWarning() << "Unable to open the plasma storage cache database: " << m_db.lastError();
    } else if (!m_db.tables().contains(m_clientName)) {
        QSqlQuery query(m_db);
        //bindValue doesn't seem to be able to replace stuff in create table
        query.prepare(QString("create table ")+m_clientName+" (valueGroup varchar(256), id varchar(256), data clob, date datetime, primary key (valueGroup, id))");
        query.exec();
    }
}

void StorageJob::start()
{
    if (!m_db.isOpen()) {
        return;
    }

    QMap<QString, QVariant> params = parameters();

    QString valueGroup = params["group"].toString();
    if (valueGroup.isEmpty()) {
        valueGroup = "default";
    }


    if (operationName() == "save") {
      QSqlQuery query(m_db);
      query.prepare("delete from "+m_clientName+" where valueGroup=:valueGroup and id = :id");
      query.bindValue(":valueGroup", valueGroup);
      query.bindValue(":id", params["key"].toString());
      query.exec();

      query.prepare("insert into "+m_clientName+" values(:valueGroup, :id, :datavalue, 'now')");
      query.bindValue(":id", params["key"].toString());
      query.bindValue(":valueGroup", valueGroup);
      query.bindValue(":datavalue", params["data"]);
      const bool success = query.exec();
      setResult(success);
      return;

    } else if (operationName() == "retrieve") {
        QSqlQuery query(m_db);
        query.prepare("delete from "+m_clientName+" where date < :date");
        QDateTime time(QDateTime::currentDateTime());
        time.addDays(-2);
        query.bindValue(":date", time.toTime_t());
        query.exec();

        //a bit redundant but should be the faster way with less string concatenation as possible
        if (params["key"].isNull()) {
            query.prepare("select * from "+m_clientName+" where valueGroup=:valueGroup");
            query.bindValue(":valueGroup", valueGroup);
        } else {
            query.prepare("select * from "+m_clientName+" where valueGroup=:valueGroup and key=:key");
            query.bindValue(":valueGroup", valueGroup);
            query.bindValue(":key", params["key"].toString());
        }

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

    } else if (operationName() == "expire") {
        QSqlQuery query(m_db);
        query.prepare("delete from "+m_clientName+" where date < :date");
        QDateTime time(QDateTime::currentDateTime());
        time.addDays(-2);
        query.bindValue(":date", time.toTime_t());
        query.exec();

    } else {
        setError(true);
    }
}

Plasma::ServiceJob* Storage::createJob(const QString &operation, QMap<QString, QVariant> &parameters)
{
    return new StorageJob(m_clientName, operation, parameters, this);
}

//Storage implementation
Storage::Storage(QObject* parent) : Plasma::Service(parent)
{
    //search among parents for an applet or dataengine: if found call the table as its plugin name
    QObject *parentObject = this;

    QString clientName("data");
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
