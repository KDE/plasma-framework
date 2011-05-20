/******************************************************************************
*   Copyright 2007 by Aaron Seigo <aseigo@kde.org>                            *
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

#include "packagestructure.h"

#include "config-plasma.h"

#include <QDir>
#include <QMap>
#include <QFileInfo>

#include <kconfiggroup.h>
#include <kdebug.h>
#include <kdesktopfile.h>
#ifndef PLASMA_NO_KIO
#include <kio/copyjob.h>
#include <kio/deletejob.h>
#include <kio/jobclasses.h>
#include <kio/job.h>
#endif
#include <kmimetype.h>
#include <kstandarddirs.h>
#include <kservicetypetrader.h>
#include <ktar.h>
#include <ktemporaryfile.h>
#include <ktempdir.h>
#include <kurl.h>
#include <kzip.h>

#include "package.h"
#include "private/packages_p.h"
#include "theme.h"

namespace Plasma
{

class ContentStructure
{
    public:
        ContentStructure()
            : directory(false),
              required(false)
        {
        }

        ContentStructure(const ContentStructure &other)
        {
            paths = other.paths;
            name = other.name;
            mimeTypes = other.mimeTypes;
            directory = other.directory;
            required = other.required;
        }

        QStringList paths;
        QString name;
        QStringList mimeTypes;
        bool directory : 1;
        bool required : 1;
};

class PackageStructurePrivate
{
public:
    PackageStructurePrivate(const QString &t)
        : type(t),
          packageRoot("plasma/plasmoids"),
          servicePrefix("plasma-applet-"),
          metadata(0),
          externalPaths(false)
    {
        contentsPrefixPaths << "contents/";
    }

    ~PackageStructurePrivate()
    {
        delete metadata;
    }

    void createPackageMetadata(const QString &path);
    QStringList entryList(const QString &prefix, const QString &requestedPath);

    static QHash<QString, PackageStructure::Ptr> structures;

    QString type;
    QString path;
    QStringList contentsPrefixPaths;
    QString packageRoot;
    QString servicePrefix;
    QMap<QByteArray, ContentStructure> contents;
    QStringList mimeTypes;
    PackageMetadata *metadata;
    bool externalPaths;
 };

QHash<QString, PackageStructure::Ptr> PackageStructurePrivate::structures;

PackageStructure::PackageStructure(QObject *parent, const QString &type)
    : QObject(parent),
      d(new PackageStructurePrivate(type))
{
}

PackageStructure::~PackageStructure()
{
    delete d;
}

PackageStructure::Ptr PackageStructure::load(const QString &packageFormat)
{
    if (packageFormat.isEmpty()) {
        return Ptr(new PackageStructure());
    }

    PackageStructure::Ptr structure = PackageStructurePrivate::structures[packageFormat];

    if (structure) {
        return structure;
    }

    if (packageFormat == "Plasma/Applet") {
        structure = defaultPackageStructure(AppletComponent);
        structure->d->type = "Plasma/Applet";
    } else if (packageFormat == "Plasma/DataEngine") {
        structure = defaultPackageStructure(DataEngineComponent);
        structure->d->type = "Plasma/DataEngine";
    } else if (packageFormat == "Plasma/Runner") {
        structure = defaultPackageStructure(RunnerComponent);
        structure->d->type = "Plasma/Runner";
    } else if (packageFormat == "Plasma/Wallpaper") {
        structure = defaultPackageStructure(WallpaperComponent);
        structure->d->type = "Plasma/Wallpaper";
    } else if (packageFormat == "Plasma/Theme") {
        structure = Theme::packageStructure();
        structure->d->type = "Plasma/Theme";
    }

    if (structure) {
        PackageStructurePrivate::structures[packageFormat] = structure;
        return structure;
    }

    // first we check for plugins in sycoca
    QString constraint = QString("[X-KDE-PluginInfo-Name] == '%1'").arg(packageFormat);
    KService::List offers =
        KServiceTypeTrader::self()->query("Plasma/PackageStructure", constraint);

    QVariantList args;
    QString error;
    foreach (const KService::Ptr &offer, offers) {
        PackageStructure::Ptr structure(
            offer->createInstance<Plasma::PackageStructure>(0, args, &error));

        if (structure) {
            return structure;
        }

        kDebug() << "Couldn't load PackageStructure for" << packageFormat
                 << "! reason given: " << error;
    }

    // if that didn't give us any love, then we try to load from a config file
    structure = new PackageStructure();
    QString configPath("plasma/packageformats/%1rc");
    configPath = KStandardDirs::locate("data", configPath.arg(packageFormat));

    if (!configPath.isEmpty()) {
        KConfig config(configPath);
        structure->read(&config);
        PackageStructurePrivate::structures[packageFormat] = structure;
        return structure;
    }

    // try to load from absolute file path
    KUrl url(packageFormat);
    if (url.isLocalFile()) {
        KConfig config(url.toLocalFile(), KConfig::SimpleConfig);
        structure->read(&config);
        PackageStructurePrivate::structures[structure->type()] = structure;
    }
#ifndef PLASMA_NO_KIO
    else {
        KTemporaryFile tmp;
        if (tmp.open()) {
            KIO::Job *job = KIO::file_copy(url, KUrl(tmp.fileName()),
                                           -1, KIO::Overwrite | KIO::HideProgressInfo);
            if (job->exec()) {
                KConfig config(tmp.fileName(), KConfig::SimpleConfig);
                structure->read(&config);
                PackageStructurePrivate::structures[structure->type()] = structure;
            }
        }
    }
#endif

    return structure;
}

PackageStructure &PackageStructure::operator=(const PackageStructure &rhs)
{
    if (this == &rhs) {
        return *this;
    }

    *d = *rhs.d;
    return *this;
}

QString PackageStructure::type() const
{
    return d->type;
}

QList<const char*> PackageStructure::directories() const
{
    QList<const char*> dirs;
    QMap<QByteArray, ContentStructure>::const_iterator it = d->contents.constBegin();
    while (it != d->contents.constEnd()) {
        if (it.value().directory) {
            dirs << it.key();
        }
        ++it;
    }
    return dirs;
}

QList<const char*> PackageStructure::requiredDirectories() const
{
    QList<const char*> dirs;
    QMap<QByteArray, ContentStructure>::const_iterator it = d->contents.constBegin();
    while (it != d->contents.constEnd()) {
        if (it.value().directory &&
            it.value().required) {
            dirs << it.key();
        }
        ++it;
    }
    return dirs;
}

QList<const char*> PackageStructure::files() const
{
    QList<const char*> files;
    QMap<QByteArray, ContentStructure>::const_iterator it = d->contents.constBegin();
    while (it != d->contents.constEnd()) {
        if (!it.value().directory) {
            files << it.key();
        }
        ++it;
    }
    return files;
}

QList<const char*> PackageStructure::requiredFiles() const
{
    QList<const char*> files;
    QMap<QByteArray, ContentStructure>::const_iterator it = d->contents.constBegin();
    while (it != d->contents.constEnd()) {
        if (!it.value().directory && it.value().required) {
            files << it.key();
        }
        ++it;
    }
    return files;
}

QStringList PackageStructure::entryList(const char *key)
{
    QString p = path(key);

    if (p.isEmpty()) {
        return QStringList();
    }

    QStringList list;
    if (d->contentsPrefixPaths.isEmpty()) {
        // no prefixes is the same as d->contentsPrefixPths with QStringList() << QString()
        list << d->entryList(QString(), p);
    } else {
        foreach (QString prefix, d->contentsPrefixPaths) {
            list << d->entryList(prefix, p);
        }
    }

    return list;
}

QStringList PackageStructurePrivate::entryList(const QString &prefix, const QString &requestedPath)
{
    QDir dir(path + prefix + requestedPath);

    if (externalPaths) {
        return dir.entryList(QDir::Files | QDir::Readable);
    }

    // ensure that we don't return files outside of our base path
    // due to symlink or ../ games
    QString canonicalized = dir.canonicalPath();
    if (canonicalized.startsWith(path)) {
        return dir.entryList(QDir::Files | QDir::Readable);
    }

    return QStringList();
}

void PackageStructure::addDirectoryDefinition(const char *key,
                                              const QString &path, const QString &name)
{
    ContentStructure s;

    if (d->contents.contains(key)) {
        s = d->contents[key];
    }

    if (!name.isEmpty()) {
        s.name = name;
    }

    s.paths.append(path);
    s.directory = true;

    d->contents[key] = s;
}

void PackageStructure::addFileDefinition(const char *key, const QString &path, const QString &name)
{
    ContentStructure s;

    if (d->contents.contains(key)) {
        s = d->contents[key];
    }

    if (!name.isEmpty()) {
        s.name = name;
    }

    s.paths.append(path);
    s.directory = false;

    d->contents[key] = s;
}

void PackageStructure::removeDefinition(const char *key)
{
    d->contents.remove(key);
}

QString PackageStructure::path(const char *key) const
{
    //kDebug() << "looking for" << key;
    QMap<QByteArray, ContentStructure>::const_iterator it = d->contents.constFind(key);
    if (it == d->contents.constEnd()) {
        return QString();
    }

    //kDebug() << "found" << key << "and the value is" << it.value().paths.first();
    return it.value().paths.first();
}

QStringList PackageStructure::searchPath(const char *key) const
{
    //kDebug() << "looking for" << key;
    QMap<QByteArray, ContentStructure>::const_iterator it = d->contents.constFind(key);
    if (it == d->contents.constEnd()) {
        return QStringList();
    }

    //kDebug() << "found" << key << "and the value is" << it.value().paths;
    return it.value().paths;
}

QString PackageStructure::name(const char *key) const
{
    QMap<QByteArray, ContentStructure>::const_iterator it = d->contents.constFind(key);
    if (it == d->contents.constEnd()) {
        return QString();
    }

    return it.value().name;
}

void PackageStructure::setRequired(const char *key, bool required)
{
    QMap<QByteArray, ContentStructure>::iterator it = d->contents.find(key);
    if (it == d->contents.end()) {
        return;
    }

    it.value().required = required;
}

bool PackageStructure::isRequired(const char *key) const
{
    QMap<QByteArray, ContentStructure>::const_iterator it = d->contents.constFind(key);
    if (it == d->contents.constEnd()) {
        return false;
    }

    return it.value().required;
}

void PackageStructure::setDefaultMimeTypes(QStringList mimeTypes)
{
    d->mimeTypes = mimeTypes;
}

void PackageStructure::setMimeTypes(const char *key, QStringList mimeTypes)
{
    QMap<QByteArray, ContentStructure>::iterator it = d->contents.find(key);
    if (it == d->contents.end()) {
        return;
    }

    it.value().mimeTypes = mimeTypes;
}

QStringList PackageStructure::mimeTypes(const char *key) const
{
    QMap<QByteArray, ContentStructure>::const_iterator it = d->contents.constFind(key);
    if (it == d->contents.constEnd()) {
        return QStringList();
    }

    if (it.value().mimeTypes.isEmpty()) {
        return d->mimeTypes;
    }

    return it.value().mimeTypes;
}

void PackageStructure::setPath(const QString &path)
{
    KUrl url(path);
    QDir dir(url.toLocalFile());
    QString basePath = dir.canonicalPath();
    bool valid = QFile::exists(basePath);

    if (valid) {
        QFileInfo info(basePath);
        if (info.isDir() && !basePath.endsWith('/')) {
            basePath.append('/');
        }
        //kDebug() << "basePath is" << basePath;
    } else {
        kDebug() << path << "invalid, basePath is" << basePath;
        return;
    }

    if (d->path == basePath) {
        return;
    }

    d->path = basePath;
    delete d->metadata;
    d->metadata = 0;
    pathChanged();
}

QString PackageStructure::path() const
{
    return d->path;
}

void PackageStructure::pathChanged()
{
    // default impl does nothing, this is a hook for subclasses.
}

void PackageStructure::read(const KConfigBase *config)
{
    d->contents.clear();
    d->mimeTypes.clear();
    KConfigGroup general(config, QString());
    d->type = general.readEntry("Type", QString());
    d->contentsPrefixPaths = general.readEntry("ContentsPrefixPaths", d->contentsPrefixPaths);
    d->packageRoot = general.readEntry("DefaultPackageRoot", d->packageRoot);
    d->externalPaths = general.readEntry("AllowExternalPaths", d->externalPaths);

    QStringList groups = config->groupList();
    foreach (const QString &group, groups) {
        KConfigGroup entry(config, group);
        QByteArray key = group.toAscii();

        QString path = entry.readEntry("Path", QString());
        QString name = entry.readEntry("Name", QString());
        QStringList mimeTypes = entry.readEntry("Mimetypes", QStringList());
        bool directory = entry.readEntry("Directory", false);
        bool required = entry.readEntry("Required", false);

        if (directory) {
            addDirectoryDefinition(key, path, name);
        } else {
            addFileDefinition(key, path, name);
        }

        setMimeTypes(key, mimeTypes);
        setRequired(key, required);
    }
}

void PackageStructure::write(KConfigBase *config) const
{
    KConfigGroup general = KConfigGroup(config, "");
    general.writeEntry("Type", type());
    general.writeEntry("ContentsPrefixPaths", d->contentsPrefixPaths);
    general.writeEntry("DefaultPackageRoot", d->packageRoot);
    general.writeEntry("AllowExternalPaths", d->externalPaths);

    QMap<QByteArray, ContentStructure>::const_iterator it = d->contents.constBegin();
    while (it != d->contents.constEnd()) {
        KConfigGroup group = config->group(it.key());
        group.writeEntry("Path", it.value().paths);
        group.writeEntry("Name", it.value().name);
        if (!it.value().mimeTypes.isEmpty()) {
            group.writeEntry("Mimetypes", it.value().mimeTypes);
        }
        if (it.value().directory) {
            group.writeEntry("Directory", true);
        }
        if (it.value().required) {
            group.writeEntry("Required", true);
        }

        ++it;
    }
}

QString PackageStructure::contentsPrefix() const
{
    return d->contentsPrefixPaths.isEmpty() ? QString() : d->contentsPrefixPaths.first();
}

void PackageStructure::setContentsPrefix(const QString &prefix)
{
    d->contentsPrefixPaths.clear();
    d->contentsPrefixPaths << prefix;
}

QStringList PackageStructure::contentsPrefixPaths() const
{
    return d->contentsPrefixPaths;
}

void PackageStructure::setContentsPrefixPaths(const QStringList &prefixPaths)
{
    d->contentsPrefixPaths = prefixPaths;
}

bool PackageStructure::installPackage(const QString &package, const QString &packageRoot)
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
#ifndef PLASMA_NO_KIO
        KIO::CopyJob *job = KIO::move(KUrl(path), KUrl(targetName), KIO::HideProgressInfo);
        const bool ok = job->exec();
        const QString errorString = job->errorString();
#else
        const bool ok = copyFolder(path, targetName);
        removeFolder(path);
        const QString errorString("unknown");
#endif
        if (!ok) {
            kWarning() << "Could not move package to destination:" << targetName << " : " << errorString;
            return false;
        }
    } else {
        // it's a directory containing the stuff, so copy the contents rather
        // than move them
#ifndef PLASMA_NO_KIO
        KIO::CopyJob *job = KIO::copy(KUrl(path), KUrl(targetName), KIO::HideProgressInfo);
        const bool ok = job->exec();
        const QString errorString = job->errorString();
#else
        const bool ok = copyFolder(path, targetName);
        const QString errorString("unknown");
#endif
        if (!ok) {
            kWarning() << "Could not copy package to destination:" << targetName << " : " << errorString;
            return false;
        }
    }

    if (archivedPackage) {
        // no need to remove the temp dir (which has been successfully moved if it's an archive)
        tempdir.setAutoRemove(false);
    }

    if (!d->servicePrefix.isEmpty()) {
        // and now we register it as a service =)
        QString metaPath = targetName + "/metadata.desktop";
        KDesktopFile df(metaPath);
        KConfigGroup cg = df.desktopGroup();

        // Q: should not installing it as a service disqualify it?
        // Q: i don't think so since KServiceTypeTrader may not be
        // used by the installing app in any case, and the
        // package is properly installed - aseigo

        //TODO: reduce code duplication with registerPackage below

        QString serviceName = d->servicePrefix + meta.pluginName();

        QString service = KStandardDirs::locateLocal("services", serviceName + ".desktop");
#ifndef PLASMA_NO_KIO
        KIO::FileCopyJob *job = KIO::file_copy(metaPath, service, -1, KIO::HideProgressInfo);
        const bool ok = job->exec();
        const QString errorString = job->errorString();
#else
        const bool ok = QFile::copy(metaPath, service);
        const QString errorString("unknown");
#endif
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
            kWarning() << "Could not register package as service (this is not necessarily fatal):" << serviceName << " : " << errorString;
        }
    }

    return true;
}

bool PackageStructure::uninstallPackage(const QString &packageName, const QString &packageRoot)
{
    // We need to remove the package directory and its metadata file.
    const QString targetName = packageRoot + '/' + packageName;

    if (!QFile::exists(targetName)) {
        kWarning() << targetName << "does not exist";
        return false;
    }

    QString serviceName = d->servicePrefix + packageName;

    QString service = KStandardDirs::locateLocal("services", serviceName + ".desktop");
    kDebug() << "Removing service file " << service;
    bool ok = QFile::remove(service);

    if (!ok) {
        kWarning() << "Unable to remove " << service;
    }

#ifndef PLASMA_NO_KIO
    KIO::DeleteJob *job = KIO::del(KUrl(targetName));
    ok = job->exec();
    const QString errorString = job->errorString();
#else
    ok = removeFolder(targetName);
    const QString errorString("unknown");
#endif
    if (!ok) {
        kWarning() << "Could not delete package from:" << targetName << " : " << errorString;
        return false;
    }

    return true;
}

void PackageStructure::createNewWidgetBrowser(QWidget *parent)
{
    Q_UNUSED(parent)
    emit newWidgetBrowserFinished();
}

QString PackageStructure::defaultPackageRoot() const
{
    return d->packageRoot;
}

QString PackageStructure::servicePrefix() const
{
    return d->servicePrefix;
}

void PackageStructure::setDefaultPackageRoot(const QString &packageRoot)
{
    d->packageRoot = packageRoot;
}

void PackageStructure::setServicePrefix(const QString &servicePrefix)
{
    d->servicePrefix = servicePrefix;
}

void PackageStructurePrivate::createPackageMetadata(const QString &path)
{
    delete metadata;
    metadata = 0;

    QString metadataPath(path + "/metadata.desktop");
    if (!QFile::exists(metadataPath)) {
        kWarning() << "No metadata file in the package, expected it at:" << metadataPath;
        metadataPath.clear();
    }

    metadata = new PackageMetadata(metadataPath);
}

PackageMetadata PackageStructure::metadata() const
{
    if (!d->metadata && !d->path.isEmpty()) {
        QFileInfo fileInfo(d->path);

        if (fileInfo.isDir()) {
            d->createPackageMetadata(d->path);
        } else if (fileInfo.exists()) {
            KArchive *archive = 0;
            KMimeType::Ptr mimeType = KMimeType::findByPath(d->path);

            if (mimeType->is("application/zip")) {
                archive = new KZip(d->path);
            } else if (mimeType->is("application/x-compressed-tar") ||
                       mimeType->is("application/x-tar")|| mimeType->is("application/x-bzip-compressed-tar")) {
                archive = new KTar(d->path);
            } else {
                kWarning() << "Could not open package file, unsupported archive format:" << d->path << mimeType->name();
            }

            if (archive && archive->open(QIODevice::ReadOnly)) {
                const KArchiveDirectory *source = archive->directory();
                KTempDir tempdir;
                source->copyTo(tempdir.name());
                d->createPackageMetadata(tempdir.name());
            } else {
                kWarning() << "Could not open package file:" << d->path;
            }

            delete archive;
        }
    }

    if (!d->metadata) {
        d->metadata = new PackageMetadata();
    }

    return *d->metadata;
}

bool PackageStructure::allowExternalPaths() const
{
    return d->externalPaths;
}

void PackageStructure::setAllowExternalPaths(bool allow)
{
    d->externalPaths = allow;
}

} // Plasma namespace

#include "packagestructure.moc"

