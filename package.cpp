/******************************************************************************
*   Copyright (C) 2007 by Aaron Seigo <aseigo@kde.org>                        *
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

#include <QDir>
#include <QFile>

#include <KArchiveDirectory>
#include <KArchiveEntry>
#include <KIO/FileCopyJob>
#include <KIO/Job>
#include <KStandardDirs>
#include <KZip>

#include "package.h"
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

bool Package::isValid()
{
    if (!d->valid) {
        return false;
    }

    foreach (const QString& dir, d->structure.requiredDirectories()) {
        if (QFile::exists(d->basePath + "/" + dir)) {
            d->valid = false;
            return false;
        }
    }

    foreach (const QString& file, d->structure.requiredFiles()) {
        if (QFile::exists(d->basePath + "/" + file)) {
            d->valid = false;
            return false;
        }
    }

    return true;
}

QString Package::filePath(const char* fileType, const QString& filename)
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

QString Package::filePath(const char* fileType)
{
    return filePath(fileType, QString());
}

QStringList Package::entryList(const char* fileType)
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

bool Package::installPackage(const QString& package, const QString& packageRoot) // static
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

    bool success = job->exec();

    if (!success) {
        KIO::SimpleJob* job = KIO::file_delete(tempdir);
        job->exec();
        return false;
    }

    return success;
}

} // Namespace
