/*
    SPDX-FileCopyrightText: 2007-2009 Aaron Seigo <aseigo@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <float.h> // FLT_MAX
#include <math.h>

#include <QCoreApplication>
#include <QDebug>
#include <QFileInfo>

#include <KConfigGroup>
#include <KDesktopFile>
#include <KLocalizedString>
#include <KRuntimePlatform>

#include "config-plasma.h"
#include "packages_p.h"

#include <kpackage/package.h>

namespace Plasma
{
void ChangeableMainScriptPackage::initPackage(KPackage::Package *package)
{
    package->addFileDefinition("mainscript", QStringLiteral("ui/main.qml"));
    package->addFileDefinition("test", QStringLiteral("tests/test.qml"));
    package->setRequired("mainscript", true);
}

void ChangeableMainScriptPackage::pathChanged(KPackage::Package *package)
{
    if (package->path().isEmpty()) {
        return;
    }

    const QString mainScript = package->metadata().value(QLatin1String("X-Plasma-MainScript"));

    if (!mainScript.isEmpty()) {
        package->addFileDefinition("mainscript", mainScript);
    }
}

void GenericPackage::initPackage(KPackage::Package *package)
{
    ChangeableMainScriptPackage::initPackage(package);

    if (QStringList platform = KRuntimePlatform::runtimePlatform(); !platform.isEmpty()) {
        for (QString &platformEntry : platform) {
            platformEntry.prepend(QLatin1String("platformcontents/"));
        }
        platform.append(QStringLiteral("contents"));
        package->setContentsPrefixPaths(platform);
    }

    package->setDefaultPackageRoot(QStringLiteral(PLASMA_RELATIVE_DATA_INSTALL_DIR "/packages/"));

    package->addDirectoryDefinition("images", QStringLiteral("images"));
    package->addDirectoryDefinition("theme", QStringLiteral("theme"));
    const QStringList imageMimeTypes{QStringLiteral("image/svg+xml"), QStringLiteral("image/png"), QStringLiteral("image/jpeg")};
    package->setMimeTypes("images", imageMimeTypes);
    package->setMimeTypes("theme", imageMimeTypes);

    package->addDirectoryDefinition("config", QStringLiteral("config"));
    package->setMimeTypes("config", QStringList{QStringLiteral("text/xml")});

    package->addDirectoryDefinition("ui", QStringLiteral("ui"));

    package->addDirectoryDefinition("data", QStringLiteral("data"));

    package->addDirectoryDefinition("scripts", QStringLiteral("code"));
    package->setMimeTypes("scripts", QStringList{QStringLiteral("text/plain")});
    package->addFileDefinition("screenshot", QStringLiteral("screenshot.png"));

    package->addDirectoryDefinition("translations", QStringLiteral("locale"));
}

} // namespace Plasma

#include "moc_packages_p.cpp"
