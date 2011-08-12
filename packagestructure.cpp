/******************************************************************************
*   Copyright 2011 by Aaron Seigo <aseigo@kde.org>                            *
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

#include "private/package_p.h"

namespace Plasma
{

PackageStructure::PackageStructure(QObject *parent, const QVariantList &args)
    : QObject(parent),
      d(0)
{
    Q_UNUSED(args)
}

PackageStructure::~PackageStructure()
{
}

void PackageStructure::initPackage(Package *package)
{
    Q_UNUSED(package)
}

void PackageStructure::pathChanged(Package *package)
{
    Q_UNUSED(package)
}

bool PackageStructure::installPackage(Package *package, const QString &archivePath, const QString &packageRoot)
{
    return PackagePrivate::installPackage(archivePath, packageRoot, package->servicePrefix());
}

bool PackageStructure::uninstallPackage(Package *package, const QString &packageName, const QString &packageRoot)
{
    return PackagePrivate::uninstallPackage(packageName, packageRoot, package->servicePrefix());
}

}

