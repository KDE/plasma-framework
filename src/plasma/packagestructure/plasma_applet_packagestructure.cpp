/*
    SPDX-FileCopyrightText: 2007-2009 Aaron Seigo <aseigo@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "plasma.h"

#include "config-plasma.h"
#include "packages_p.h"
#include <KLocalizedString>
#include <kpackage/package.h>
#include <kpackage/packagestructure.h>

class PlasmoidPackage : public Plasma::GenericPackage
{
    Q_OBJECT
public:
    PlasmoidPackage(QObject *parent = nullptr, const QVariantList &args = QVariantList())
        : GenericPackage(parent, args)
    {
    }

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
        if (package->metadata().serviceTypes().contains(QLatin1String("Plasma/Containment"))) {
            package->addFileDefinition("compactapplet", QStringLiteral("applet/CompactApplet.qml"), i18n("Custom expander for compact applets"));
        } else {
            package->removeDefinition("compactapplet");
        }
    }
};

K_PLUGIN_CLASS_WITH_JSON(PlasmoidPackage, "plasma_applet_packagestructure.json")

#include "plasma_applet_packagestructure.moc"
