/////////////////////////////////////////////////////////////////////////
// storage.h                                                           //
//                                                                     //
// Copyright (C)  2010  Brian Pritchett <batenkaitos@gmail.com>        //
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

#include <plasma/service.h>
#include <plasma/servicejob.h>

//Begin StorageJob
class StorageJob : public Plasma::ServiceJob
{
    Q_OBJECT
public:
    StorageJob(const QString& destination,
               const QString& operation,
               const QMap<QString, QVariant>& parameters,
               QObject *parent = 0);
    void start();
};
//End StorageJob

class Storage : public Plasma::Service
{
    Q_OBJECT

public:
    Storage(const QString& destination, QObject* parent);
    ~Storage();

protected:
    Plasma::ServiceJob* createJob(const QString &operation, QMap<QString, QVariant> &parameters);

private:
    QString m_serviceName;
};


#endif //PLASMA_STORAGE_H
