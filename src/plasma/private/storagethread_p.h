/*
    SPDX-FileCopyrightText: 2011 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef STORAGETHREAD_H
#define STORAGETHREAD_H

#include <QPointer>
#include <QSqlDatabase>
#include <QThread>

#include "storage_p.h"

namespace Plasma
{
class StorageThread : public QThread
{
    Q_OBJECT
public:
    explicit StorageThread(QObject *parent = nullptr);
    ~StorageThread() override;

    void run() override;

    static Plasma::StorageThread *self();

    void closeDb();

public Q_SLOTS:
    void save(QPointer<StorageJob> caller, const QVariantMap &parameters);
    void retrieve(QPointer<StorageJob> caller, const QVariantMap &parameters);
    void deleteEntry(QPointer<StorageJob> caller, const QVariantMap &parameters);
    void expire(QPointer<StorageJob> caller, const QVariantMap &parameters);

Q_SIGNALS:
    void newResult(StorageJob *caller, const QVariant &result);

private:
    void initializeDb(StorageJob *caller);
    QSqlDatabase m_db;
};

}

#endif
