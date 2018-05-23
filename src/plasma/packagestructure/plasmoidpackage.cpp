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
#include <KLocalizedString>
#include "packages_p.h"
#include "config-plasma.h"

class PlasmoidPackage : public Plasma::GenericPackage
{
    Q_OBJECT
public:
    PlasmoidPackage(QObject *parent = nullptr, const QVariantList &args = QVariantList()) : GenericPackage(parent, args) {}

    void initPackage(KPackage::Package *package) override
    {
        GenericPackage::initPackage(package);
        package->setDefaultPackageRoot(QStringLiteral(PLASMA_RELATIVE_DATA_INSTALL_DIR "/plasmoids/"));

        package->addFileDefinition("configmodel", QStringLiteral("config/config.qml"), i18n("Configuration UI pages model"));
        package->addFileDefinition("mainconfigxml", QStringLiteral("config/main.xml"), i18n("Configuration XML file"));
    }

    void pathChanged(KPackage::Package *package) override
    {
        ChangeableMainScriptPackage::pathChanged(package);

        if (!package->metadata().isValid()) {
            return;
        }
        if (package->metadata().serviceTypes().contains(QStringLiteral("Plasma/Containment"))) {
            package->addFileDefinition("compactapplet", QStringLiteral("applet/CompactApplet.qml"), i18n("Custom expander for compact applets"));
        } else {
            package->removeDefinition("compactapplet");
        }
    }
};

K_EXPORT_KPACKAGE_PACKAGE_WITH_JSON(PlasmoidPackage, "plasmoid-packagestructure.json")

#include "plasmoidpackage.moc"
