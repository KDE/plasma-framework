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

#include <QDBusInterface>
#include <QDBusPendingCall>
#include <QFile>
#include <QIODevice>
#include <QRegExp>
#include <QtNetwork/QHostInfo>
#include <qtemporarydir.h>

#include <karchive.h>
#include <kdebug.h>
#include <kdesktopfile.h>
#include <kmimetype.h>
#include <kservicetypetrader.h>

#include "config-plasma.h"

#include <kplugininfo.h>
#include <kstandarddirs.h>
#include <ktar.h>
#include <ktempdir.h>
#include <ktemporaryfile.h>
#include <kzip.h>
#include <kdebug.h>

#include "packagestructure.h"
#include "pluginloader.h"
#include "private/package_p.h"
#include "private/packages_p.h"

namespace Plasma
{

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

Package::Package(PackageStructure *structure)
    : d(new PackagePrivate())
{
    d->structure = structure;
    if (d->structure) {
        d->structure.data()->initPackage(this);
    }
}

Package::Package()
    : d(new PackagePrivate(PackageStructure::Ptr(0), QString()))
{
}

Package::Package(const QString &packageRoot, const QString &package,
                 PackageStructure::Ptr structure)
    : d(new PackagePrivate(structure, packageRoot + '/' + package))
{
}

Package::Package(const QString &packagePath, PackageStructure::Ptr structure)
    : d(new PackagePrivate(structure, packagePath))
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

bool Package::isValid() const
{
    if (!d->valid) {
        return false;
    }

    //search for the file in all prefixes and in all possible paths for each prefix
    //even if it's a big nested loop, usually there is one prefix and one location
    //so shouldn't cause too much disk access
    QHashIterator<QByteArray, ContentStructure> it(d->contents);
    while (it.hasNext()) {
        it.next();
        if (!it.value().required) {
            continue;
        }

        bool failed = true;
        foreach (const QString &path, it.value().paths) {
            foreach (const QString &prefix, d->contentsPrefixPaths) {
                if (QFile::exists(d->path + prefix + path)) {
                    failed = false;
                    break;
                }
            }
            if (!failed) {
                break;
            }
        }

        if (failed) {
            kWarning() << "Could not find required" << (it.value().directory ? "directory" : "file") << it.key();
            d->valid = false;
            return false;
        }
    }

    return true;
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
    if (!d->metadata && !d->path.isEmpty()) {
        const QString metadataPath = filePath("metadata");
        if (!metadataPath.isEmpty()) {
            d->createPackageMetadata(metadataPath);
        } else {
            QFileInfo fileInfo(d->path);

            if (fileInfo.isDir()) {
                d->createPackageMetadata(d->path);
            } else if (fileInfo.exists()) {
                KArchive *archive = 0;
                KMimeType::Ptr mimeType = KMimeType::findByPath(d->path);

                if (mimeType->is("application/zip")) {
                    archive = new KZip(d->path);
                } else if (mimeType->is("application/x-compressed-tar") || mimeType->is("application/x-gzip") ||
                           mimeType->is("application/x-tar") || mimeType->is("application/x-bzip-compressed-tar")) {
                    archive = new KTar(d->path);
                } else {
                    kWarning() << "Could not open package file, unsupported archive format:" << d->path << mimeType->name();
                }

                if (archive && archive->open(QIODevice::ReadOnly)) {
                    const KArchiveDirectory *source = archive->directory();
                    QTemporaryDir tempdir;
                    source->copyTo(tempdir.path() + '/');
                    d->createPackageMetadata(tempdir.path() + '/');
                } else {
                    kWarning() << "Could not open package file:" << d->path;
                }

                delete archive;
            }
        }
    }

    if (!d->metadata) {
        d->metadata = new KPluginInfo();
    }

    return *d->metadata;
}

QString Package::filePath(const char *fileType, const QString &filename) const
{
    if (!d->valid) {
        //kDebug() << "package is not valid";
        return QString();
    }

    const QString discoveryKey(fileType + filename);
    if (d->discoveries.contains(discoveryKey)) {
    //qDebug() << "looking for" << discoveryKey << d->discoveries.value(discoveryKey);
        return d->discoveries[discoveryKey];
    }

    QStringList paths;

    if (qstrlen(fileType) != 0) {
        if (!d->contents.contains(fileType)) {
            //kDebug() << "package does not contain" << fileType << filename;
            return QString();
        }

        paths = d->contents[fileType].paths;

        if (paths.isEmpty()) {
            //kDebug() << "no matching path came of it, while looking for" << fileType << filename;
            d->discoveries.insert(discoveryKey, QString());
            return QString();
        }
    } else {
        //when filetype is empty paths is always empty, so try with an empty string
        paths << QString();
    }

    //Nested loop, but in the medium case resolves to just one iteration
    //kDebug() << "prefixes:" << prefixes.count() << prefixes;
    foreach (const QString &contentsPrefix, d->contentsPrefixPaths) {
        const QString prefix(d->path + contentsPrefix);

        foreach (const QString &path, paths) {
            QString file = prefix + path;

            if (!filename.isEmpty()) {
                file.append("/").append(filename);
            }

            //kDebug() << "testing" << file << QFile::exists("/bin/ls") << QFile::exists(file);
            if (QFile::exists(file)) {
                if (d->externalPaths) {
                    //kDebug() << "found" << file;
                    d->discoveries.insert(discoveryKey, file);
                    return file;
                }

                // ensure that we don't return files outside of our base path
                // due to symlink or ../ games
                QDir dir(file);
                QString canonicalized = dir.canonicalPath() + QDir::separator();

                //kDebug() << "testing that" << canonicalized << "is in" << d->path;
                if (canonicalized.startsWith(d->path)) {
                    //kDebug() << "found" << file;
                    d->discoveries.insert(discoveryKey, file);
                    return file;
                }
            }
        }
    }

    //kDebug() << fileType << filename << "does not exist in" << prefixes << "at root" << d->path;
    return QString();
}

QStringList Package::entryList(const char *key) const
{
    if (!d->valid) {
        return QStringList();
    }

    QHash<QByteArray, ContentStructure>::const_iterator it = d->contents.constFind(key);
    if (it == d->contents.constEnd()) {
        //kDebug() << "couldn't find" << key;
        return QStringList();
    }

    //kDebug() << "going to list" << key;
    QStringList list;
    foreach (const QString &prefix, d->contentsPrefixPaths) {
        //kDebug() << "     looking in" << prefix;
        foreach (const QString &path, it.value().paths) {
            //kDebug() << "         looking in" << path;
            if (it.value().directory) {
                //kDebug() << "it's a directory, so trying out" << d->path + prefix + path;
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
                //kDebug() << "it's a file at" << fullPath << QFile::exists(fullPath);
                if (!QFile::exists(fullPath)) {
                    continue;
                }

                if (d->externalPaths) {
                    list += fullPath;
                } else {
                    QDir dir(fullPath);
                    QString canonicalized = dir.canonicalPath() + QDir::separator();

                    //kDebug() << "testing that" << canonicalized << "is in" << d->path;
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
    if (path == d->path) {
        return;
    }

    d.detach();
    d->discoveries.clear();
    if (path.isEmpty()) {
        d->path.clear();
        d->valid = false;

        if (d->structure) {
            d->structure.data()->pathChanged(this);
        }

        return;
    }

    QDir dir(path);
    if (dir.isRelative()) {
        QString location;
        if (!d->defaultPackageRoot.isEmpty()) {
            dir.setPath(d->defaultPackageRoot);
            if (dir.isRelative()) {
                location = QStandardPaths::locate(QStandardPaths::GenericDataLocation, d->defaultPackageRoot + path);
            } else {
                location = d->defaultPackageRoot + path;
            }
        }

        if (location.isEmpty()) {
            location = QStandardPaths::locate(QStandardPaths::GenericDataLocation, path);

            if (location.isEmpty()) {
                d->path.clear();
                d->valid = false;
                return;
            }
        }

        dir.setPath(location);
    }

    QString basePath = dir.canonicalPath();
    bool valid = QFile::exists(basePath);

    if (valid) {
        QFileInfo info(basePath);
        if (info.isDir() && !basePath.endsWith('/')) {
            basePath.append('/');
        }
        //kDebug() << "basePath is" << basePath;
    } else {
#ifndef NDEBUG
        kDebug() << path << "invalid, basePath is" << basePath;
#endif
        return;
    }

    if (d->path == basePath) {
        return;
    }

    d->path = basePath;
    delete d->metadata;
    d->metadata = 0;
    d->valid = !d->path.isEmpty();

    if (d->structure) {
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
    }
}

QString Package::contentsHash() const
{
    if (!d->valid) {
        kWarning() << "can not create hash due to Package being invalid";
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
            kWarning() << "could not add" << f.fileName() << "to the hash; file could not be opened for reading.";
        }
    } else {
        kWarning() << "no metadata at" << metadataPath;
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
    }

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
    }

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
    d->contents.remove(key);
}

void Package::setRequired(const char *key, bool required)
{
    QHash<QByteArray, ContentStructure>::iterator it = d->contents.find(key);
    if (it == d->contents.end()) {
        return;
    }

    d.detach();
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
    it.value().mimeTypes = mimeTypes;
#endif
}

QList<const char*> Package::directories() const
{
    QList<const char*> dirs;
    QHash<QByteArray, ContentStructure>::const_iterator it = d->contents.constBegin();
    while (it != d->contents.constEnd()) {
        if (it.value().directory) {
            dirs << it.key();
        }
        ++it;
    }
    return dirs;
}

QList<const char*> Package::requiredDirectories() const
{
    QList<const char*> dirs;
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

QList<const char*> Package::files() const
{
    QList<const char*> files;
    QHash<QByteArray, ContentStructure>::const_iterator it = d->contents.constBegin();
    while (it != d->contents.constEnd()) {
        if (!it.value().directory) {
            files << it.key();
        }
        ++it;
    }
    return files;
}

QList<const char*> Package::requiredFiles() const
{
    QList<const char*> files;
    QHash<QByteArray, ContentStructure>::const_iterator it = d->contents.constBegin();
    while (it != d->contents.constEnd()) {
        if (!it.value().directory && it.value().required) {
            files << it.key();
        }
        ++it;
    }

    return files;
}

bool Package::installPackage(const QString &package, const QString &packageRoot)
{
    if (d->structure) {
        return d->structure.data()->installPackage(this, package, packageRoot);
    }

    return PackagePrivate::installPackage(package, packageRoot, d->servicePrefix);
}

bool PackagePrivate::installPackage(const QString &package, const QString &packageRoot, const QString &servicePrefix)
{
    //TODO: report *what* failed if something does fail
    QDir root(packageRoot);

    if (!root.exists()) {
        QDir().mkpath(packageRoot);
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
    QTemporaryDir tempdir;
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
                   mimetype->is("application/x-tar")|| mimetype->is("application/x-bzip-compressed-tar") ||
                   mimetype->is("application/x-xz") || mimetype->is("application/x-lzma")) {
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
        path = tempdir.path() + '/';

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

    KPluginInfo meta(metadataPath);
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
        const bool ok = copyFolder(path, targetName);
        removeFolder(path);
        if (!ok) {
            kWarning() << "Could not move package to destination:" << targetName;
            return false;
        }
    } else {
        kDebug() << "************************** 12";
        // it's a directory containing the stuff, so copy the contents rather
        // than move them
        const bool ok = copyFolder(path, targetName);
        kDebug() << "************************** 13";
        if (!ok) {
            kWarning() << "Could not copy package to destination:" << targetName;
            return false;
        }
    }

    if (archivedPackage) {
        // no need to remove the temp dir (which has been successfully moved if it's an archive)
        tempdir.setAutoRemove(false);
    }

    if (!servicePrefix.isEmpty()) {
        // and now we register it as a service =)
        kDebug() << "************************** 1";
        QString metaPath = targetName + "/metadata.desktop";
        kDebug() << "************************** 2";
        KDesktopFile df(metaPath);
        KConfigGroup cg = df.desktopGroup();
        kDebug() << "************************** 3";

        // Q: should not installing it as a service disqualify it?
        // Q: i don't think so since KServiceTypeTrader may not be
        // used by the installing app in any case, and the
        // package is properly installed - aseigo

        //TODO: reduce code duplication with registerPackage below

        const QString serviceName = servicePrefix + meta.pluginName() + ".desktop";

        QString service = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QLatin1String("/kde5/services/") + serviceName;
        kDebug() << "************************** 4";
        const bool ok = QFile::copy(metaPath, service);
        kDebug() << "************************** 5";
        if (ok) {
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
            kWarning() << "Could not register package as service (this is not necessarily fatal):" << serviceName;
        }
        kDebug() << "************************** 7";
    }

    QDBusInterface sycoca("org.kde.kded5", "/kbuildsycoca");
    sycoca.asyncCall("recreate");
    return true;
}

bool Package::uninstallPackage(const QString &packageName, const QString &packageRoot)
{
    if (d->structure) {
        return d->structure.data()->uninstallPackage(this, packageName, packageRoot);
    }

    return PackagePrivate::uninstallPackage(packageName, packageRoot, d->servicePrefix);
}

bool PackagePrivate::uninstallPackage(const QString &packageName, const QString &packageRoot, const QString &servicePrefix)
{
    // We need to remove the package directory and its metadata file.
    const QString targetName = packageRoot + '/' + packageName;

    if (!QFile::exists(targetName)) {
        kWarning() << targetName << "does not exist";
        return false;
    }

    const QString serviceName = servicePrefix + packageName + ".desktop";

    QString service = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QLatin1String("/kde5/services/") + serviceName;
#ifndef NDEBUG
    kDebug() << "Removing service file " << service;
#endif
    bool ok = QFile::remove(service);

    if (!ok) {
        kWarning() << "Unable to remove " << service;
    }

    ok = removeFolder(targetName);
    const QString errorString("unknown");
    if (!ok) {
        kWarning() << "Could not delete package from:" << targetName << " : " << errorString;
        return false;
    }

    QDBusInterface sycoca("org.kde.kded5", "/kbuildsycoca");
    sycoca.asyncCall("recreate");
    return true;
}

PackagePrivate::PackagePrivate()
        : QSharedData(),
          servicePrefix("plasma-applet-"),
          metadata(0),
          externalPaths(false),
          valid(false)
{
    contentsPrefixPaths << "contents/";

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
        QFile::copy(iconPath, installedIcon);
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
    if (structure) {
        if (p.isEmpty()) {
            structure->setPath(structure->defaultPackageRoot());
        } else {
            structure->setPath(p);
        }
    }

    valid = structure && !structure->path().isEmpty();
}

PackagePrivate::PackagePrivate(const PackageStructure::Ptr st, const QString &packageRoot, const QString &path)
        : structure(st),
          service(0)
{
    if (structure) {
        if (packageRoot.isEmpty()) {
            structure->setPath(structure->defaultPackageRoot()%"/"%path);
        } else {
            structure->setPath(packageRoot%"/"%path);
        }
    }

    valid = structure && !structure->path().isEmpty();
}

PackagePrivate::PackagePrivate(const PackagePrivate &other)
    : QSharedData()
{
    *this = other;
    metadata = 0;
}

PackagePrivate::~PackagePrivate()
{
    delete metadata;
}

PackagePrivate &PackagePrivate::operator=(const PackagePrivate &rhs)
{
    if (&rhs == this) {
        return *this;
    }

    structure = rhs.structure;
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

void PackagePrivate::createPackageMetadata(const QString &path)
{
    delete metadata;

    QString metadataPath(path + "/metadata.desktop");
    if (!QFile::exists(metadataPath)) {
        kWarning() << "No metadata file in the package, expected it at:" << metadataPath;
        metadataPath.clear();
    }

    metadata = new KPluginInfo(metadataPath);
}

} // Namespace
