/******************************************************************************
*   Copyright 2007 by Aaron Seigo <aseigo@kde.org>                            *
*   Copyright 2010 by Marco Martin <notmart@gmail.com>                        *
*   Copyright 2010 by Kevin Ottens <ervin@kde.org>                            *
*   Copyright 2009 by Rob Scheepmaker                                         *
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

#include <karchive.h>
#include <kcomponentdata.h>
#include <kdesktopfile.h>
#ifndef PLASMA_NO_KIO
#include <kio/copyjob.h>
#include <kio/deletejob.h>
#include <kio/jobclasses.h>
#include <kio/job.h>
#endif
#include <kmimetype.h>
#include <kplugininfo.h>
#include <kstandarddirs.h>
#include <ktar.h>
#include <ktempdir.h>
#include <ktemporaryfile.h>
#include <kzip.h>
#include <kdebug.h>

#include "private/package_p.h"
#include "private/plasmoidservice_p.h"
#include "private/service_p.h"
#include "remote/authorizationmanager.h"
#include "remote/authorizationmanager_p.h"

namespace Plasma
{

#ifdef PLASMA_NO_KIO // Provide some convenience for dealing with folders

bool copyFolder(QString sourcePath, QString targetPath)
{
    QDir source(sourcePath);
    if(!source.exists())
        return false;

    QDir target(targetPath);
    if(!target.exists()) {
        QString targetName = target.dirName();
        target.cdUp();
        target.mkdir(targetName);
        target = QDir(targetPath);
    }

    foreach (const QString &fileName, source.entryList(QDir::Files)) {
        QString sourceFilePath = sourcePath + QDir::separator() + fileName;
        QString targetFilePath = targetPath + QDir::separator() + fileName;

        if (!QFile::copy(sourceFilePath, targetFilePath)) {
            return false;
        }
    }

    foreach (const QString &subFolderName, source.entryList(QDir::AllDirs | QDir::NoDotAndDotDot)) {
        QString sourceSubFolderPath = sourcePath + QDir::separator() + subFolderName;
        QString targetSubFolderPath = targetPath + QDir::separator() + subFolderName;

        if (!copyFolder(sourceSubFolderPath, targetSubFolderPath)) {
            return false;
        }
    }

    return true;
}

bool removeFolder(QString folderPath)
{
    QDir folder(folderPath);
    if(!folder.exists())
        return false;

    foreach (const QString &fileName, folder.entryList(QDir::Files)) {
        if (!QFile::remove(folderPath + QDir::separator() + fileName)) {
            return false;
        }
    }

    foreach (const QString &subFolderName, folder.entryList(QDir::AllDirs | QDir::NoDotAndDotDot)) {
        if (!removeFolder(folderPath + QDir::separator() + subFolderName)) {
            return false;
        }
    }

    QString folderName = folder.dirName();
    folder.cdUp();
    return folder.rmdir(folderName);
}

#endif // PLASMA_NO_KIO


Package::Package()
    : d(new PackagePrivate(PackageStructure::Ptr(0), QString()))
{
}

Package::Package(const QString &packagePath, PackageStructure::Ptr structure)
    : d(new PackagePrivate(structure, packagePath))
{
}

Package::Package(const Package &other)
    : d(new PackagePrivate(*other.d))
{
}

Package::~Package()
{
    delete d;
}

Package &Package::operator=(const Package &rhs)
{
    if (&rhs != this) {
        *d = *rhs.d;
    }

    return *this;
}

bool Package::isValid() const
{
    if (!d->valid) {
        return false;
    }

    //search for the file in all prefixes and in all possible paths for each prefix
    //even if it's a big nested loop, usually there is one prefix and one location
    //so shouldn't cause too much disk access
    QStringList prefixes = d->structure->contentsPrefixPaths();
    if (prefixes.isEmpty()) {
        prefixes << QString();
    }

    foreach (const char *dir, d->structure->requiredDirectories()) {
        bool failed = true;
        foreach (const QString &path, d->structure->searchPath(dir)) {
            foreach (const QString &prefix, prefixes) {
                if (QFile::exists(d->structure->path() + prefix + path)) {
                    failed = false;
                    break;
                }
            }
            if (!failed) {
                break;
            }
        }

        if (failed) {
            kWarning() << "Could not find required directory" << dir;
            d->valid = false;
            return false;
        }
    }

    foreach (const char *file, d->structure->requiredFiles()) {
        bool failed = true;
        foreach (const QString &path, d->structure->searchPath(file)) {
            foreach (const QString &prefix, prefixes) {
                if (QFile::exists(d->structure->path() + prefix + path)) {
                    failed = false;
                    break;
                }
            }
            if (!failed) {
                break;
            }
        }

        if (failed) {
            kWarning() << "Could not find required file" << file;
            d->valid = false;
            return false;
        }
    }

    return true;
}

QString Package::filePath(const char *fileType, const QString &filename) const
{
    if (!d->valid) {
        //kDebug() << "package is not valid";
        return QString();
    }

    QStringList paths;

    if (qstrlen(fileType) != 0) {
        paths = d->structure->searchPath(fileType);

        if (paths.isEmpty()) {
            //kDebug() << "no matching path came of it, while looking for" << fileType << filename;
            return QString();
        }
    } else {
        //when filetype is empty paths is always empty, so try with an empty string
        paths << QString();
    }

    //Nested loop, but in the medium case resolves to just one iteration
    QStringList prefixes = d->structure->contentsPrefixPaths();
    if (prefixes.isEmpty()) {
        prefixes << QString();
    }

    //kDebug() << "prefixes:" << prefixes.count() << prefixes;
    foreach (const QString &contentsPrefix, prefixes) {
        const QString prefix(d->structure->path() + contentsPrefix);

        foreach (const QString &path, paths) {
            QString file = prefix + path;

            if (!filename.isEmpty()) {
                file.append("/").append(filename);
            }

            //kDebug() << "testing" << file << QFile::exists("/bin/ls") << QFile::exists(file);
            if (QFile::exists(file)) {
                if (d->structure->allowExternalPaths()) {
                    //kDebug() << "found" << file;
                    return file;
                }

                // ensure that we don't return files outside of our base path
                // due to symlink or ../ games
                QDir dir(file);
                QString canonicalized = dir.canonicalPath() + QDir::separator();

                //kDebug() << "testing that" << canonicalized << "is in" << d->structure->path();
                if (canonicalized.startsWith(d->structure->path())) {
                    //kDebug() << "found" << file;
                    return file;
                }
            }
        }
    }

    //kDebug() << fileType << filename << "does not exist in" << prefixes << "at root" << d->structure->path();
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

void Package::setPath(const QString &path)
{
    if (d->structure) {
        d->structure->setPath(path);
        d->valid = !d->structure->path().isEmpty();
    }
}

const QString Package::path() const
{
    return d->structure ? d->structure->path() : QString();
}

const PackageStructure::Ptr Package::structure() const
{
    return d->structure;
}

void PackagePrivate::updateHash(const QString &basePath, const QString &subPath, const QDir &dir, QCryptographicHash &hash)
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
            hash.addData(subPath.toUtf8());
        }

        hash.addData(file.toUtf8());

        QFileInfo info(dir.path() + '/' + file);
        if (info.isSymLink()) {
            hash.addData(info.symLinkTarget().toUtf8());
        } else {
            QFile f(info.filePath());
            if (f.open(QIODevice::ReadOnly)) {
                while (!f.atEnd()) {
                    hash.addData(f.read(1024));
                }
            } else {
                kWarning() << "could not add" << f.fileName() << "to the hash; file could not be opened for reading. "
                           << "permissions fail?" << info.permissions() << info.isFile();
            }
        }
    }

    foreach (const QString &subDirPath, dir.entryList(QDir::Dirs | filters, sorting)) {
        const QString relativePath = subPath + subDirPath + '/';
        hash.addData(relativePath.toUtf8());

        QDir subDir(dir.path());
        subDir.cd(subDirPath);

        if (subDir.path() != subDir.canonicalPath()) {
            hash.addData(subDir.canonicalPath().toUtf8());
        } else {
            updateHash(basePath, relativePath, subDir, hash);
        }
    }
}

QString Package::contentsHash() const
{
    if (!d->valid) {
        kWarning() << "can not create hash due to Package being invalid";
        return QString();
    }

    QCryptographicHash hash(QCryptographicHash::Sha1);
    QString metadataPath = d->structure->path() + "metadata.desktop";
    if (QFile::exists(metadataPath)) {
        QFile f(metadataPath);
        if (f.open(QIODevice::ReadOnly)) {
            while (!f.atEnd()) {
                hash.addData(f.read(1024));
            }
        } else {
            kWarning() << "could not add" << f.fileName() << "to the hash; file could not be opened for reading.";
        }
    } else {
        kWarning() << "no metadata at" << metadataPath;
    }

    QStringList prefixes = d->structure->contentsPrefixPaths();
    if (prefixes.isEmpty()) {
        prefixes << QString();
    }

    foreach (QString prefix, prefixes) {
        const QString basePath = d->structure->path() + prefix;
        QDir dir(basePath);

        if (!dir.exists()) {
            return QString();
        }

        d->updateHash(basePath, QString(), dir, hash);
    }

    return hash.result();
}

PackagePrivate::PackagePrivate(const PackageStructure::Ptr st, const QString &p)
        : structure(st),
          service(0)
{
    if (structure) {
        structure->setPath(p);
    }

    valid = structure && !structure->path().isEmpty();
}

PackagePrivate::PackagePrivate(const PackagePrivate &other)
        : structure(other.structure),
          service(other.service),
          valid(other.valid)
{
}

PackagePrivate::~PackagePrivate()
{
}

PackagePrivate &PackagePrivate::operator=(const PackagePrivate &rhs)
{
    structure = rhs.structure;
    service = rhs.service;
    valid = rhs.valid;
    return *this;
}

} // Namespace
