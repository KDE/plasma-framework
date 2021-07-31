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

class DataEnginePackage : public Plasma::ChangeableMainScriptPackage
{
    Q_OBJECT
public:
    DataEnginePackage(QObject *parent = nullptr, const QVariantList &args = QVariantList())
        : ChangeableMainScriptPackage(parent, args)
    {
    }

    void initPackage(KPackage::Package *package) override
    {
        ChangeableMainScriptPackage::initPackage(package);
        package->setDefaultPackageRoot(QStringLiteral(PLASMA_RELATIVE_DATA_INSTALL_DIR "/dataengines/"));

        package->addDirectoryDefinition("data", QStringLiteral("data"), i18n("Data Files"));

        package->addDirectoryDefinition("scripts", QStringLiteral("code"), i18n("Executable Scripts"));
        QStringList mimetypes;
        mimetypes << QStringLiteral("text/plain");
        package->setMimeTypes("scripts", mimetypes);

        package->addDirectoryDefinition("services", QStringLiteral("services/"), i18n("Service Descriptions"));
        package->setMimeTypes("services", mimetypes);

        package->addDirectoryDefinition("translations", QStringLiteral("locale"), i18n("Translations"));
    }
};

K_PLUGIN_CLASS_WITH_JSON(DataEnginePackage, "plasma_dataengine_packagestructure.json")

#include "plasma_dataengine_packagestructure.moc"
