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
#include <QDebug>
#include "private/package_p.h"
#include "private/packagestructure_p.h"

#include <kpackage/packagetrader.h>
#include <kpackage/packagestructure.h>

#include <QVariantMap>

namespace Plasma
{

PackageStructure::PackageStructure(QObject *parent, const QVariantList &args)
    : QObject(parent),
      d(new PackageStructurePrivate)
{
    if (!args.isEmpty() && args.first().canConvert<QString>()) {
        d->internalStructure = KPackage::PackageTrader::self()->loadPackageStructure(args.first().toString());
    }

    Q_UNUSED(args)
}

PackageStructure::~PackageStructure()
{
    delete d;
}

void PackageStructure::initPackage(Package *package)
{
    if (d->internalStructure) {
        d->internalStructure->initPackage(package->d->internalPackage);
    }
}

void PackageStructure::pathChanged(Package *package)
{
   if (d->internalStructure) {
       d->internalStructure->pathChanged(package->d->internalPackage);
    }
}

KJob *PackageStructure::install(Package *package, const QString &archivePath, const QString &packageRoot)
{
    if (d->internalStructure) {
        return d->internalStructure->install(package->d->internalPackage, archivePath, packageRoot);
    }

    return 0;
}

KJob *PackageStructure::uninstall(Package *package, const QString &packageRoot)
{
    if (d->internalStructure) {
        return d->internalStructure->uninstall(package->d->internalPackage, packageRoot);
    }

    return 0;
}

}

#include "moc_packagestructure.cpp"
