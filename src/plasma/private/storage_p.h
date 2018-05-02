/////////////////////////////////////////////////////////////////////////
// storage.h                                                           //
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

#ifndef PLASMA_STORAGE_H
#define PLASMA_STORAGE_H

#include <QSqlDatabase>

#include <plasma/service.h>
#include <plasma/servicejob.h>

//Begin StorageJob
class StorageJob : public Plasma::ServiceJob
{
    Q_OBJECT
    Q_PROPERTY(QVariantMap data READ data WRITE setData)

public:
    StorageJob(const QString &destination,
               const QString &operation,
               const QVariantMap &parameters,
               QObject *parent = nullptr);
    ~StorageJob();
    void setData(const QVariantMap &data);
    QVariantMap data() const;
    void start() Q_DECL_OVERRIDE;
    QString clientName() const;

protected Q_SLOTS:
    void resultSlot(StorageJob *job, const QVariant &result);

private:
    QString m_clientName;
    QVariantMap m_data;
};
//End StorageJob

Q_DECLARE_METATYPE(StorageJob *)
Q_DECLARE_METATYPE(QWeakPointer<StorageJob>)

class Storage : public Plasma::Service
{
    Q_OBJECT

public:
    Storage(QObject *parent = nullptr);
    ~Storage();

protected:
    Plasma::ServiceJob *createJob(const QString &operation, QVariantMap &parameters) Q_DECL_OVERRIDE;

private:
    QString m_clientName;
};

#endif //PLASMA_STORAGE_H
