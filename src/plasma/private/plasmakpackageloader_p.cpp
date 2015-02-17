/*
 *   Copyright 2015 Marco Martin <mart@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
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

#include "plasmakpackageloader_p.h"

#include "packages_p.h"


namespace Plasma
{
KPackageLoader::KPackageLoader()
    : KPackage::PackageLoader()
{
}

KPackageLoader::~KPackageLoader()
{
}

KPackage::Package KPackageLoader::internalLoadPackage(const QString &packageFormat)
{
    KPackage::PackageStructure *structure = 0;

    if (packageFormat.startsWith("Plasma")) {
        if (packageFormat.endsWith("/Applet")) {
            structure = new PlasmoidPackage();
        } else if (packageFormat.endsWith("/DataEngine")) {
            structure = new DataEnginePackage();
        } else if (packageFormat.endsWith("/Theme")) {
            structure = new ThemePackage();
        } else if (packageFormat.endsWith("/ContainmentActions")) {
            structure = new ContainmentActionsPackage();
        } else if (packageFormat.endsWith("/Generic")) {
            structure = new GenericPackage();
        }

        if (structure) {
            return KPackage::Package(structure);
        }
    }

    return KPackage::Package();
}

}

