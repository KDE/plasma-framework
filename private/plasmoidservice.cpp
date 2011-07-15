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
#include <plasma/service.h>
#include <plasma/servicejob.h>

#include <kdebug.h>
#include <ktemporaryfile.h>
#include <kzip.h>
#include <kservicetypetrader.h>

#include <QDir>
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
        if (m_service->m_packagePath.isEmpty()) {
            // just return the plugin name in this case
            setResult(m_service->m_pluginName);
        } else {
            QFileInfo fileInfo(m_service->m_packagePath);

            if (fileInfo.exists() && fileInfo.isAbsolute()) {
                kDebug() << "file exists, let's try and read it";
                QFile file(m_service->m_packagePath);
                file.open(QIODevice::ReadOnly);
                setResult(file.readAll());
            } else {
                setResult(QString());
            }
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

    setResult(false);
}

PlasmoidService::PlasmoidService(Applet *applet)
    : Plasma::Service(applet),
      m_pluginName(applet->pluginName())
{
    setName("plasmoidservice");

    if (applet->package().isValid()) {
        const QString root = applet->package().path();
        m_metadata = root + "metadata.desktop";

        m_tempFile.open();
        m_packagePath = m_tempFile.fileName();
        m_tempFile.close();

        // put everything into a zip archive
        KZip creation(m_packagePath);
        creation.setCompression(KZip::NoCompression);
        if (creation.open(QIODevice::WriteOnly)) {
            QDir dir(root);
            foreach (const QString &entry, dir.entryList(QDir::Files)) {
                creation.addLocalFile(root + entry, entry);
            }

            foreach (const QString &entry, dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot)) {
                creation.addLocalDirectory(root + entry, entry);
            }

            creation.close();
        } else {
            kDebug() << "could not open archive";
        }
    } else {
        kDebug() << "applet lacks a valid package";
        const QString constraint = QString("[X-KDE-PluginInfo-Name] == '%1'").arg(applet->pluginName());
        KService::List offers = KServiceTypeTrader::self()->query("Plasma/Applet", constraint);
        if (!offers.isEmpty()) {
            m_metadata = offers.first()->entryPath();
        }
    }
}

Plasma::ServiceJob* PlasmoidService::createJob(const QString& operation, QHash<QString,QVariant>& parameters)
{
    return new PlasmoidServiceJob(destination(), operation, parameters, this);
}

}

#include "plasmoidservice_p.moc"

