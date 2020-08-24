/*
    SPDX-FileCopyrightText: 2007-2009 Aaron Seigo <aseigo@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

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

    void initPackage(KPackage::Package *package) override
    {
        ChangeableMainScriptPackage::initPackage(package);
        package->setDefaultPackageRoot(QStringLiteral(PLASMA_RELATIVE_DATA_INSTALL_DIR "/containmentactions/"));
    }
};


K_EXPORT_KPACKAGE_PACKAGE_WITH_JSON(ContainmentActionsPackage, "containmentactions-packagestructure.json")

#include "containmentactionspackage.moc"
