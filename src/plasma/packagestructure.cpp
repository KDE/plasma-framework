/*
    SPDX-FileCopyrightText: 2011 Aaron Seigo <aseigo@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "packagestructure.h"
#include "debug_p.h"
#include "private/package_p.h"
#include "private/packagestructure_p.h"
#include <QDebug>

#include <kpackage/packageloader.h>
#include <kpackage/packagestructure.h>

#include <QVariantMap>

#include <KDesktopFile>
#include <KJob>
#include <QDBusConnection>
#include <QDBusPendingCall>
#include <QDir>
#include <QFile>

namespace Plasma
{
QHash<KPackage::Package *, Plasma::Package *> PackageStructureWrapper::s_packagesMap;

PackageStructureWrapper::PackageStructureWrapper(Plasma::PackageStructure *structure, QObject *parent, const QVariantList &args)
    : KPackage::PackageStructure(parent, args)
    , m_struct(structure)
{
}

PackageStructureWrapper::~PackageStructureWrapper()
{
}

void PackageStructureWrapper::initPackage(KPackage::Package *package)
{
    if (!m_struct || !s_packagesMap.contains(package)) {
        return;
    }

    m_struct->initPackage(s_packagesMap.value(package));
}

void PackageStructureWrapper::pathChanged(KPackage::Package *package)
{
    if (!m_struct || !s_packagesMap.contains(package)) {
        return;
    }

    m_struct->pathChanged(s_packagesMap.value(package));
}

KJob *PackageStructureWrapper::install(KPackage::Package *package, const QString &archivePath, const QString &packageRoot)
{
    if (!m_struct || !s_packagesMap.contains(package)) {
        return nullptr;
    }

    return m_struct->install(s_packagesMap.value(package), archivePath, packageRoot);
}

KJob *PackageStructureWrapper::uninstall(KPackage::Package *package, const QString &packageRoot)
{
    if (!m_struct || !s_packagesMap.contains(package)) {
        return nullptr;
    }

    return m_struct->uninstall(s_packagesMap.value(package), packageRoot);
}

void PackageStructurePrivate::installPathChanged(const QString &path)
{
    KJob *job = qobject_cast<KJob *>(q->sender());
    if (!job || job->error()) {
        return;
    }

    const QString servicePrefix = job->property("servicePrefix").toString();
    const QString serviceName = job->property("serviceName").toString();

    // uninstall
    if (path.isEmpty()) {
        if (serviceName.isEmpty()) {
            return;
        }

        QString service = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QLatin1String("/kservices5/") + serviceName;

        bool ok = QFile::remove(service);
        if (!ok) {
            qCWarning(LOG_PLASMA) << "Unable to remove " << service;
        }

        // install
    } else {
        if (!servicePrefix.isEmpty()) {
            // and now we register it as a service =)
            QString metaPath = path + QStringLiteral("/metadata.desktop");
            KDesktopFile df(metaPath);
            KConfigGroup cg = df.desktopGroup();
            const QString pluginName = cg.readEntry("X-KDE-PluginInfo-Name", QString());

            if (pluginName.isEmpty()) {
                return;
            }

            // Q: should not installing it as a service disqualify it?
            // Q: i don't think so since KServiceTypeTrader may not be
            // used by the installing app in any case, and the
            // package is properly installed - aseigo

            // TODO: remove installation of the desktop file in kservices5 when possible

            const QString serviceName = servicePrefix + pluginName + QStringLiteral(".desktop");

            QString localServiceDirectory = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QLatin1String("/kservices5/");
            if (!QDir().mkpath(localServiceDirectory)) {
                qCDebug(LOG_PLASMA) << "Failed to create ... " << localServiceDirectory;
                qCWarning(LOG_PLASMA) << "Could not create local service directory:" << localServiceDirectory;
                return;
            }
            QString service = localServiceDirectory + serviceName;

            qCDebug(LOG_PLASMA) << "-- Copying " << metaPath << service;
            const bool ok = QFile::copy(metaPath, service);
            if (ok) {
                qCDebug(LOG_PLASMA) << "Copying metadata went ok.";
                // the icon in the installed file needs to point to the icon in the
                // installation dir!
                QString iconPath = path + QLatin1Char('/') + cg.readEntry("Icon");
                QFile icon(iconPath);
                if (icon.exists()) {
                    KDesktopFile df(service);
                    KConfigGroup cg = df.desktopGroup();
                    cg.writeEntry("Icon", iconPath);
                }
            } else {
                qCWarning(LOG_PLASMA) << "Could not register package as service (this is not necessarily fatal):" << serviceName;
            }
        }
    }
    const auto call = QDBusMessage::createMethodCall(QStringLiteral("org.kde.kded5"),
                                                     QStringLiteral("/kbuildsycoca"),
                                                     QStringLiteral("org.kde.kbuildsycoca"),
                                                     QStringLiteral("recreate"));
    QDBusConnection::sessionBus().asyncCall(call);
}

PackageStructure::PackageStructure(QObject *parent, const QVariantList &args)
    : QObject(parent)
    , d(new PackageStructurePrivate(this))
{
    if (!args.isEmpty() && args.first().canConvert<QString>()) {
        d->internalStructure = KPackage::PackageLoader::self()->loadPackageStructure(args.first().toString());
    }

    Q_UNUSED(args)
}

PackageStructure::~PackageStructure()
{
    delete d;
}

void PackageStructure::initPackage(Package *package)
{
    if (d->internalStructure && !qobject_cast<PackageStructureWrapper *>(d->internalStructure)) {
        d->internalStructure->initPackage(package->d->internalPackage);
    }
}

void PackageStructure::pathChanged(Package *package)
{
    if (d->internalStructure && !qobject_cast<PackageStructureWrapper *>(d->internalStructure)) {
        d->internalStructure->pathChanged(package->d->internalPackage);
    }
}

KJob *PackageStructure::install(Package *package, const QString &archivePath, const QString &packageRoot)
{
    if (d->internalStructure && !qobject_cast<PackageStructureWrapper *>(d->internalStructure)) {
        KJob *job = d->internalStructure->install(package->d->internalPackage, archivePath, packageRoot);
        if (job) {
            job->setProperty("servicePrefix", package->servicePrefix());
            connect(job, SIGNAL(installPathChanged(QString)), this, SLOT(installPathChanged(QString)));
        }
        return job;
    } else if (d->internalStructure) {
        KJob *job = d->internalStructure->KPackage::PackageStructure::install(package->d->internalPackage, archivePath, packageRoot);
        connect(job, SIGNAL(installPathChanged(QString)), this, SLOT(installPathChanged(QString)));
        return job;
    }

    return nullptr;
}

KJob *PackageStructure::uninstall(Package *package, const QString &packageRoot)
{
    if (d->internalStructure && !qobject_cast<PackageStructureWrapper *>(d->internalStructure)) {
        QString metaPath = package->path() + QStringLiteral("/metadata.desktop");
        KDesktopFile df(metaPath);
        KConfigGroup cg = df.desktopGroup();
        const QString pluginName = cg.readEntry("X-KDE-PluginInfo-Name", QString());
        const QString serviceName = package->servicePrefix() + pluginName + QStringLiteral(".desktop");

        KJob *job = d->internalStructure->uninstall(package->d->internalPackage, packageRoot);
        if (job) {
            job->setProperty("serviceName", serviceName);

            connect(job, SIGNAL(installPathChanged(QString)), this, SLOT(installPathChanged(QString)));
        }
        return job;
    } else if (d->internalStructure) {
        KJob *job = d->internalStructure->KPackage::PackageStructure::uninstall(package->d->internalPackage, packageRoot);
        connect(job, SIGNAL(installPathChanged(QString)), this, SLOT(installPathChanged(QString)));
        return job;
    }

    return nullptr;
}

}

#include "moc_packagestructure.cpp"
#include "private/moc_packagestructure_p.cpp"
