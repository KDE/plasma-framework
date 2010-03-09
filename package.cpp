/******************************************************************************
*   Copyright 2007 by Aaron Seigo <aseigo@kde.org>                            *
*   Copyright 2007 by Riccardo Iaconelli <riccardo@kde.org>                   *
*                                                                             *
*   This library is free software; you can redistribute it and/or             *
*   modify it under the terms of the GNU Library General Public               *
*   License as published by the Free Software Foundation; either              *
*   version 2 of the License, or (at your option) any later version.          *
*                                                                             *
*   This library is distributed in the hope that it will be useful,           *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of            *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU          *
*   Library General Public License for more details.                          *
*                                                                             *
*   You should have received a copy of the GNU Library General Public License *
*   along with this library; see the file COPYING.LIB.  If not, write to      *
*   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,      *
*   Boston, MA 02110-1301, USA.                                               *
*******************************************************************************/

#include "package.h"
#include "config-plasma.h"

#include <QDir>
#include <QFile>
#include <QRegExp>
#include <QtNetwork/QHostInfo>

#ifdef QCA2_FOUND
#include <QtCrypto>
#endif

#include <karchive.h>
#include <kcomponentdata.h>
#include <kdesktopfile.h>
#include <kio/copyjob.h>
#include <kio/deletejob.h>
#include <kio/jobclasses.h>
#include <kio/job.h>
#include <kmimetype.h>
#include <kplugininfo.h>
#include <kstandarddirs.h>
#include <ktar.h>
#include <ktempdir.h>
#include <ktemporaryfile.h>
#include <kzip.h>
#include <kdebug.h>

#include "authorizationmanager.h"
#include "packagemetadata.h"
#include "private/authorizationmanager_p.h"
#include "private/package_p.h"
#include "private/plasmoidservice_p.h"
#include "private/service_p.h"

namespace Plasma
{

Package::Package(const QString &packageRoot, const QString &package,
                 PackageStructure::Ptr structure)
    : d(new PackagePrivate(structure, packageRoot + '/' + package))
{
}

Package::Package(const QString &packagePath, PackageStructure::Ptr structure)
    : d(new PackagePrivate(structure, packagePath))
{
}

Package::~Package()
{
    delete d;
}

bool Package::isValid() const
{
    if (!d->valid) {
        return false;
    }

    foreach (const char *dir, d->structure->requiredDirectories()) {
        if (!QFile::exists(d->structure->path() + d->structure->contentsPrefix() + d->structure->path(dir))) {
            kWarning() << "Could not find required directory" << dir;
            d->valid = false;
            return false;
        }
    }

    foreach (const char *file, d->structure->requiredFiles()) {
        if (!QFile::exists(d->structure->path() + d->structure->contentsPrefix() + d->structure->path(file))) {
            kWarning() << "Could not find required file" << file << ", look in"
                       << d->structure->path() + d->structure->contentsPrefix() + d->structure->path(file) << endl;
            d->valid = false;
            return false;
        }
    }

    return true;
}

QString Package::filePath(const char *fileType, const QString &filename) const
{
    if (!d->valid) {
        kDebug() << "package is not valid";
        return QString();
    }

    QString path;

    if (qstrlen(fileType) != 0) {
        path = d->structure->path(fileType);

        if (path.isEmpty()) {
            kDebug() << "no matching path came of it, while looking for" << fileType << filename;
            return QString();
        }
    }

    path.prepend(d->structure->path() + d->structure->contentsPrefix());

    if (!filename.isEmpty()) {
        path.append("/").append(filename);
    }

    if (QFile::exists(path)) {
        if (d->structure->allowExternalPaths()) {
            return path;
        }

        // ensure that we don't return files outside of our base path
        // due to symlink or ../ games
        QDir dir(path);
        QString canonicalized = dir.canonicalPath() + QDir::separator();
        if (canonicalized.startsWith(d->structure->path())) {
            return path;
        }
    }

    kDebug() << path << "does not exist";
    return QString();
}

QString Package::filePath(const char *fileType) const
{
    return filePath(fileType, QString());
}

QStringList Package::entryList(const char *fileType) const
{
    if (!d->valid) {
        return QStringList();
    }

    return d->structure->entryList(fileType);
}

PackageMetadata Package::metadata() const
{
    return d->structure->metadata();
}

void Package::setPath(const QString &path)
{
    d->structure->setPath(path);
    d->valid = !d->structure->path().isEmpty();
}

const QString Package::path() const
{
    return d->structure->path();
}

const PackageStructure::Ptr Package::structure() const
{
    return d->structure;
}

#ifdef QCA2_FOUND
void PackagePrivate::updateHash(const QString &basePath, const QString &subPath, const QDir &dir, QCA::Hash &hash)
{
    // hash is calculated as a function of:
    // * files ordered alphabetically by name, with each file's:
    //      * path relative to the content root
    //      * file data
    // * directories ordered alphabetically by name, with each dir's:
    //      * path relative to the content root
    //      * file listing (recursing)
    // symlinks (in both the file and dir case) are handled by adding
    // the name of the symlink itself and the abs path of what it points to

    const QDir::SortFlags sorting = QDir::Name | QDir::IgnoreCase;
    const QDir::Filters filters = QDir::Hidden | QDir::System | QDir::NoDotAndDotDot;
    foreach (const QString &file, dir.entryList(QDir::Files | filters, sorting)) {
        if (!subPath.isEmpty()) {
            hash.update(subPath.toUtf8());
        }

        hash.update(file.toUtf8());

        QFileInfo info(dir.path() + '/' + file);
        if (info.isSymLink()) {
            hash.update(info.symLinkTarget().toUtf8());
        } else {
            QFile f(info.filePath());
            if (f.open(QIODevice::ReadOnly)) {
                while (!f.atEnd()) {
                    hash.update(f.read(1024));
                }
            } else {
                kWarning() << "could not add" << f.fileName() << "to the hash; file could not be opened for reading. "
                           << "permissions fail?" << info.permissions() << info.isFile();
            }
        }
    }

    foreach (const QString &subDirPath, dir.entryList(QDir::Dirs | filters, sorting)) {
        const QString relativePath = subPath + subDirPath + '/';
        hash.update(relativePath.toUtf8());

        QDir subDir(dir.path());
        subDir.cd(subDirPath);

        if (subDir.path() != subDir.canonicalPath()) {
            hash.update(subDir.canonicalPath().toUtf8());
        } else {
            updateHash(basePath, relativePath, subDir, hash);
        }
    }
}
#endif

QString Package::contentsHash() const
{
#ifdef QCA2_FOUND
    if (!QCA::isSupported("sha1")) {
        kWarning() << "can not create hash for" << path() << "due to no SHA1 support in QCA2";
        return QString();
    }

    const QString basePath = d->structure->path() + d->structure->contentsPrefix();
    QDir dir(basePath);

    if (!dir.exists()) {
        return QString();
    }

    QCA::Hash hash("sha1");
    QString metadataPath = d->structure->path() + "metadata.desktop";
    if (QFile::exists(metadataPath)) {
        QFile f(metadataPath);
        if (f.open(QIODevice::ReadOnly)) {
            while (!f.atEnd()) {
                hash.update(f.read(1024));
            }
        } else {
            kWarning() << "could not add" << f.fileName() << "to the hash; file could not be opened for reading.";
        }
    } else {
        kWarning() << "no metadata at" << metadataPath;
    }

    d->updateHash(basePath, QString(), dir, hash);
    return QCA::arrayToHex(hash.final().toByteArray());
#else
    // no QCA2!
    kWarning() << "can not create hash for" << path() << "due to no cryptographic support (QCA2)";
    return QString();
#endif
}

//TODO: provide a version of this that allows one to ask for certain types of packages, etc?
//      should we be using KService here instead/as well?
QStringList Package::listInstalled(const QString &packageRoot) // static
{
    QDir dir(packageRoot);

    if (!dir.exists()) {
        return QStringList();
    }

    QStringList packages;

    foreach (const QString &sdir, dir.entryList(QDir::AllDirs | QDir::Readable)) {
        QString metadata = packageRoot + '/' + sdir + "/metadata.desktop";
        if (QFile::exists(metadata)) {
            PackageMetadata m(metadata);
            packages << m.pluginName();
        }
    }

    return packages;
}

QStringList Package::listInstalledPaths(const QString &packageRoot) // static
{
    QDir dir(packageRoot);

    if (!dir.exists()) {
        return QStringList();
    }

    QStringList packages;

    foreach (const QString &sdir, dir.entryList(QDir::AllDirs | QDir::Readable)) {
        QString metadata = packageRoot + '/' + sdir + "/metadata.desktop";
        if (QFile::exists(metadata)) {
            packages << sdir;
        }
    }

    return packages;
}

bool Package::installPackage(const QString &package,
                             const QString &packageRoot,
                             const QString &servicePrefix) // static
{
    //TODO: report *what* failed if something does fail
    QDir root(packageRoot);

    if (!root.exists()) {
        KStandardDirs::makeDir(packageRoot);
        if (!root.exists()) {
            kWarning() << "Could not create package root directory:" << packageRoot;
            return false;
        }
    }

    QFileInfo fileInfo(package);
    if (!fileInfo.exists()) {
        kWarning() << "No such file:" << package;
        return false;
    }

    QString path;
    KTempDir tempdir;
    bool archivedPackage = false;

    if (fileInfo.isDir()) {
        // we have a directory, so let's just install what is in there
        path = package;

        // make sure we end in a slash!
        if (path[path.size() - 1] != '/') {
            path.append('/');
        }
    } else {
        KArchive *archive = 0;
        KMimeType::Ptr mimetype = KMimeType::findByPath(package);

        if (mimetype->is("application/zip")) {
            archive = new KZip(package);
        } else if (mimetype->is("application/x-compressed-tar") ||
                   mimetype->is("application/x-tar")|| mimetype->is("application/x-bzip-compressed-tar")) {
            archive = new KTar(package);
        } else {
            kWarning() << "Could not open package file, unsupported archive format:" << package << mimetype->name();
            return false;
        }

        if (!archive->open(QIODevice::ReadOnly)) {
            kWarning() << "Could not open package file:" << package;
	    delete archive;
            return false;
        }

        archivedPackage = true;
        path = tempdir.name();

        const KArchiveDirectory *source = archive->directory();
        source->copyTo(path);

        QStringList entries = source->entries();
        if (entries.count() == 1) {
            const KArchiveEntry *entry = source->entry(entries[0]);
            if (entry->isDirectory()) {
                path.append(entry->name()).append("/");
            }
        }
	delete archive;
    }

    QString metadataPath = path + "metadata.desktop";
    if (!QFile::exists(metadataPath)) {
        kWarning() << "No metadata file in package" << package << metadataPath;
        return false;
    }

    PackageMetadata meta(metadataPath);
    QString targetName = meta.pluginName();

    if (targetName.isEmpty()) {
        kWarning() << "Package plugin name not specified";
        return false;
    }

    // Ensure that package names are safe so package uninstall can't inject
    // bad characters into the paths used for removal.
    QRegExp validatePluginName("^[\\w-\\.]+$"); // Only allow letters, numbers, underscore and period.
    if (!validatePluginName.exactMatch(targetName)) {
        kWarning() << "Package plugin name " << targetName << "contains invalid characters";
        return false;
    }

    targetName = packageRoot + '/' + targetName;
    if (QFile::exists(targetName)) {
        kWarning() << targetName << "already exists";
        return false;
    }

    if (archivedPackage) {
        // it's in a temp dir, so just move it over.
        KIO::CopyJob *job = KIO::move(KUrl(path), KUrl(targetName), KIO::HideProgressInfo);
        if (!job->exec()) {
            kWarning() << "Could not move package to destination:" << targetName << " : " << job->errorString();
            return false;
        }
    } else {
        // it's a directory containing the stuff, so copy the contents rather
        // than move them
        KIO::CopyJob *job = KIO::copy(KUrl(path), KUrl(targetName), KIO::HideProgressInfo);
        if (!job->exec()) {
            kWarning() << "Could not copy package to destination:" << targetName << " : " << job->errorString();
            return false;
        }
    }

    if (archivedPackage) {
        // no need to remove the temp dir (which has been successfully moved if it's an archive)
        tempdir.setAutoRemove(false);
    }

    if (!servicePrefix.isEmpty()) {
        // and now we register it as a service =)
        QString metaPath = targetName + "/metadata.desktop";
        KDesktopFile df(metaPath);
        KConfigGroup cg = df.desktopGroup();

        // Q: should not installing it as a service disqualify it?
        // Q: i don't think so since KServiceTypeTrader may not be
        // used by the installing app in any case, and the
        // package is properly installed - aseigo

        //TODO: reduce code duplication with registerPackage below

        QString serviceName = servicePrefix + meta.pluginName();

        QString service = KStandardDirs::locateLocal("services", serviceName + ".desktop");
        KIO::FileCopyJob *job = KIO::file_copy(metaPath, service, -1, KIO::HideProgressInfo);
        if (job->exec()) {
            // the icon in the installed file needs to point to the icon in the
            // installation dir!
            QString iconPath = targetName + '/' + cg.readEntry("Icon");
            QFile icon(iconPath);
            if (icon.exists()) {
                KDesktopFile df(service);
                KConfigGroup cg = df.desktopGroup();
                cg.writeEntry("Icon", iconPath);
            }
        } else {
            kWarning() << "Could not register package as service (this is not necessarily fatal):" << serviceName << " : " << job->errorString();
        }
    }

    return true;
}

bool Package::uninstallPackage(const QString &pluginName,
                               const QString &packageRoot,
                               const QString &servicePrefix) // static
{
    // We need to remove the package directory and its metadata file.
    QString targetName = pluginName;
    targetName = packageRoot + '/' + targetName;

    if (!QFile::exists(targetName)) {
        kWarning() << targetName << "does not exist";
        return false;
    }

    QString serviceName = servicePrefix + pluginName;

    QString service = KStandardDirs::locateLocal("services", serviceName + ".desktop");
    kDebug() << "Removing service file " << service;
    bool ok = QFile::remove(service);

    if (!ok) {
        kWarning() << "Unable to remove " << service;
        return ok;
    }

    KIO::DeleteJob *job = KIO::del(KUrl(targetName));
    if (!job->exec()) {
        kWarning() << "Could not delete package from:" << targetName << " : " << job->errorString();
        return false;
    }

    return true;
}

bool Package::registerPackage(const PackageMetadata &data, const QString &iconPath)
{
    QString serviceName("plasma-applet-" + data.pluginName());
    QString service = KStandardDirs::locateLocal("services", serviceName + ".desktop");

    if (data.pluginName().isEmpty()) {
        return false;
    }

    data.write(service);

    KDesktopFile config(service);
    KConfigGroup cg = config.desktopGroup();
    const QString type = data.type().isEmpty() ? "Service" : data.type();
    cg.writeEntry("Type", type);
    const QString serviceTypes = data.serviceType().isNull() ? "Plasma/Applet,Plasma/Containment" : data.serviceType();
    cg.writeEntry("X-KDE-ServiceTypes", serviceTypes);
    cg.writeEntry("X-KDE-PluginInfo-EnabledByDefault", true);

    QFile icon(iconPath);
    if (icon.exists()) {
        //FIXME: the '/' search will break on non-UNIX. do we care?
        QString installedIcon("plasma_applet_" + data.pluginName() +
                              iconPath.right(iconPath.length() - iconPath.lastIndexOf("/")));
        cg.writeEntry("Icon", installedIcon);
        installedIcon = KStandardDirs::locateLocal("icon", installedIcon);
        KIO::FileCopyJob *job = KIO::file_copy(iconPath, installedIcon, -1, KIO::HideProgressInfo);
        job->exec();
    }

    return true;
}

bool Package::createPackage(const PackageMetadata &metadata,
                            const QString &source,
                            const QString &destination,
                            const QString &icon) // static
{
    Q_UNUSED(icon)
    if (!metadata.isValid()) {
        kWarning() << "Metadata file is not complete";
        return false;
    }

    // write metadata in a temporary file
    KTemporaryFile metadataFile;
    if (!metadataFile.open()) {
        return false;
    }
    metadata.write(metadataFile.fileName());

    // put everything into a zip archive
    KZip creation(destination);
    creation.setCompression(KZip::NoCompression);
    if (!creation.open(QIODevice::WriteOnly)) {
        return false;
    }

    creation.addLocalFile(metadataFile.fileName(), "metadata.desktop");
    creation.addLocalDirectory(source, "contents");
    creation.close();
    return true;
}

PackagePrivate::PackagePrivate(const PackageStructure::Ptr st, const QString &p)
        : structure(st),
          service(0)
{
    structure->setPath(p);
    valid = !structure->path().isEmpty();
}

PackagePrivate::~PackagePrivate()
{
}

void PackagePrivate::publish(AnnouncementMethods methods)
{
    if (!service) {
        service = new PlasmoidService(structure->path());
    }

    QString resourceName =
    i18nc("%1 is the name of a plasmoid, %2 the name of the machine that plasmoid is published on",
          "%1 on %2", structure->metadata().name(), QHostInfo::localHostName());
    kDebug() << "publishing package under name " << resourceName;
    service->d->publish(methods, resourceName, structure->metadata());
}

void PackagePrivate::unpublish()
{
    if (service) {
        service->d->unpublish();
    }
}

bool PackagePrivate::isPublished() const
{
    if (service) {
        return service->d->isPublished();
    } else {
        return false;
    }
}

} // Namespace
