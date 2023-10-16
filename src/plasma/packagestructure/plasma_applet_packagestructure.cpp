/*
    SPDX-FileCopyrightText: 2007-2009 Aaron Seigo <aseigo@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "plasma.h"

#include "config-plasma.h"
#include "packages_p.h"
#include <KLocalizedString>
#include <kcoreaddons_export.h>
#include <kpackage/package.h>
#include <kpackage/packagestructure.h>

class PlasmoidPackage : public Plasma::GenericPackage
{
    Q_OBJECT
public:
    using Plasma::GenericPackage::GenericPackage;

    void initPackage(KPackage::Package *package) override
    {
        GenericPackage::initPackage(package);
        package->setDefaultPackageRoot(QStringLiteral(PLASMA_RELATIVE_DATA_INSTALL_DIR "/plasmoids/"));

        package->addFileDefinition("configmodel", QStringLiteral("config/config.qml"));
        package->addFileDefinition("mainconfigxml", QStringLiteral("config/main.xml"));
    }

    void pathChanged(KPackage::Package *package) override
    {
        GenericPackage::pathChanged(package);
        const KPluginMetaData md = package->metadata();
        if (!md.isValid()) {
            return;
        }
        if (md.rawData().contains(QStringLiteral("X-Plasma-ContainmentType"))) {
            package->addFileDefinition("compactapplet", QStringLiteral("applet/CompactApplet.qml"));
        } else {
            package->removeDefinition("compactapplet");
        }
    }
};

K_PLUGIN_CLASS_WITH_JSON(PlasmoidPackage, "plasma_applet_packagestructure.json")

#include "plasma_applet_packagestructure.moc"
