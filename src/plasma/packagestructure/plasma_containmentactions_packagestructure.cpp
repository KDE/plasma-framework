/*
    SPDX-FileCopyrightText: 2007-2009 Aaron Seigo <aseigo@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "plasma.h"

#include "config-plasma.h"
#include "packages_p.h"
#include <kpackage/package.h>
#include <kpackage/packagestructure.h>

class ContainmentActionsPackage : public Plasma::ChangeableMainScriptPackage
{
    Q_OBJECT
public:
    ContainmentActionsPackage(QObject *parent = nullptr, const QVariantList &args = QVariantList())
        : ChangeableMainScriptPackage(parent, args)
    {
    }

    void initPackage(KPackage::Package *package) override
    {
        ChangeableMainScriptPackage::initPackage(package);
        package->setDefaultPackageRoot(QStringLiteral(PLASMA_RELATIVE_DATA_INSTALL_DIR "/containmentactions/"));
    }
};

K_PLUGIN_CLASS_WITH_JSON(ContainmentActionsPackage, "plasma_containmentactions_packagestructure.json")

#include "plasma_containmentactions_packagestructure.moc"
