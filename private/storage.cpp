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


class RefCountedDatabase
{
public:
    void ref()
    {
        if (m_ref == 0) {
            m_db = QSqlDatabase::addDatabase("QSQLITE", QString("plasma-storage-%1").arg((quintptr)this));
            m_db.setDatabaseName(KStandardDirs::locateLocal("appdata", "plasma-storage2.db"));
        }
        //Q_ASSERT(db.isValid());
        m_ref.ref();
    }

    bool deref()
    {
        //kill the database if the last one in use
        bool last = !m_ref.deref();
        if (last) {
            m_db.close();
            QString name = m_db.connectionName();
            m_db = QSqlDatabase();
            QSqlDatabase::removeDatabase(name);
        }
        return !last;
    }

    inline QSqlDatabase *database()
    {
        return &m_db;
    }

private:
    QSqlDatabase m_db;
    QAtomicInt m_ref;
};

static QThreadStorage<RefCountedDatabase *> s_databasePool;


//Storage Job implentation
StorageJob::StorageJob(const QString& destination,
                       const QString& operation,
                       const QMap<QString, QVariant>& parameters,
                       QObject *parent)
    : ServiceJob(destination, operation, parameters, parent),
      m_clientName(destination)
{
    m_rdb = s_databasePool.localData();
    if (m_rdb == 0) {
        s_databasePool.setLocalData(new RefCountedDatabase);
        m_rdb = s_databasePool.localData();
    }

    m_rdb->ref();

    if (!m_rdb->database()->open()) {
        kWarning() << "Unable to open the plasma storage cache database: " << m_rdb->database()->lastError();
    } else if (!m_rdb->database()->tables().contains(m_clientName)) {
        QSqlQuery query(*m_rdb->database());
        query.prepare(QString("create table ") + m_clientName + " (valueGroup varchar(256), id varchar(256), txt TEXT, int INTEGER, float REAL, binary BLOB, creationTime datetime, accessTime datetime, primary key (valueGroup, id))");
        if (!query.exec()) {
            kWarning() << "Unable to create table for" << m_clientName;
            m_rdb->database()->close();
        }
    }
    Plasma::StorageThread::self()->start();
    qRegisterMetaType<StorageJob *>();
}

StorageJob::~StorageJob()
{
    if (!m_rdb->deref()) {
        s_databasePool.setLocalData(0);
    }
}

void StorageJob::setData(const QVariantHash &data)
{
    m_data = data;
}

QVariantHash StorageJob::data() const
{
    return m_data;
}

void StorageJob::start()
{
    if (!m_rdb->database()->isOpen()) {
        return;
    }

    //FIXME: QHASH
    QMap<QString, QVariant> params = parameters();

    QString valueGroup = params["group"].toString();
    if (valueGroup.isEmpty()) {
        valueGroup = "default";
    }

    //kDebug() << operationName();
    m_rdb->database()->transaction();

    if (operationName() == "save") {
        QSqlQuery query(*m_rdb->database());
        if (params.value("key").toString().isNull()) {
            m_data.insert(params.value("key").toString(), params.value("data"));
        }

        QHashIterator<QString, QVariant> it(m_data);

        QString ids;
        while (it.hasNext()) {
            it.next();
            QSqlField field(":id", QVariant::String);
            field.setValue(it.key());
            if (!ids.isEmpty()) {
                ids.append(", ");
            }
            ids.append(m_rdb->database()->driver()->formatValue(field));
        }

        query.prepare("delete from " + m_clientName + " where valueGroup = :valueGroup and id in (" + ids + ");");
        query.bindValue(":valueGroup", valueGroup);

        if (!query.exec()) {
            m_rdb->database()->commit();
            setResult(false);
            return;
        }

        query.prepare("insert into " + m_clientName + " values(:valueGroup, :id, :txt, :int, :float, :binary, date('now'), date('now'))");
        query.bindValue(":valueGroup", valueGroup);
        query.bindValue(":txt", QVariant());
        query.bindValue(":int", QVariant());
        query.bindValue(":float", QVariant());
        query.bindValue(":binary", QVariant());

        const QString key = params.value("key").toString();
        if (!key.isEmpty()) {
            m_data.insert(key, params["data"]);
        }

        it.toFront();
        while (it.hasNext()) {
            it.next();
            //kDebug() << "going to insert" << valueGroup << it.key();
            query.bindValue(":id", it.key());

            QString field;
            switch (QMetaType::Type(it.value().type())) {
                case QVariant::String:
                    field = ":txt";
                    break;
                case QVariant::Int:
                    field = ":int";
                    break;
                case QVariant::Double:
                case QMetaType::Float:
                    field = ":float";
                    break;
                case QVariant::ByteArray:
                    field = ":binary";
                    break;
                default:
                    continue;
                    break;
            }

            query.bindValue(field, it.value());

            if (!query.exec()) {
                //kDebug() << "query failed:" << query.lastQuery() << query.lastError().text();
                m_rdb->database()->commit();
                setResult(false);
                return;
            }

            query.bindValue(field, QVariant());
        }

        setResult(true);
    } else if (operationName() == "retrieve") {
        QSqlQuery query(*m_rdb->database());
        
        QMetaObject::invokeMethod(Plasma::StorageThread::self(), "retrieve", Qt::QueuedConnection, Q_ARG(StorageJob *, this), Q_ARG(const QVariantMap&, params));

        //a bit redundant but should be the faster way with less string concatenation as possible
        if (params["key"].toString().isEmpty()) {
            //update modification time
            query.prepare("update " + m_clientName + " set accessTime=date('now') where valueGroup=:valueGroup");
            query.bindValue(":valueGroup", valueGroup);
            query.exec();

            query.prepare("select * from " + m_clientName + " where valueGroup=:valueGroup");
            query.bindValue(":valueGroup", valueGroup);
        } else {
            //update modification time
            query.prepare("update " + m_clientName + " set accessTime=date('now') where valueGroup=:valueGroup and id=:key");
            query.bindValue(":valueGroup", valueGroup);
            query.bindValue(":key", params["key"].toString());
            query.exec();

            query.prepare("select * from " + m_clientName + " where valueGroup=:valueGroup and id=:key");
            query.bindValue(":valueGroup", valueGroup);
            query.bindValue(":key", params["key"].toString());
        }

        const bool success = query.exec();

        m_data.clear();
        if (success) {
            QSqlRecord rec = query.record();
            const int keyColumn = rec.indexOf("id");
            const int textColumn = rec.indexOf("txt");
            const int intColumn = rec.indexOf("int");
            const int floatColumn = rec.indexOf("float");
            const int binaryColumn = rec.indexOf("binary");

            while (query.next()) {
                const QString key = query.value(keyColumn).toString();
                if (!query.value(textColumn).isNull()) {
                    m_data.insert(key, query.value(textColumn));
                } else if (!query.value(intColumn).isNull()) {
                    m_data.insert(key, query.value(intColumn));
                } else if (!query.value(floatColumn).isNull()) {
                    m_data.insert(key, query.value(floatColumn));
                } else if (!query.value(binaryColumn).isNull()) {
                    m_data.insert(key, query.value(binaryColumn));
                }
            }

            setResult(m_data);
        } else {
            setResult(false);
        }
    } else if (operationName() == "delete") {
        QSqlQuery query(*m_rdb->database());

        if (params["key"].toString().isEmpty()) {
            query.prepare("delete from "+m_clientName+" where valueGroup=:valueGroup");
            query.bindValue(":valueGroup", valueGroup);
        } else {
            query.prepare("delete from "+m_clientName+" where valueGroup=:valueGroup and id=:key");
            query.bindValue(":valueGroup", valueGroup);
            query.bindValue(":key", params["key"].toString());
        }

        const bool success = query.exec();
        setResult(success);

    } else if (operationName() == "expire") {
        QSqlQuery query(*m_rdb->database());
        if (valueGroup.isEmpty()) {
            query.prepare("delete from "+m_clientName+" where accessTime < :date");
            QDateTime time(QDateTime::currentDateTime());
            time.addSecs(-params["age"].toUInt());
            query.bindValue(":date", time.toTime_t());
        } else {
            query.prepare("delete from "+m_clientName+" where valueGroup=:valueGroup and accessTime < :date");
            query.bindValue(":valueGroup", valueGroup);
            QDateTime time(QDateTime::currentDateTime());
            time.addSecs(-params["age"].toUInt());
            query.bindValue(":date", time.toTime_t());
        }

        const bool success = query.exec();
        setResult(success);

    } else {
        setError(true);
    }
    m_rdb->database()->commit();
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
