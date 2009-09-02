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

#ifndef PLASMOIDSERVICE_H
#define PLASMOIDSERVICE_H

#include "dataengineconsumer_p.h"

#include "../packagemetadata.h"
#include "../package.h"
#include "../service.h"
#include "../servicejob.h"

#include <ktemporaryfile.h>

namespace Plasma
{

class Applet;
class DataEngine;
class GetSource;
class PlasmoidService;

class PlasmoidServiceJob : public ServiceJob
{
   Q_OBJECT

    public:
        PlasmoidServiceJob(const QString &plasmoidLocation,
                           const QString &destination,
                           const QString &operation,
                           QMap<QString,QVariant>& parameters,
                           PlasmoidService *parent = 0);

        void start();

    private:
        PlasmoidService *m_service;
        QString m_packagePath;
        QString m_pluginName;
};

class PlasmoidService : public Service, DataEngineConsumer
{
    Q_OBJECT

    public:
        PlasmoidService(const QString &plasmoidLocation);
        PlasmoidService(Applet *applet);
        PackageMetadata metadata() const;


    protected:
        Plasma::ServiceJob* createJob(const QString& operation,
                                      QMap<QString,QVariant>& parameters);

    private:
        QString m_packagePath;
        PackageMetadata m_metadata;
        KTemporaryFile m_tempFile;

        friend class PlasmoidServiceJob;
};

}

#endif 
