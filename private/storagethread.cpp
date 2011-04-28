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
    QString name = m_db.connectionName();
    QSqlDatabase::removeDatabase(name);
}

Plasma::StorageThread *self()
{
    return &privateStorageThreadSelf->self;
}

void StorageThread::save(ServiceJob* caller, const QVariantMap &params)
{
    bool success = true;

    emit newResult(caller, success);
}

void StorageThread::retrieve(ServiceJob* caller, const QVariantMap &params)
{

    bool success = true;
    emit newResult(caller, success);
}

void StorageThread::deleteEntry(ServiceJob* caller, const QVariantMap &params)
{

    const bool success = true;
    emit newResult(caller, success);
}

void StorageThread::expire(ServiceJob* caller, const QVariantMap &params)
{
    
    bool success = true;
    emit newResult(caller, success);
}


void StorageThread::run()
{
    
}

}

#include "storagethread_p.moc"
