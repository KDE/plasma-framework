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

#include <qtemporarydir.h>

#include <karchive.h>
#include <QDebug>
#include <kdesktopfile.h>
#include <kservicetypetrader.h>
#include <ktar.h>
#include <kzip.h>

#include "config-plasma.h"

#include <qstandardpaths.h>
#include <qmimedatabase.h>

#include "packagestructure.h"
#include "pluginloader.h"
#include "private/package_p.h"
#include "private/packages_p.h"
#include "private/packagejob_p.h"

namespace Plasma
{

Package::Package(PackageStructure *structure)
    : d(new PackagePrivate())
{
    d->structure = structure;

    if (d->structure) {
        d->structure.data()->initPackage(this);
    }
}

Package::Package(const Package &other)
    : d(other.d)
{
}

Package::~Package()
{
}

Package &Package::operator=(const Package &rhs)
{
    if (&rhs != this) {
        d = rhs.d;
    }

    return *this;
}

bool Package::hasValidStructure() const
{
    return d->structure;
}

bool Package::isValid() const
{
    if (!d->structure) {
        return false;
    }

    //Minimal packages with no metadata *are* supposed to be possible
    //so if !metadata().isValid() go ahead
    if (metadata().isValid() && metadata().isHidden()) {
        return false;
    }

    if (d->checkedValid) {
        return d->valid;
    }

    d->valid = true;

    //search for the file in all prefixes and in all possible paths for each prefix
    //even if it's a big nested loop, usually there is one prefix and one location
    //so shouldn't cause too much disk access
    QHashIterator<QByteArray, ContentStructure> it(d->contents);
    const QString rootPath = d->tempRoot.isEmpty() ? d->path : d->tempRoot;

    while (it.hasNext()) {
        it.next();
        if (!it.value().required) {
            continue;
        }

        bool failed = true;
        foreach (const QString &path, it.value().paths) {
            foreach (const QString &prefix, d->contentsPrefixPaths) {
                if (QFile::exists(rootPath + prefix + path)) {
                    failed = false;
                    break;
                }
            }

            if (!failed) {
                break;
            }
        }

        if (failed) {
            //qWarning() << "Could not find required" << (it.value().directory ? "directory" : "file") << it.key() << "for package" << path();
            d->valid = false;
            break;
        }
    }

    return d->valid;
}

QString Package::name(const char *key) const
{
#ifndef PLASMA_NO_PACKAGE_EXTRADATA
    QHash<QByteArray, ContentStructure>::const_iterator it = d->contents.constFind(key);
    if (it == d->contents.constEnd()) {
        return QString();
    }

    return it.value().name;
#else
    Q_UNUSED(key);
    return QString();
#endif
}

bool Package::isRequired(const char *key) const
{
    QHash<QByteArray, ContentStructure>::const_iterator it = d->contents.constFind(key);
    if (it == d->contents.constEnd()) {
        return false;
    }

    return it.value().required;
}

QStringList Package::mimeTypes(const char *key) const
{
#ifndef PLASMA_NO_PACKAGE_EXTRADATA
    QHash<QByteArray, ContentStructure>::const_iterator it = d->contents.constFind(key);
    if (it == d->contents.constEnd()) {
        return QStringList();
    }

    if (it.value().mimeTypes.isEmpty()) {
        return d->mimeTypes;
    }

    return it.value().mimeTypes;
#else
    return QStringList();
#endif
}

QString Package::defaultPackageRoot() const
{
    return d->defaultPackageRoot;
}

void Package::setDefaultPackageRoot(const QString &packageRoot)
{
    d.detach();
    d->defaultPackageRoot = packageRoot;
    if (!d->defaultPackageRoot.isEmpty() && !d->defaultPackageRoot.endsWith('/')) {
        d->defaultPackageRoot.append('/');
    }
}

void Package::setFallbackPackage(const Plasma::Package &package)
{
    if ((d->fallbackPackage && d->fallbackPackage->path() == package.path() && d->fallbackPackage->metadata() == package.metadata()) ||
        //can't be fallback of itself
        (package.path() == path() && package.metadata() == metadata()) ||
        d->hasCycle(package)) {
        return;
    }

    d->fallbackPackage = new Package(package);
}

Plasma::Package Package::fallbackPackage() const
{
    if (d->fallbackPackage) {
        return (*d->fallbackPackage);
    } else {
        return Package();
    }
}

QString Package::servicePrefix() const
{
    return d->servicePrefix;
}

void Package::setServicePrefix(const QString &servicePrefix)
{
    d.detach();
    d->servicePrefix = servicePrefix;
}

bool Package::allowExternalPaths() const
{
    return d->externalPaths;
}

void Package::setAllowExternalPaths(bool allow)
{
    d.detach();
    d->externalPaths = allow;
}

KPluginInfo Package::metadata() const
{
    //qDebug() << "metadata: " << d->path << filePath("metadata");
    if (!d->metadata && !d->path.isEmpty()) {
        const QString metadataPath = filePath("metadata");
        if (!metadataPath.isEmpty()) {
            d->createPackageMetadata(metadataPath);
        } else {
            // d->path might still be a file, if its path has a trailing /,
            // the fileInfo lookup will fail, so remove it.
            QString p = d->path;
            if (p.endsWith("/")) {
                p.chop(1);
            }
            QFileInfo fileInfo(p);

            if (fileInfo.isDir()) {
                d->createPackageMetadata(d->path);
            } else if (fileInfo.exists()) {
                d->path = p;
                d->tempRoot = d->unpack(p);
            }
        }
    }

    if (!d->metadata) {
        d->metadata = new KPluginInfo();
    }

    return *d->metadata;
}

QString PackagePrivate::unpack(const QString &filePath)
{
    KArchive *archive = 0;
    QMimeDatabase db;
    QMimeType mimeType = db.mimeTypeForFile(filePath);

    if (mimeType.inherits("application/zip")) {
        archive = new KZip(filePath);
    } else if (mimeType.inherits("application/x-compressed-tar") || mimeType.inherits("application/x-gzip") ||
               mimeType.inherits("application/x-tar") || mimeType.inherits("application/x-bzip-compressed-tar") ||
               mimeType.inherits("application/x-xz") || mimeType.inherits("application/x-lzma")) {
        archive = new KTar(filePath);
    } else {
        //qWarning() << "Could not open package file, unsupported archive format:" << filePath << mimeType.name();
    }
    QString tempRoot;
    if (archive && archive->open(QIODevice::ReadOnly)) {
        const KArchiveDirectory *source = archive->directory();
        QTemporaryDir tempdir;
        tempdir.setAutoRemove(false);
        tempRoot = tempdir.path() + '/';
        source->copyTo(tempRoot);

        if (!QFile::exists(tempdir.path() + "/metadata.desktop")) {
            // search metadata.desktop, the zip file might have the package contents in a subdirectory
            QDir unpackedPath(tempdir.path());
            const QStringList &entries = unpackedPath.entryList(QDir::Dirs);
            foreach (const QString &pack, entries) {
                if ((pack != "." && pack != "..") &&
                        (QFile::exists(unpackedPath.absolutePath() + '/' + pack + "/metadata.desktop"))) {
                    tempRoot = unpackedPath.absolutePath() + '/' + pack + '/';
                }
            }
        }

        createPackageMetadata(tempRoot);
    } else {
        //qWarning() << "Could not open package file:" << path;
    }

    delete archive;
    return tempRoot;
}

QString Package::filePath(const char *fileType, const QString &filename) const
{
    if (!d->valid) {
        //qDebug() << "package is not valid";
        return d->fallbackFilePath(fileType, filename);
    }

    const QString discoveryKey(fileType + filename);
    if (d->discoveries.contains(discoveryKey)) {
        //qDebug() << "looking for" << discoveryKey << d->discoveries.value(discoveryKey);
        return d->discoveries[discoveryKey];
    }

    QStringList paths;

    if (qstrlen(fileType) != 0) {
        //qDebug()<<d->contents.keys();
        if (!d->contents.contains(fileType)) {
            //qDebug() << "package does not contain" << fileType << filename;
            return d->fallbackFilePath(fileType, filename);
        }

        paths = d->contents[fileType].paths;

        if (paths.isEmpty()) {
            //qDebug() << "no matching path came of it, while looking for" << fileType << filename;
            d->discoveries.insert(discoveryKey, QString());
            return d->fallbackFilePath(fileType, filename);
        }
    } else {
        //when filetype is empty paths is always empty, so try with an empty string
        paths << QString();
    }

    //Nested loop, but in the medium case resolves to just one iteration
    //qDebug() << "prefixes:" << prefixes.count() << prefixes;
    foreach (const QString &contentsPrefix, d->contentsPrefixPaths) {
        const QString prefix(d->path + contentsPrefix);

        foreach (const QString &path, paths) {
            QString file = prefix + path;

            if (!filename.isEmpty()) {
                file.append("/").append(filename);
            }

            //qDebug() << "testing" << file << QFile::exists("/bin/ls") << QFile::exists(file);
            if (QFile::exists(file)) {
                if (d->externalPaths) {
                    //qDebug() << "found" << file;
                    d->discoveries.insert(discoveryKey, file);
                    return file;
                }

                // ensure that we don't return files outside of our base path
                // due to symlink or ../ games
                QDir dir(file);
                QString canonicalized = dir.canonicalPath() + QDir::separator();

                //qDebug() << "testing that" << canonicalized << "is in" << d->path;
                if (canonicalized.startsWith(d->path)) {
                    //qDebug() << "found" << file;
                    d->discoveries.insert(discoveryKey, file);
                    return file;
                }
            }
        }
    }

    //qDebug() << fileType << filename << "does not exist in" << prefixes << "at root" << d->path;
    return d->fallbackFilePath(fileType, filename);
}

QStringList Package::entryList(const char *key) const
{
    if (!d->valid) {
        return QStringList();
    }

    QHash<QByteArray, ContentStructure>::const_iterator it = d->contents.constFind(key);
    if (it == d->contents.constEnd()) {
        //qDebug() << "couldn't find" << key;
        return QStringList();
    }

    //qDebug() << "going to list" << key;
    QStringList list;
    foreach (const QString &prefix, d->contentsPrefixPaths) {
        //qDebug() << "     looking in" << prefix;
        foreach (const QString &path, it.value().paths) {
            //qDebug() << "         looking in" << path;
            if (it.value().directory) {
                //qDebug() << "it's a directory, so trying out" << d->path + prefix + path;
                QDir dir(d->path + prefix + path);

                if (d->externalPaths) {
                    list += dir.entryList(QDir::Files | QDir::Readable);
                } else {
                    // ensure that we don't return files outside of our base path
                    // due to symlink or ../ games
                    QString canonicalized = dir.canonicalPath();
                    if (canonicalized.startsWith(d->path)) {
                        list += dir.entryList(QDir::Files | QDir::Readable);
                    }
                }
            } else {
                const QString fullPath = d->path + prefix + path;
                //qDebug() << "it's a file at" << fullPath << QFile::exists(fullPath);
                if (!QFile::exists(fullPath)) {
                    continue;
                }

                if (d->externalPaths) {
                    list += fullPath;
                } else {
                    QDir dir(fullPath);
                    QString canonicalized = dir.canonicalPath() + QDir::separator();

                    //qDebug() << "testing that" << canonicalized << "is in" << d->path;
                    if (canonicalized.startsWith(d->path)) {
                        list += fullPath;
                    }
                }
            }
        }
    }

    return list;
}

void Package::setPath(const QString &path)
{
    // if the path is already what we have, don't bother
    if (path == d->path) {
        return;
    }

    // our dptr is shared, and it is almost certainly going to change.
    // hold onto the old pointer just in case it does not, however!
    QExplicitlySharedDataPointer<PackagePrivate> oldD(d);
    d.detach();

    // without structure we're doomed
    if (!d->structure) {
        d->path.clear();
        d->discoveries.clear();
        d->valid = false;
        d->checkedValid = true;
        return;
    }

    // empty path => nothing to do
    if (path.isEmpty()) {
        d->path.clear();
        d->discoveries.clear();
        d->valid = false;
        d->structure.data()->pathChanged(this);
        return;
    }

    // now we look for all possible paths, including resolving
    // relative paths against the system search paths
    QStringList paths;
    if (QDir::isRelativePath(path)) {
        QString p;

        if (d->defaultPackageRoot.isEmpty()) {
            p = path % "/";
        } else {
            p = d->defaultPackageRoot % path % "/";
        }

        if (QDir::isRelativePath(p)) {
            paths << QStandardPaths::locateAll(QStandardPaths::GenericDataLocation, p, QStandardPaths::LocateDirectory);
        } else {
            const QDir dir(p);
            if (QFile::exists(dir.canonicalPath())) {
                paths << p;
            }
        }

        //qDebug() << "paths:" << p << paths << d->defaultPackageRoot;
    } else {
        const QDir dir(path);
        if (QFile::exists(dir.canonicalPath())) {
            paths << path;
        }
    }

    QFileInfo fileInfo(path);
    if (fileInfo.isFile() && d->tempRoot.isEmpty()) {
        d->path = path;
        d->tempRoot = d->unpack(path);
    }

    // now we search each path found, caching our previous path to know if
    // anything actually really changed
    const QString previousPath = d->path;
    foreach (const QString &p, paths) {
        d->checkedValid = false;
        d->path = p;

        if (!d->path.endsWith('/')) {
            d->path.append('/');
        }

        // we need to tell the structure we're changing paths ...
        d->structure.data()->pathChanged(this);
        // ... and then testing the results for validity
        if (isValid()) {
            break;
        }
    }


    // if nothing did change, then we go back to the old dptr
    if (d->path == previousPath) {
        d = oldD;
        return;
    }

    // .. but something did change, so we get rid of our discovery cache
    d->discoveries.clear();
    delete d->metadata;
    d->metadata = 0;

    QString fallback;

    // uh-oh, but we didn't end up with anything valid, so we sadly reset ourselves
    // to futility.
    if (!d->valid) {
        d->path.clear();
        d->structure.data()->pathChanged(this);
    }
}

const QString Package::path() const
{
    return d->path;
}

QStringList Package::contentsPrefixPaths() const
{
    return d->contentsPrefixPaths;
}

void Package::setContentsPrefixPaths(const QStringList &prefixPaths)
{
    d.detach();
    d->contentsPrefixPaths = prefixPaths;
    if (d->contentsPrefixPaths.isEmpty()) {
        d->contentsPrefixPaths << QString();
    } else {
        // the code assumes that the prefixes have a trailing slash
        // so let's make that true here
        QMutableStringListIterator it(d->contentsPrefixPaths);
        while (it.hasNext()) {
            it.next();

            if (!it.value().endsWith('/')) {
                it.setValue(it.value() % '/');
            }
        }
    }
}

QString Package::contentsHash() const
{
    if (!d->valid) {
        qWarning() << "can not create hash due to Package being invalid";
        return QString();
    }

    QCryptographicHash hash(QCryptographicHash::Sha1);
    QString metadataPath = d->path + "metadata.desktop";
    if (QFile::exists(metadataPath)) {
        QFile f(metadataPath);
        if (f.open(QIODevice::ReadOnly)) {
            while (!f.atEnd()) {
                hash.addData(f.read(1024));
            }
        } else {
            qWarning() << "could not add" << f.fileName() << "to the hash; file could not be opened for reading.";
        }
    } else {
        qWarning() << "no metadata at" << metadataPath;
    }

    foreach (QString prefix, d->contentsPrefixPaths) {
        const QString basePath = d->path + prefix;
        QDir dir(basePath);

        if (!dir.exists()) {
            return QString();
        }

        d->updateHash(basePath, QString(), dir, hash);
    }

    return hash.result().toHex();
}

void Package::addDirectoryDefinition(const char *key, const QString &path, const QString &name)
{
    ContentStructure s;

    if (d->contents.contains(key)) {
        s = d->contents[key];
        if (s.paths.contains(path) && s.directory == true
#ifndef PLASMA_NO_PACKAGE_EXTRADATA
            && s.name == name
#endif
        ) {
            return;
        }
    }

    d.detach();

#ifndef PLASMA_NO_PACKAGE_EXTRADATA
    if (!name.isEmpty()) {
        s.name = name;
    }
#else
    Q_UNUSED(name)
#endif

    s.paths.append(path);
    s.directory = true;

    d->contents[key] = s;
}

void Package::addFileDefinition(const char *key, const QString &path, const QString &name)
{
    ContentStructure s;

    if (d->contents.contains(key)) {
        s = d->contents[key];
        if (s.paths.contains(path) && s.directory == false
#ifndef PLASMA_NO_PACKAGE_EXTRADATA
            && s.name == name
#endif
        ) {
            return;
        }
    }

    d.detach();
#ifndef PLASMA_NO_PACKAGE_EXTRADATA
    if (!name.isEmpty()) {
        s.name = name;
    }
#else
    Q_UNUSED(name)
#endif

    s.paths.append(path);
    s.directory = false;

    d->contents[key] = s;
}

void Package::removeDefinition(const char *key)
{
    if (d->contents.contains(key)) {
        d.detach();
        d->contents.remove(key);
    }
}

void Package::setRequired(const char *key, bool required)
{
    QHash<QByteArray, ContentStructure>::iterator it = d->contents.find(key);
    if (it == d->contents.end()) {
        return;
    }

    d.detach();
    // have to find the item again after detaching: d->contents is a different object now
    it = d->contents.find(key);
    it.value().required = required;
}

void Package::setDefaultMimeTypes(QStringList mimeTypes)
{
#ifndef PLASMA_NO_PACKAGE_EXTRADATA
    d.detach();
    d->mimeTypes = mimeTypes;
#endif
}

void Package::setMimeTypes(const char *key, QStringList mimeTypes)
{
#ifndef PLASMA_NO_PACKAGE_EXTRADATA
    QHash<QByteArray, ContentStructure>::iterator it = d->contents.find(key);
    if (it == d->contents.end()) {
        return;
    }

    d.detach();
    // have to find the item again after detaching: d->contents is a different object now
    it = d->contents.find(key);
    it.value().mimeTypes = mimeTypes;
#endif
}

QList<const char *> Package::directories() const
{
    QList<const char *> dirs;
    QHash<QByteArray, ContentStructure>::const_iterator it = d->contents.constBegin();
    while (it != d->contents.constEnd()) {
        if (it.value().directory) {
            dirs << it.key();
        }
        ++it;
    }
    return dirs;
}

QList<const char *> Package::requiredDirectories() const
{
    QList<const char *> dirs;
    QHash<QByteArray, ContentStructure>::const_iterator it = d->contents.constBegin();
    while (it != d->contents.constEnd()) {
        if (it.value().directory &&
                it.value().required) {
            dirs << it.key();
        }
        ++it;
    }
    return dirs;
}

QList<const char *> Package::files() const
{
    QList<const char *> files;
    QHash<QByteArray, ContentStructure>::const_iterator it = d->contents.constBegin();
    while (it != d->contents.constEnd()) {
        if (!it.value().directory) {
            files << it.key();
        }
        ++it;
    }
    return files;
}

QList<const char *> Package::requiredFiles() const
{
    QList<const char *> files;
    QHash<QByteArray, ContentStructure>::const_iterator it = d->contents.constBegin();
    while (it != d->contents.constEnd()) {
        if (!it.value().directory && it.value().required) {
            files << it.key();
        }
        ++it;
    }

    return files;
}

KJob *Package::install(const QString &sourcePackage, const QString &packageRoot)
{
    const QString src = sourcePackage;
    const QString dest = packageRoot.isEmpty() ? defaultPackageRoot() : packageRoot;
    //qDebug() << "Source: " << src;
    //qDebug() << "PackageRoot: " << dest;
    KJob *j = d->structure.data()->install(this, src, dest);
    //connect(j, SIGNAL(finished(bool)), SLOT(installFinished(bool)));
    return j;
}

KJob *Package::uninstall(const QString &packageName, const QString &packageRoot)
{
    d->createPackageMetadata(packageRoot + packageName);
    return d->structure.data()->uninstall(this, packageRoot);
}

PackagePrivate::PackagePrivate()
    : QSharedData(),
      servicePrefix("plasma-applet-"),
      fallbackPackage(0),
      metadata(0),
      externalPaths(false),
      valid(false),
      checkedValid(false)
{
    contentsPrefixPaths << "contents/";
}

PackagePrivate::PackagePrivate(const PackagePrivate &other)
    : QSharedData()
{
    *this = other;
    metadata = 0;
}

PackagePrivate::~PackagePrivate()
{
    if (!tempRoot.isEmpty()) {
        QDir dir(tempRoot);
        dir.removeRecursively();
    }
    delete metadata;
    delete fallbackPackage;
}

PackagePrivate &PackagePrivate::operator=(const PackagePrivate &rhs)
{
    if (&rhs == this) {
        return *this;
    }

    structure = rhs.structure;
    if (rhs.fallbackPackage) {
        fallbackPackage = new Package(*rhs.fallbackPackage);
    } else {
        fallbackPackage = 0;
    }
    path = rhs.path;
    contentsPrefixPaths = rhs.contentsPrefixPaths;
    servicePrefix = rhs.servicePrefix;
    contents = rhs.contents;
#ifndef PLASMA_NO_PACKAGE_EXTRADATA
    mimeTypes = rhs.mimeTypes;
#endif
    defaultPackageRoot = rhs.defaultPackageRoot;
    servicePrefix = rhs.servicePrefix;
    metadata = 0;
    externalPaths = rhs.externalPaths;
    valid = rhs.valid;
    return *this;
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
                qWarning() << "could not add" << f.fileName() << "to the hash; file could not be opened for reading. "
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

void PackagePrivate::createPackageMetadata(const QString &path)
{
    delete metadata;
    QString metadataPath(path + "/metadata.desktop");
    if (!QFile::exists(metadataPath)) {
        qWarning() << "No metadata file in the package, expected it at:" << metadataPath;
        metadataPath.clear();
    }

    metadata = new KPluginInfo(metadataPath);
}

QString PackagePrivate::fallbackFilePath(const char *key, const QString &filename) const
{
    //don't fallback if the package isn't valid and never fallback the metadata file
    if (qstrcmp(key, "metadata") != 0 && fallbackPackage && fallbackPackage->isValid()) {
        return fallbackPackage->filePath(key, filename);
    } else {
        return QString();
    }
}

bool PackagePrivate::hasCycle(const Plasma::Package &package)
{
    if (!package.d->fallbackPackage) {
        return false;
    }

    //This is the Floyd cycle detection algorithm
    //http://en.wikipedia.org/wiki/Cycle_detection#Tortoise_and_hare
    Plasma::Package *slowPackage = const_cast<Plasma::Package *>(&package);
    Plasma::Package *fastPackage = const_cast<Plasma::Package *>(&package);

    while (fastPackage && fastPackage->d->fallbackPackage) {
        //consider two packages the same if they have the same metadata
        if ((fastPackage->d->fallbackPackage->metadata().isValid() && fastPackage->d->fallbackPackage->metadata() == slowPackage->metadata()) ||
            (fastPackage->d->fallbackPackage->d->fallbackPackage && fastPackage->d->fallbackPackage->d->fallbackPackage->metadata().isValid() && fastPackage->d->fallbackPackage->d->fallbackPackage->metadata() == slowPackage->metadata())) {
            qWarning() << "Warning: the fallback chain of " << package.metadata().pluginName() << "contains a cyclical dependency.";
            return true;
        }
        fastPackage = fastPackage->d->fallbackPackage->d->fallbackPackage;
        slowPackage = slowPackage->d->fallbackPackage;
    }
    return false;
}

} // Namespace
