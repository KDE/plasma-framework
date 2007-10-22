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
#include <KTemporaryFile>
#include <KZip>
#include <KDebug>

#include "packagemetadata.h"
#include "packagestructure.h"

namespace Plasma
{

class Package::Private
{
public:
    Private(const PackageStructure& st, const QString& p)
        : structure(st),
          basePath(p),
          valid(QFile::exists(basePath))
    {
        if (valid && basePath[basePath.length() - 1] != '/') {
            basePath.append('/');
        }
    }

    PackageStructure structure;
    QString basePath;
    bool valid;
};

Package::Package(const QString& packageRoot, const QString& package,
                 const PackageStructure& structure)
    : d(new Private(structure, packageRoot + '/' + package))
{
}

Package::~Package()
{
}

bool Package::isValid() const
{
    if (!d->valid) {
        return false;
    }

    foreach (const char *dir, d->structure.requiredDirectories()) {
        if (QFile::exists(d->basePath + '/' + d->structure.path(dir))) {
            d->valid = false;
            return false;
        }
    }

    foreach (const char *file, d->structure.requiredFiles()) {
        if (QFile::exists(d->basePath + '/' + d->structure.path(file))) {
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

    QString path = d->structure.path(fileType);

    if (path.isEmpty()) {
        return QString();
    }

    if (!filename.isEmpty()) {
        path.prepend(d->basePath);
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

    QString path = d->structure.path(fileType);
    if (path.isEmpty()) {
        return QStringList();
    }

    QDir dir(d->basePath + path);

    if (!dir.exists()) {
        return QStringList();
    }

    return dir.entryList(QDir::Files | QDir::Readable);
}

//TODO: provide a version of this that allows one to ask for certain types of packages, etc?
QStringList Package::knownPackages(const QString& packageRoot) // static
{
    QDir dir(packageRoot);

    if (!dir.exists()) {
        return QStringList();
    }

    QStringList packages;

    foreach (const QString& dir, dir.entryList(QDir::AllDirs | QDir::Readable)) {
        QString metadata = packageRoot + '/' + dir + "/metadata.desktop";
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
            return false;
        }
    }

    if (!QFile::exists(package)) {
        return false;
    }

    KZip archive(package);
    if (!archive.open(QIODevice::ReadOnly)) {
        return false;
    }

    const KArchiveDirectory* source = archive.directory();
    const KArchiveEntry* metadata = source->entry("metadata.desktop");

    if (!metadata) {
        return false;
    }

    QFile f(package);
    QString tempdir = packageRoot + '/' + f.fileName();
    if (QFile::exists(tempdir)) {
        return false;
    }

    source->copyTo(tempdir);

    QString metadataPath = tempdir + "/metadata.desktop";
    if (!QFile::exists(metadataPath)) {
        KIO::SimpleJob* job = KIO::file_delete(tempdir);
        job->exec();
        return false;
    }
    PackageMetadata meta(metadataPath);
    QString targetName = meta.name();

    if (targetName.isEmpty()) {
        KIO::SimpleJob* job = KIO::file_delete(tempdir);
        job->exec();
        return false;
    }

    targetName = packageRoot + '/' + targetName;
    if (QFile::exists(targetName)) {
        KIO::SimpleJob* job = KIO::file_delete(tempdir);
        job->exec();
        return false;
    }

    KIO::FileCopyJob* job = KIO::file_move(tempdir, targetName);

    if (!job->exec()) {
        KIO::SimpleJob* job = KIO::file_delete(tempdir);
        job->exec();
        return false;
    }

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
    job = KIO::file_copy(targetName, service);
    return job->exec();
}

bool Package::createPackage(const PackageMetadata & metadata,
                            const QString& source,
                            const QString& destination)
{
    if (!metadata.isComplete()) {
        kWarning(550) << "Metadata file is not complete" ;
        return false;
    }

    KTemporaryFile metadataFile;
    metadataFile.open();
    metadata.write(metadataFile.fileName());

    KTemporaryFile releaseNotes;
    //We just write the content of the QString containing the metadata in an
    //empty temporary file that we will package with the name metadata.desktop
    if (releaseNotes.open()) {
        QTextStream out(&releaseNotes);
        if (metadata.releaseNotes().isEmpty()) {
            out << metadata.releaseNotes();
        } else {
            out << "NO_RELEASE_NOTES";
        }
    }

    //OK, we've got the temporary file with the metadata in it.
    //Now we just need to put everything into a zip archive.
    KZip creation(destination);
    creation.setCompression(KZip::NoCompression);

    if (!creation.open(QIODevice::WriteOnly)) {
        return false;
    }

    creation.addLocalFile(metadataFile.fileName(), "metadata.desktop");
    creation.addLocalFile(releaseNotes.fileName(), "notes.txt");

    if (!metadata.icon().isEmpty()) {
        //TODO: just one icon?
        creation.addLocalFile(metadata.icon(), "icon.png");
    }

    if (!metadata.preview().isEmpty()) {
        //TODO: just one icon?
        creation.addLocalFile(metadata.preview(), "preview.png");
    }

    creation.addLocalDirectory(source, "contents");
    creation.close();
    return true;
}


} // Namespace
