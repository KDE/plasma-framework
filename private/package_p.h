/*
 *   Copyright © 2009 Rob Scheepmaker <r.scheepmaker@student.utwente.nl>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License version 2 as
 *   published by the Free Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef PLASMA_PACKAGE_P_H
#define PLASMA_PACKAGE_P_H

#include "../plasma.h"
#include "../package.h"
#include "../service.h"

#include <QCryptographicHash>
#include <QDir>
#include <QString>

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

class PackagePrivate
{
public:
    PackagePrivate();
    PackagePrivate(const PackagePrivate &other);
    ~PackagePrivate();

    PackagePrivate &operator=(const PackagePrivate &rhs);

    void createPackageMetadata(const QString &path);
    void updateHash(const QString &basePath, const QString &subPath, const QDir &dir, QCryptographicHash &hash);
    static bool installPackage(const QString &archivePath, const QString &packageRoot, const QString &servicePrefix);
    static bool uninstallPackage(const QString &packageName, const QString &packageRoot, const QString &servicePrefix);

    QWeakPointer<PackageStructure> structure;
    QString type;
    QString path;
    QStringList contentsPrefixPaths;
    QString defaultPackageRoot;
    QString servicePrefix;
    QMap<QByteArray, ContentStructure> contents;
    QStringList mimeTypes;
    KPluginInfo *metadata;
    bool externalPaths : 1;
    bool valid : 1;
};

}

#endif
