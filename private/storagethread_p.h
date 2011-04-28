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

#include "servicejob_p.h"

namespace Plasma
{

class StorageThread : public QThread
{
    Q_OBJECT
public:
    StorageThread(QObject *parent = 0);
    ~StorageThread();

    void run();

    static StorageThread *self();

public Q_SLOTS:
    void save(ServiceJob* caller, const QVariantMap &parameters);
    void retrieve(ServiceJob* caller, const QVariantMap &parameters);
    void deleteEntry(ServiceJob* caller, const QVariantMap &parameters);
    void expire(ServiceJob* caller, const QVariantMap &parameters);

Q_SIGNALS:
    void newResult(ServiceJob* caller, bool result);

private:
    QSqlDatabase m_db;
};

}

#endif
