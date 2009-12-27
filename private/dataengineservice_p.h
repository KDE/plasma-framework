/*
 *   Copyright © 2009 Rob Scheepmaker <r.scheepmaker@student.utwente.nl>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License version 2 as
 *   published by the Free Software Foundation
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

#ifndef PLASMA_DATAENGINESERVICE_P_H
#define PLASMA_DATAENGINESERVICE_P_H

#include <QStringList>

#include "../dataengine.h"
#include "../service.h"

namespace Plasma
{

class DataEngine;
class GetSource;

class DataEngineService : public Plasma::Service
{
    Q_OBJECT

    public:
        DataEngineService(DataEngine *engine);

    public Q_SLOTS:
        void dataUpdated(QString source, Plasma::DataEngine::Data data);

    protected:
        Plasma::ServiceJob* createJob(const QString& operation,
                                      QMap<QString,QVariant>& parameters);

    private Q_SLOTS:
        void sourceAdded(QString source);

    private:
        DataEngine *m_engine;
        QMap<QString, QStringList> m_peersAlreadyUpdated;
        QMap<QString, DataEngine::Data> m_data;

        friend class GetSource;
};

}

#endif // PLASMA_DATAENGINESERVICE_P_H
