/*
    SPDX-FileCopyrightText: 2007 Aaron Seigo <aseigo@kde.org>
    SPDX-FileCopyrightText: 2010 Marco Martin <notmart@gmail.com>
    SPDX-FileCopyrightText: 2010 Kevin Ottens <ervin@kde.org>
    SPDX-FileCopyrightText: 2009 Rob Scheepmaker

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "package.h"

#include <QTemporaryDir>

#include <KArchive>
#include <KDesktopFile>
#include <KJob>
#include <KTar>
#include <QDebug>
#include <kzip.h>

#include "config-plasma.h"

#include <QMimeDatabase>
#include <QStandardPaths>

#include "packagestructure.h"
#include "pluginloader.h"
#include "private/package_p.h"
#include "private/packagestructure_p.h"

namespace Plasma
{
PackagePrivate::PackagePrivate()
    : internalPackage(nullptr)
    , fallbackPackage(nullptr)
    , structure(nullptr)
{
}

PackagePrivate::~PackagePrivate()
{
}

Package::Package(PackageStructure *structure)
    : d(new Plasma::PackagePrivate())
{
    d->structure = structure;

    if (!structure) {
        d->internalPackage = new KPackage::Package();
        return;
    }

    if (!structure->d->internalStructure) {
        d->structure->d->internalStructure = new KPackage::PackageStructure;
    }

    d->internalPackage = new KPackage::Package(structure->d->internalStructure);
    PackageStructureWrapper::s_packagesMap[d->internalPackage] = this;
    structure->initPackage(this);
}

Package::Package(const Package &other)
    : d(new Plasma::PackagePrivate())
{
    d->internalPackage = new KPackage::Package(*other.d->internalPackage);
    d->structure = other.d->structure;
    PackageStructureWrapper::s_packagesMap[d->internalPackage] = this;
}

Package::Package(const KPackage::Package &other)
    : d(new Plasma::PackagePrivate())
{
    d->internalPackage = new KPackage::Package(other);
    PackageStructureWrapper::s_packagesMap[d->internalPackage] = this;
}

Package::~Package()
{
    PackageStructureWrapper::s_packagesMap.remove(d->internalPackage);
    delete d->internalPackage;
}

Package &Package::operator=(const Package &rhs)
{
    if (&rhs != this) {
        d->internalPackage = new KPackage::Package(*rhs.d->internalPackage);
        d->structure = rhs.d->structure;
        PackageStructureWrapper::s_packagesMap[d->internalPackage] = this;
    }

    return *this;
}

bool Package::hasValidStructure() const
{
    return d->internalPackage->hasValidStructure();
}

bool Package::isValid() const
{
    return d->internalPackage->isValid();
}

QString Package::name(const char *key) const
{
    return d->internalPackage->name(key);
}

bool Package::isRequired(const char *key) const
{
    return d->internalPackage->isRequired(key);
}

QStringList Package::mimeTypes(const char *key) const
{
    return d->internalPackage->mimeTypes(key);
}

QString Package::defaultPackageRoot() const
{
    return d->internalPackage->defaultPackageRoot();
}

void Package::setDefaultPackageRoot(const QString &packageRoot)
{
    d->internalPackage->setDefaultPackageRoot(packageRoot);
}

void Package::setFallbackPackage(const Plasma::Package &package)
{
    d->fallbackPackage = new Package(package);
    d->internalPackage->setFallbackPackage(*package.d->internalPackage);
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
    d->servicePrefix = servicePrefix;
}

KPackage::Package Package::kPackage() const
{
    return *d->internalPackage;
}

bool Package::allowExternalPaths() const
{
    return d->internalPackage->allowExternalPaths();
}

void Package::setAllowExternalPaths(bool allow)
{
    d->internalPackage->setAllowExternalPaths(allow);
}

KPluginInfo Package::metadata() const
{
    return KPluginInfo::fromMetaData(d->internalPackage->metadata());
}

QString Package::filePath(const char *fileType, const QString &filename) const
{
    return d->internalPackage->filePath(fileType, filename);
}

QStringList Package::entryList(const char *key) const
{
    return d->internalPackage->entryList(key);
}

void Package::setPath(const QString &path)
{
    if (path == d->internalPackage->path()) {
        return;
    }

    d->internalPackage->setPath(path);
}

const QString Package::path() const
{
    return d->internalPackage->path();
}

QStringList Package::contentsPrefixPaths() const
{
    return d->internalPackage->contentsPrefixPaths();
}

void Package::setContentsPrefixPaths(const QStringList &prefixPaths)
{
    d->internalPackage->setContentsPrefixPaths(prefixPaths);
}

QString Package::contentsHash() const
{
    return QString::fromLocal8Bit(d->internalPackage->cryptographicHash(QCryptographicHash::Sha1));
}

void Package::addDirectoryDefinition(const char *key, const QString &path, const QString &name)
{
    d->internalPackage->addDirectoryDefinition(key, path, name);
}

void Package::addFileDefinition(const char *key, const QString &path, const QString &name)
{
    d->internalPackage->addFileDefinition(key, path, name);
}

void Package::removeDefinition(const char *key)
{
    d->internalPackage->removeDefinition(key);
}

void Package::setRequired(const char *key, bool required)
{
    d->internalPackage->setRequired(key, required);
}

void Package::setDefaultMimeTypes(QStringList mimeTypes)
{
    d->internalPackage->setDefaultMimeTypes(mimeTypes);
}

void Package::setMimeTypes(const char *key, QStringList mimeTypes)
{
    d->internalPackage->setMimeTypes(key, mimeTypes);
}

QList<const char *> Package::directories() const
{
    QList<const char *> dirs;
    const auto directories = d->internalPackage->directories();
    for (const auto &data : directories) {
        dirs << data.constData();
    }

    return dirs;
}

QList<const char *> Package::requiredDirectories() const
{
    QList<const char *> dirs;
    const auto directories = d->internalPackage->requiredDirectories();
    for (const auto &data : directories) {
        dirs << data.constData();
    }

    return dirs;
}

QList<const char *> Package::files() const
{
    QList<const char *> files;
    const auto lstFiles = d->internalPackage->files();
    for (const auto &data : lstFiles) {
        files << data.constData();
    }

    return files;
}

QList<const char *> Package::requiredFiles() const
{
    QList<const char *> files;
    const auto lstFiles = d->internalPackage->requiredFiles();
    for (const auto &data : lstFiles) {
        files << data.constData();
    }

    return files;
}

KJob *Package::install(const QString &sourcePackage, const QString &packageRoot)
{
    const QString src = sourcePackage;
    const QString dest = packageRoot.isEmpty() ? defaultPackageRoot() : packageRoot;
    // qCDebug(LOG_PLASMA) << "Source: " << src;
    // qCDebug(LOG_PLASMA) << "PackageRoot: " << dest;
    KJob *j = d->structure->install(this, src, dest);
    return j;
}

KJob *Package::uninstall(const QString &packageName, const QString &packageRoot)
{
    setPath(packageRoot + packageName);

    return d->structure->uninstall(this, packageRoot);
}

} // Namespace
