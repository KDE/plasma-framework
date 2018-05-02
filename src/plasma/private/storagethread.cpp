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
#include <QDir>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlField>
#include <QSqlDriver>
#include <QSqlRecord>
#include <QDataStream>

#include <QDebug>
#include <qstandardpaths.h>
#include "debug_p.h"

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

Q_GLOBAL_STATIC(StorageThreadSingleton, privateStorageThreadSelf)

static void closeConnection()
{
    StorageThread::self()->closeDb();
    StorageThread::self()->quit();
}

StorageThread::StorageThread(QObject *parent)
    : QThread(parent)
{
    qAddPostRoutine(closeConnection);
}

StorageThread::~StorageThread()
{
}

Plasma::StorageThread *StorageThread::self()
{
    return &privateStorageThreadSelf()->self;
}

void StorageThread::closeDb()
{
    QString name = m_db.connectionName();
    QSqlDatabase::removeDatabase(name);
    m_db = QSqlDatabase();
}

void StorageThread::initializeDb(StorageJob *caller)
{
    if (!m_db.open()) {
        m_db = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), QStringLiteral("plasma-storage-%1").arg((quintptr)this));
        const QString storageDir = QStandardPaths::writableLocation(QStandardPaths::DataLocation);
        QDir().mkpath(storageDir);
        m_db.setDatabaseName(storageDir + QLatin1Char('/') + QStringLiteral("plasma-storage2.db"));
    }

    if (!m_db.open()) {
        qCWarning(LOG_PLASMA) << "Unable to open the plasma storage cache database: " << m_db.lastError();
    } else if (!m_db.tables().contains(caller->clientName())) {
        QSqlQuery query(m_db);
        query.prepare(QStringLiteral("create table ") + caller->clientName() + QStringLiteral(" (valueGroup varchar(256), id varchar(256), txt TEXT, int INTEGER, float REAL, binary BLOB, creationTime datetime, accessTime datetime, primary key (valueGroup, id))"));
        if (!query.exec()) {
            qCWarning(LOG_PLASMA) << "Unable to create table for" << caller->clientName();
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
    QString valueGroup = params[QStringLiteral("group")].toString();
    if (valueGroup.isEmpty()) {
        valueGroup = QStringLiteral("default");
    }
    QSqlQuery query(m_db);
    if (params.value(QStringLiteral("key")).toString().isNull()) {
        caller->data().insert(params.value(QStringLiteral("key")).toString(), params.value(QStringLiteral("data")));
    }

    QMapIterator<QString, QVariant> it(caller->data());

    QString ids;
    while (it.hasNext()) {
        it.next();
        QSqlField field(QStringLiteral(":id"), QVariant::String);
        field.setValue(it.key());
        if (!ids.isEmpty()) {
            ids.append(QStringLiteral(", "));
        }
        ids.append(m_db.driver()->formatValue(field));
    }

    query.prepare(QStringLiteral("delete from ") + caller->clientName() + QStringLiteral(" where valueGroup = :valueGroup and id in (") + ids + QStringLiteral(");"));
    query.bindValue(QStringLiteral(":valueGroup"), valueGroup);

    if (!query.exec()) {
        m_db.commit();
        emit newResult(caller, false);
        return;
    }

    query.prepare(QStringLiteral("insert into ") + caller->clientName() + QStringLiteral(" values(:valueGroup, :id, :txt, :int, :float, :binary, date('now'), date('now'))"));
    query.bindValue(QStringLiteral(":valueGroup"), valueGroup);
    query.bindValue(QStringLiteral(":txt"), QVariant());
    query.bindValue(QStringLiteral(":int"), QVariant());
    query.bindValue(QStringLiteral(":float"), QVariant());
    query.bindValue(QStringLiteral(":binary"), QVariant());

    const QString key = params.value(QStringLiteral("key")).toString();
    if (!key.isEmpty()) {
        QVariantMap data = caller->data();
        data.insert(key, params[QStringLiteral("data")]);
        caller->setData(data);
    }

    it.toFront();
    while (it.hasNext()) {
        it.next();
        //qCDebug(LOG_PLASMA) << "going to insert" << valueGroup << it.key();
        query.bindValue(QStringLiteral(":id"), it.key());

        QString field;
        bool binary = false;
        switch (QMetaType::Type(it.value().type())) {
        case QVariant::String:
            field = QStringLiteral(":txt");
            break;
        case QVariant::Int:
            field = QStringLiteral(":int");
            break;
        case QVariant::Double:
        case QMetaType::Float:
            field = QStringLiteral(":float");
            break;
        case QVariant::ByteArray:
            binary = true;
            field = QStringLiteral(":binary");
            break;
        default:
            continue;
        }

        if (binary) {
            QByteArray b;
            QDataStream ds(&b, QIODevice::WriteOnly);
            ds << it.value();
            query.bindValue(field, b);
        } else {
            query.bindValue(field, it.value());
        }

        if (!query.exec()) {
            //qCDebug(LOG_PLASMA) << "query failed:" << query.lastQuery() << query.lastError().text();
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
    QString valueGroup = params[QStringLiteral("group")].toString();
    if (valueGroup.isEmpty()) {
        valueGroup = QStringLiteral("default");
    }

    QSqlQuery query(m_db);

    //a bit redundant but should be the faster way with less string concatenation as possible
    if (params[QStringLiteral("key")].toString().isEmpty()) {
        //update modification time
        query.prepare(QStringLiteral("update ") + clientName + QStringLiteral(" set accessTime=date('now') where valueGroup=:valueGroup"));
        query.bindValue(QStringLiteral(":valueGroup"), valueGroup);
        query.exec();

        query.prepare(QStringLiteral("select * from ") + clientName + QStringLiteral(" where valueGroup=:valueGroup"));
        query.bindValue(QStringLiteral(":valueGroup"), valueGroup);
    } else {
        //update modification time
        query.prepare(QStringLiteral("update ") + clientName + QStringLiteral(" set accessTime=date('now') where valueGroup=:valueGroup and id=:key"));
        query.bindValue(QStringLiteral(":valueGroup"), valueGroup);
        query.bindValue(QStringLiteral(":key"), params[QStringLiteral("key")].toString());
        query.exec();

        query.prepare(QStringLiteral("select * from ") + clientName + QStringLiteral(" where valueGroup=:valueGroup and id=:key"));
        query.bindValue(QStringLiteral(":valueGroup"), valueGroup);
        query.bindValue(QStringLiteral(":key"), params[QStringLiteral("key")].toString());
    }

    const bool success = query.exec();

    QVariant result;

    if (success) {
        QSqlRecord rec = query.record();
        const int keyColumn = rec.indexOf(QStringLiteral("id"));
        const int textColumn = rec.indexOf(QStringLiteral("txt"));
        const int intColumn = rec.indexOf(QStringLiteral("int"));
        const int floatColumn = rec.indexOf(QStringLiteral("float"));
        const int binaryColumn = rec.indexOf(QStringLiteral("binary"));

        QVariantMap data;
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
    QString valueGroup = params[QStringLiteral("group")].toString();
    if (valueGroup.isEmpty()) {
        valueGroup = QStringLiteral("default");
    }

    QSqlQuery query(m_db);

    if (params[QStringLiteral("key")].toString().isEmpty()) {
        query.prepare(QStringLiteral("delete from ") + caller->clientName() + QStringLiteral(" where valueGroup=:valueGroup"));
        query.bindValue(QStringLiteral(":valueGroup"), valueGroup);
    } else {
        query.prepare(QStringLiteral("delete from ") + caller->clientName() + QStringLiteral(" where valueGroup=:valueGroup and id=:key"));
        query.bindValue(QStringLiteral(":valueGroup"), valueGroup);
        query.bindValue(QStringLiteral(":key"), params[QStringLiteral("key")].toString());
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
    QString valueGroup = params[QStringLiteral("group")].toString();
    if (valueGroup.isEmpty()) {
        valueGroup = QStringLiteral("default");
    }

    QSqlQuery query(m_db);
    if (valueGroup.isEmpty()) {
        query.prepare(QStringLiteral("delete from ") + caller->clientName() + QStringLiteral(" where accessTime < :date"));
        QDateTime time(QDateTime::currentDateTime().addSecs(-params[QStringLiteral("age")].toUInt()));
        query.bindValue(QStringLiteral(":date"), time.toTime_t());
    } else {
        query.prepare(QStringLiteral("delete from ") + caller->clientName() + QStringLiteral(" where valueGroup=:valueGroup and accessTime < :date"));
        query.bindValue(QStringLiteral(":valueGroup"), valueGroup);
        QDateTime time(QDateTime::currentDateTime().addSecs(-params[QStringLiteral("age")].toUInt()));
        query.bindValue(QStringLiteral(":date"), time.toTime_t());
    }

    const bool success = query.exec();

    emit newResult(caller, success);
}

void StorageThread::run()
{
    exec();
}

}

#include "moc_storagethread_p.cpp"
