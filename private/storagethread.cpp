/*
 *   Copyright 2011 Marco Martin <mart@kde.org>
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

#include "storagethread_p.h"

#include <QCoreApplication>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlField>
#include <QSqlDriver>
#include <QSqlRecord>

#include <kdebug.h>
#include <kstandarddirs.h>

namespace Plasma
{

class StorageThreadSingleton
{
public:
    StorageThreadSingleton()
    {
    }

   StorageThread self;
};

K_GLOBAL_STATIC(StorageThreadSingleton, privateStorageThreadSelf)


StorageThread::StorageThread(QObject *parent)
    : QThread(parent)
{
}

StorageThread::~StorageThread()
{
    if (!QCoreApplication::closingDown()) {
        QString name = m_db.connectionName();
        QSqlDatabase::removeDatabase(name);
    }
}

Plasma::StorageThread *StorageThread::self()
{
    return &privateStorageThreadSelf->self;
}

void StorageThread::initializeDb(StorageJob *caller)
{
    if (!m_db.open()) {
        m_db = QSqlDatabase::addDatabase("QSQLITE", QString("plasma-storage-%1").arg((quintptr)this));
        m_db.setDatabaseName(KStandardDirs::locateLocal("appdata", "plasma-storage2.db"));
    }

    if (!m_db.open()) {
        kWarning() << "Unable to open the plasma storage cache database: " << m_db.lastError();
    } else if (!m_db.tables().contains(caller->clientName())) {
        QSqlQuery query(m_db);
        query.prepare(QString("create table ") + caller->clientName() + " (valueGroup varchar(256), id varchar(256), txt TEXT, int INTEGER, float REAL, binary BLOB, creationTime datetime, accessTime datetime, primary key (valueGroup, id))");
        if (!query.exec()) {
            kWarning() << "Unable to create table for" << caller->clientName();
            m_db.close();
        }
    }
    m_db.transaction();
}

void StorageThread::save(QWeakPointer<StorageJob> wcaller, const QVariantMap &params)
{
    StorageJob *caller = wcaller.data();
    if (!caller) {
        return;
    }

    initializeDb(caller);
    QString valueGroup = params["group"].toString();
    if (valueGroup.isEmpty()) {
        valueGroup = "default";
    }
    QSqlQuery query(m_db);
    if (params.value("key").toString().isNull()) {
        caller->data().insert(params.value("key").toString(), params.value("data"));
    }

    QHashIterator<QString, QVariant> it(caller->data());

    QString ids;
    while (it.hasNext()) {
        it.next();
        QSqlField field(":id", QVariant::String);
        field.setValue(it.key());
        if (!ids.isEmpty()) {
            ids.append(", ");
        }
        ids.append(m_db.driver()->formatValue(field));
    }

    query.prepare("delete from " + caller->clientName() + " where valueGroup = :valueGroup and id in (" + ids + ");");
    query.bindValue(":valueGroup", valueGroup);

    if (!query.exec()) {
        m_db.commit();
        emit newResult(caller, false);
        return;
    }

    query.prepare("insert into " + caller->clientName() + " values(:valueGroup, :id, :txt, :int, :float, :binary, date('now'), date('now'))");
    query.bindValue(":valueGroup", valueGroup);
    query.bindValue(":txt", QVariant());
    query.bindValue(":int", QVariant());
    query.bindValue(":float", QVariant());
    query.bindValue(":binary", QVariant());

    const QString key = params.value("key").toString();
    if (!key.isEmpty()) {
        caller->data().insert(key, params["data"]);
    }

    it.toFront();
    while (it.hasNext()) {
        it.next();
        //kDebug() << "going to insert" << valueGroup << it.key();
        query.bindValue(":id", it.key());

        QString field;
        bool binary = false;
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
                binary = true;
                field = ":binary";
                break;
            default:
                continue;
                break;
        }

        if (binary) {
            QByteArray b;
            QDataStream ds(&b, QIODevice::WriteOnly);
            ds << it.value();
            query.bindValue(field, ds);
        } else {
            query.bindValue(field, it.value());
        }

        if (!query.exec()) {
            //kDebug() << "query failed:" << query.lastQuery() << query.lastError().text();
            m_db.commit();
            emit newResult(caller, false);
            return;
        }

        query.bindValue(field, QVariant());
    }
    m_db.commit();

    emit newResult(caller, true);
}

void StorageThread::retrieve(QWeakPointer<StorageJob> wcaller, const QVariantMap &params)
{
    StorageJob *caller = wcaller.data();
    if (!caller) {
        return;
    }

    const QString clientName = caller->clientName();
    initializeDb(caller);
    QString valueGroup = params["group"].toString();
    if (valueGroup.isEmpty()) {
        valueGroup = "default";
    }

    QSqlQuery query(m_db);

    //a bit redundant but should be the faster way with less string concatenation as possible
    if (params["key"].toString().isEmpty()) {
        //update modification time
        query.prepare("update " + clientName + " set accessTime=date('now') where valueGroup=:valueGroup");
        query.bindValue(":valueGroup", valueGroup);
        query.exec();

        query.prepare("select * from " + clientName + " where valueGroup=:valueGroup");
        query.bindValue(":valueGroup", valueGroup);
    } else {
        //update modification time
        query.prepare("update " + clientName + " set accessTime=date('now') where valueGroup=:valueGroup and id=:key");
        query.bindValue(":valueGroup", valueGroup);
        query.bindValue(":key", params["key"].toString());
        query.exec();

        query.prepare("select * from " + clientName + " where valueGroup=:valueGroup and id=:key");
        query.bindValue(":valueGroup", valueGroup);
        query.bindValue(":key", params["key"].toString());
    }

    const bool success = query.exec();

    QVariant result;

    if (success) {
        QSqlRecord rec = query.record();
        const int keyColumn = rec.indexOf("id");
        const int textColumn = rec.indexOf("txt");
        const int intColumn = rec.indexOf("int");
        const int floatColumn = rec.indexOf("float");
        const int binaryColumn = rec.indexOf("binary");

        QVariantHash data;
        while (query.next()) {
            const QString key = query.value(keyColumn).toString();
            if (!query.value(textColumn).isNull()) {
                data.insert(key, query.value(textColumn));
            } else if (!query.value(intColumn).isNull()) {
                data.insert(key, query.value(intColumn));
            } else if (!query.value(floatColumn).isNull()) {
                data.insert(key, query.value(floatColumn));
            } else if (!query.value(binaryColumn).isNull()) {
                QByteArray bytes = query.value(binaryColumn).toByteArray();
                QDataStream in(bytes);
                QVariant v(in);
                data.insert(key, v);
            }
        }

        result = data;
    } else {
        result = false;
    }

    emit newResult(caller, result);
}

void StorageThread::deleteEntry(QWeakPointer<StorageJob> wcaller, const QVariantMap &params)
{
    StorageJob *caller = wcaller.data();
    if (!caller) {
        return;
    }

    initializeDb(caller);
    QString valueGroup = params["group"].toString();
    if (valueGroup.isEmpty()) {
        valueGroup = "default";
    }

    QSqlQuery query(m_db);

    if (params["key"].toString().isEmpty()) {
        query.prepare("delete from " + caller->clientName() + " where valueGroup=:valueGroup");
        query.bindValue(":valueGroup", valueGroup);
    } else {
        query.prepare("delete from " + caller->clientName() + " where valueGroup=:valueGroup and id=:key");
        query.bindValue(":valueGroup", valueGroup);
        query.bindValue(":key", params["key"].toString());
    }

    const bool success = query.exec();
    m_db.commit();

    emit newResult(caller, success);
}

void StorageThread::expire(QWeakPointer<StorageJob> wcaller, const QVariantMap &params)
{
    StorageJob *caller = wcaller.data();
    if (!caller) {
        return;
    }

    initializeDb(caller);
    QString valueGroup = params["group"].toString();
    if (valueGroup.isEmpty()) {
        valueGroup = "default";
    }

    QSqlQuery query(m_db);
    if (valueGroup.isEmpty()) {
        query.prepare("delete from " + caller->clientName() + " where accessTime < :date");
        QDateTime time(QDateTime::currentDateTime());
        time.addSecs(-params["age"].toUInt());
        query.bindValue(":date", time.toTime_t());
    } else {
        query.prepare("delete from " + caller->clientName() + " where valueGroup=:valueGroup and accessTime < :date");
        query.bindValue(":valueGroup", valueGroup);
        QDateTime time(QDateTime::currentDateTime());
        time.addSecs(-params["age"].toUInt());
        query.bindValue(":date", time.toTime_t());
    }

    const bool success = query.exec();

    emit newResult(caller, success);
}

void StorageThread::run()
{
    exec();
}

}

#include "storagethread_p.moc"
