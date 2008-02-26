/******************************************************************************
*   Copyright 2007 by Aaron Seigo <aseigo@kde.org>                        *
*   Copyright 2007 by Riccardo Iaconelli <riccardo@kde.org>               *
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

#include <QDir>
#include <QFile>

#include <KArchiveDirectory>
#include <KArchiveEntry>
#include <KComponentData>
#include <KIO/FileCopyJob>
#include <KIO/Job>
#include <KPluginInfo>
#include <KStandardDirs>
#include <KTempDir>
#include <KTemporaryFile>
#include <KZip>
#include <KDebug>

#include "packagemetadata.h"

namespace Plasma
{

class Package::Private
{
public:
    Private(const PackageStructure::Ptr st, const QString& p)
        : structure(st),
          basePath(p),
          valid(QFile::exists(basePath)),
          metadata(0)
    {
        if (valid && basePath[basePath.length() - 1] != '/') {
            basePath.append('/');
        }
    }

    ~Private()
    {
        delete metadata;
    }

    const PackageStructure::Ptr structure;
    QString basePath;
    bool valid;
    PackageMetadata *metadata;
};

Package::Package(const QString& packageRoot, const QString& package,
                 const PackageStructure::Ptr structure)
    : d(new Private(structure, packageRoot + '/' + package))
{
}

Package::Package(const QString &packagePath, const PackageStructure::Ptr structure)
    : d(new Private(structure, packagePath))
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
        if (!QFile::exists(d->basePath + "contents/" + d->structure->path(dir))) {
            kWarning(505) << "Could not find required directory" << dir;
            d->valid = false;
            return false;
        }
    }

    foreach (const char *file, d->structure->requiredFiles()) {
        if (!QFile::exists(d->basePath + "contents/" + d->structure->path(file))) {
            kWarning(505) << "Could not find required file" << file << ", look in"
                          << d->basePath + "contents/" + d->structure->path(file) << endl;
            d->valid = false;
            return false;
        }
    }

    return true;
}

QString Package::filePath(const char* fileType, const QString& filename) const
{
    if (!d->valid) {
        return QString();
    }

    QString path = d->structure->path(fileType);

    if (path.isEmpty()) {
        return QString();
    }

    path.prepend(d->basePath + "contents/");

    if (!filename.isEmpty()) {
        path.append("/").append(filename);
    }

    if (QFile::exists(path)) {
        return path;
    }

    return QString();
}

QString Package::filePath(const char* fileType) const
{
    return filePath(fileType, QString());
}

QStringList Package::entryList(const char* fileType) const
{
    if (!d->valid) {
        return QStringList();
    }

    QString path = d->structure->path(fileType);
    if (path.isEmpty()) {
        return QStringList();
    }

    QDir dir(d->basePath + "contents/" + path);

    if (!dir.exists()) {
        return QStringList();
    }

    return dir.entryList(QDir::Files | QDir::Readable);
}

const PackageMetadata* Package::metadata() const
{
    if (!d->metadata) {
        d->metadata = new PackageMetadata(d->basePath + "metadata.desktop");
    }
    return d->metadata;
}

//TODO: provide a version of this that allows one to ask for certain types of packages, etc?
//      should we be using KService here instead/as well?
QStringList Package::knownPackages(const QString& packageRoot) // static
{
    QDir dir(packageRoot);

    if (!dir.exists()) {
        return QStringList();
    }

    QStringList packages;

    foreach (const QString& sdir, dir.entryList(QDir::AllDirs | QDir::Readable)) {
        QString metadata = packageRoot + '/' + sdir + "/metadata.desktop";
        if (QFile::exists(metadata)) {
            PackageMetadata m(metadata);
            packages << m.name();
        }
    }

    return packages;
}

bool Package::installPackage(const QString& package,
                             const QString& packageRoot) // static
{
    //TODO: report *what* failed if something does fail
    QDir root(packageRoot);

    if (!root.exists()) {
        KStandardDirs::makeDir(packageRoot);
        if (!root.exists()) {
            kWarning(505) << "Could not create package root directory:" << packageRoot;
            return false;
        }
    }

    if (!QFile::exists(package)) {
        kWarning(505) << "No such file:" << package;
        return false;
    }

    KZip archive(package);
    if (!archive.open(QIODevice::ReadOnly)) {
        kWarning(505) << "Could not open package file:" << package;
        return false;
    }

    const KArchiveDirectory* source = archive.directory();
    const KArchiveEntry* metadata = source->entry("metadata.desktop");

    if (!metadata) {
        kWarning(505) << "No metadata file in package" << package;
        return false;
    }

    QFile f(package);
    KTempDir tempdir;
    source->copyTo(tempdir.name());

    QString metadataPath = tempdir.name() + "metadata.desktop";
    if (!QFile::exists(metadataPath)) {
        kWarning(505) << "No metadata file in package" << package;
        return false;
    }
    PackageMetadata meta(metadataPath);
    QString targetName = meta.name();

    if (targetName.isEmpty()) {
        kWarning(505) << "Package name not specified";
        return false;
    }

    targetName = packageRoot + '/' + targetName;
    if (QFile::exists(targetName)) {
        kWarning(505) << targetName << "already exists";
        return false;
    }

    KIO::FileCopyJob* job = KIO::file_move(tempdir.name(), targetName, -1,
        KIO::HideProgressInfo);

    if (!job->exec()) {
        kWarning(505) << "Could not move package to destination:" << targetName;
        return false;
    }
    
    // no need to remove the temp dir (which has been moved)
    tempdir.setAutoRemove(false);

    // and now we register it as a service =)
    targetName.append("/metadata.desktop");
    QString service = KStandardDirs::locateLocal("services",
                                                 KGlobal::mainComponent().componentName());
    KPluginInfo pluginInfo(targetName);

    if (pluginInfo.pluginName().isEmpty()) {
        // should not installing it as a service disqualify it?
        // i don't think so since KServiceTypeTrader may not be
        // used by the installing app in any case, and the
        // package is properly installed - aseigo
        return true;
    }

    service.append(pluginInfo.pluginName()).append(".desktop");
    job = KIO::file_copy(targetName, service, -1, KIO::HideProgressInfo);
    return job->exec();
}

bool Package::createPackage(const PackageMetadata &metadata,
                            const QString &source,
                            const QString &destination,
                            const QString &icon) // static
{
    if (!metadata.isComplete()) {
        kWarning(550) << "Metadata file is not complete";
        return false;
    }

    // write metadata in a temporary file
    KTemporaryFile metadataFile;
    if (!metadataFile.open()) {
        return false;
    }
    metadata.write(metadataFile.fileName(), icon);

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


} // Namespace
