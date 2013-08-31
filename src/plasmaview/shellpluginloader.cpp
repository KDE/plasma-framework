/*
 *   Copyright 2013 Marco Martin <mart@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "shellpluginloader.h"
#include "shellpackage_p.h"


class ShellPluginLoaderPrivate
{
public:
    ShellPluginLoaderPrivate();
    ~ShellPluginLoaderPrivate();
    Plasma::Package internalLoadPackage(const QString &packageFormat, const QString &specialization);
};

ShellPluginLoaderPrivate::ShellPluginLoaderPrivate()
{
}

Plasma::Package ShellPluginLoaderPrivate::internalLoadPackage(const QString &packageFormat, const QString &specialization)
{
    Q_UNUSED(specialization)

    if (packageFormat.endsWith("/Shell")) {
        Plasma::PackageStructure *structure = new ShellPackageStructure();
        return Plasma::Package(structure);
    } else {
        return Plasma::Package();
    }
}




/////////////////////ShellPluginLoader
ShellPluginLoader::ShellPluginLoader()
    : Plasma::PluginLoader(),
      d(new ShellPluginLoaderPrivate())
{
}

ShellPluginLoader::~ShellPluginLoader()
{
}

Plasma::Package ShellPluginLoader::internalLoadPackage(const QString &packageFormat, const QString &specialization)
{
    return d->internalLoadPackage(packageFormat, specialization);
}

