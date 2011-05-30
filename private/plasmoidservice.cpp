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

#include "plasmoidservice_p.h"

#include "authorizationmanager_p.h"
#include "dataengineconsumer_p.h"
#include "dataengine_p.h"

#include <plasma/applet.h>
#include <plasma/remote/authorizationmanager.h>
#include <plasma/remote/authorizationrule.h>
#include <plasma/packagemetadata.h>
#include <plasma/service.h>
#include <plasma/servicejob.h>

#include <kdebug.h>
#include <ktemporaryfile.h>
#include <kzip.h>

#include <QFile>
#include <QFileInfo>
#include <QHostInfo>

namespace Plasma
{

PlasmoidServiceJob::PlasmoidServiceJob(const QString &destination,
                                       const QString &operation,
                                       QHash<QString,QVariant>& parameters,
                                       PlasmoidService *service)
    : Plasma::ServiceJob(destination, operation, parameters,
                         static_cast<Plasma::Service*>(service)),
      m_service(service)
{
}

void PlasmoidServiceJob::start()
{
    if (operationName() == "GetPackage") {
        kDebug() << "sending " << m_service->m_packagePath;
        QFileInfo fileInfo(m_service->m_packagePath);

        if (fileInfo.exists() && fileInfo.isAbsolute()) {
            kDebug() << "file exists, let's try and read it";
            QFile file(m_service->m_packagePath);
            file.open(QIODevice::ReadOnly);
            setResult(file.readAll());
        } else {
            kDebug() << "file doesn't exists, we're sending the plugin name";
            setResult(m_service->m_packagePath);
        }
    } else if (operationName() == "GetMetaData") {
        QFile file(m_service->m_metadata);
        setResult(file.readAll());
    } else if (operationName() == "DataEngine") {
        DataEngine *engine  = m_service->dataEngine(parameters()["EngineName"].toString());
        QString serviceName = "plasma-dataengine-" + parameters()["EngineName"].toString();
        engine->d->publish(NoAnnouncement, serviceName);
        if (!AuthorizationManager::self()->d->matchingRule(serviceName, identity())) {
            AuthorizationRule *rule = new AuthorizationRule(serviceName, identity().id());
            rule->setPolicy(AuthorizationRule::Allow);
            AuthorizationManager::self()->d->rules.append(rule);
        }
        setResult(serviceName);
    }
}


PlasmoidService::PlasmoidService(const QString &packageLocation)
    : Plasma::Service(0)
{
    setName("plasmoidservice");

    QString location;
    location = packageLocation;
    if (!location.endsWith('/')) {
        location.append('/');
    }

    m_metadata = location + "metadata.desktop";

    /*FIXME: either do something useful on error or don't waste time with them
    if (!QFile::exists(m_metadata)) {
        kDebug() << "not a valid package";
    }
    */

    m_tempFile.open();
    m_packagePath = m_tempFile.fileName();
    m_tempFile.close();

    // put everything into a zip archive
    KZip creation(m_packagePath);
    creation.setCompression(KZip::NoCompression);
    if (!creation.open(QIODevice::WriteOnly)) {
        /*FIXME: either do something useful on error or don't waste time with it */
        kDebug() << "could not open archive";
    }

    creation.addLocalFile(location + "metadata.desktop", "metadata.desktop");
    location.append("contents/");
    creation.addLocalDirectory(location, "contents");
    creation.close();
}

PlasmoidService::PlasmoidService(Applet *applet)
{
    setName("plasmoidservice");
    if (!applet->package() || !applet->package()->isValid()) {
        kDebug() << "not a valid package";
        m_packagePath = applet->pluginName();
    }
}

Plasma::ServiceJob* PlasmoidService::createJob(const QString& operation, QHash<QString,QVariant>& parameters)
{
    return new PlasmoidServiceJob(destination(), operation, parameters, this);
}

}

#include "plasmoidservice_p.moc"

