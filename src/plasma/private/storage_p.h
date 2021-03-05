/*
    storage.h
    SPDX-FileCopyrightText: 2010 Brian Pritchett <batenkaitos@gmail.com>
    SPDX-FileCopyrightText: 2010 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef PLASMA_STORAGE_P_H
#define PLASMA_STORAGE_P_H

#include <QSqlDatabase>

#include <plasma/service.h>
#include <plasma/servicejob.h>

// Begin StorageJob
class StorageJob : public Plasma::ServiceJob
{
    Q_OBJECT
    Q_PROPERTY(QVariantMap data READ data WRITE setData)

public:
    StorageJob(const QString &destination, const QString &operation, const QVariantMap &parameters, QObject *parent = nullptr);
    ~StorageJob() override;
    void setData(const QVariantMap &data);
    QVariantMap data() const;
    void start() override;
    QString clientName() const;

protected Q_SLOTS:
    void resultSlot(StorageJob *job, const QVariant &result);

private:
    QString m_clientName;
    QVariantMap m_data;
};
// End StorageJob

class Storage : public Plasma::Service
{
    Q_OBJECT

public:
    explicit Storage(QObject *parent = nullptr);
    ~Storage() override;

protected:
    Plasma::ServiceJob *createJob(const QString &operation, QVariantMap &parameters) override;

private:
    QString m_clientName;
};

#endif // PLASMA_STORAGE_P_H
