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

#ifndef STORAGETHREAD_H
#define STORAGETHREAD_H

#include <QThread>
#include <QSqlDatabase>
#include <QWeakPointer>

#include "storage_p.h"

namespace Plasma
{

class StorageThread : public QThread
{
    Q_OBJECT
public:
    explicit StorageThread(QObject *parent = nullptr);
    ~StorageThread();

    void run() Q_DECL_OVERRIDE;

    static Plasma::StorageThread *self();

    void closeDb();

public Q_SLOTS:
    void save(QWeakPointer<StorageJob> caller, const QVariantMap &parameters);
    void retrieve(QWeakPointer<StorageJob> caller, const QVariantMap &parameters);
    void deleteEntry(QWeakPointer<StorageJob> caller, const QVariantMap &parameters);
    void expire(QWeakPointer<StorageJob> caller, const QVariantMap &parameters);

Q_SIGNALS:
    void newResult(StorageJob *caller, const QVariant &result);

private:
    void initializeDb(StorageJob *caller);
    QSqlDatabase m_db;
};

}

#endif
