/******************************************************************************
*   Copyright 2007-2009 by Aaron Seigo <aseigo@kde.org>                       *
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

#include "plasma.h"

#include <kpackage/package.h>
#include <kpackage/packagestructure.h>
#include "packages_p.h"
#include "config-plasma.h"

class ContainmentActionsPackage : public Plasma::ChangeableMainScriptPackage
{
    Q_OBJECT
public:
    ContainmentActionsPackage(QObject *parent = nullptr, const QVariantList &args = QVariantList()) : ChangeableMainScriptPackage(parent, args) {}

    void initPackage(KPackage::Package *package) Q_DECL_OVERRIDE
    {
        ChangeableMainScriptPackage::initPackage(package);
        package->setDefaultPackageRoot(QStringLiteral(PLASMA_RELATIVE_DATA_INSTALL_DIR "/containmentactions/"));
    }
};


K_EXPORT_KPACKAGE_PACKAGE_WITH_JSON(ContainmentActionsPackage, "containmentactions-packagestructure.json")

#include "containmentactionspackage.moc"
