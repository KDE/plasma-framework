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

#include "private/packages_p.h"

#include <math.h>
#include <float.h> // FLT_MAX

#include <QCoreApplication>
#include <QFileInfo>
#include <QDebug>

#include <kconfiggroup.h>
#include <kdesktopfile.h>
#include <klocalizedstring.h>
#include <kmessagebox.h>

#include <kdeclarative/kdeclarative.h>
#include "package.h"
#include "config-plasma.h"

#include <kpackage/package.h>

namespace Plasma
{

void ChangeableMainScriptPackage::initPackage(KPackage::Package *package)
{
    package->addFileDefinition("mainscript", QStringLiteral("ui/main.qml"), i18n("Main Script File"));
    package->setRequired("mainscript", true);
}

QString ChangeableMainScriptPackage::mainScriptConfigKey() const
{
    return QStringLiteral("X-Plasma-MainScript");
}

void ChangeableMainScriptPackage::pathChanged(KPackage::Package *package)
{
    if (package->path().isEmpty()) {
        return;
    }

    KDesktopFile config(package->path() + "/metadata.desktop");
    KConfigGroup cg = config.desktopGroup();
    QString mainScript = cg.readEntry(mainScriptConfigKey(), QString());

    if (!mainScript.isEmpty()) {
        package->addFileDefinition("mainscript", mainScript, i18n("Main Script File"));
    }
}

void GenericPackage::initPackage(KPackage::Package *package)
{
    ChangeableMainScriptPackage::initPackage(package);

    QStringList platform = KDeclarative::KDeclarative::runtimePlatform();
    if (!platform.isEmpty()) {
        QMutableStringListIterator it(platform);
        while (it.hasNext()) {
            it.next();
            it.setValue("platformcontents/" + it.value());
        }

        platform.append(QStringLiteral("contents"));
        package->setContentsPrefixPaths(platform);
    }

    package->setDefaultPackageRoot(QStringLiteral(PLASMA_RELATIVE_DATA_INSTALL_DIR "/packages/"));

    package->addDirectoryDefinition("images", QStringLiteral("images"), i18n("Images"));
    package->addDirectoryDefinition("theme", QStringLiteral("theme"), i18n("Themed Images"));
    QStringList mimetypes;
    mimetypes << QStringLiteral("image/svg+xml") << QStringLiteral("image/png") << QStringLiteral("image/jpeg");
    package->setMimeTypes("images", mimetypes);
    package->setMimeTypes("theme", mimetypes);

    package->addDirectoryDefinition("config", QStringLiteral("config"), i18n("Configuration Definitions"));
    mimetypes.clear();
    mimetypes << QStringLiteral("text/xml");
    package->setMimeTypes("config", mimetypes);

    package->addDirectoryDefinition("ui", QStringLiteral("ui"), i18n("User Interface"));

    package->addDirectoryDefinition("data", QStringLiteral("data"), i18n("Data Files"));

    package->addDirectoryDefinition("scripts", QStringLiteral("code"), i18n("Executable Scripts"));
    mimetypes.clear();
    mimetypes << QStringLiteral("text/plain");
    package->setMimeTypes("scripts", mimetypes);
    package->addFileDefinition("screenshot", QStringLiteral("screenshot.png"), i18n("Screenshot"));

    package->addDirectoryDefinition("translations", QStringLiteral("locale"), i18n("Translations"));
}

void PlasmoidPackage::initPackage(KPackage::Package *package)
{
    GenericPackage::initPackage(package);
    package->setDefaultPackageRoot(QStringLiteral(PLASMA_RELATIVE_DATA_INSTALL_DIR "/plasmoids/"));

    package->addFileDefinition("configmodel", QStringLiteral("config/config.qml"), i18n("Configuration UI pages model"));
    package->addFileDefinition("mainconfigxml", QStringLiteral("config/main.xml"), i18n("Configuration XML file"));
}

void PlasmoidPackage::pathChanged(KPackage::Package *package)
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

void DataEnginePackage::initPackage(KPackage::Package *package)
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

void ThemePackage::initPackage(KPackage::Package *package)
{
    // by default the packages have "contents/" as contentsPrefixPaths
    // but for the themes we don't want that, so unset it.
    package->setContentsPrefixPaths(QStringList());

    package->addDirectoryDefinition("dialogs", QStringLiteral("dialogs/"), i18n("Images for dialogs"));
    package->addFileDefinition("dialogs/background", QStringLiteral("dialogs/background.svg"),
                               i18n("Generic dialog background"));
    package->addFileDefinition("dialogs/background", QStringLiteral("dialogs/background.svgz"),
                               i18n("Generic dialog background"));
    package->addFileDefinition("dialogs/shutdowndialog", QStringLiteral("dialogs/shutdowndialog.svg"),
                               i18n("Theme for the logout dialog"));
    package->addFileDefinition("dialogs/shutdowndialog", QStringLiteral("dialogs/shutdowndialog.svgz"),
                               i18n("Theme for the logout dialog"));

    package->addDirectoryDefinition("wallpapers", QStringLiteral("wallpapers/"), i18n("Wallpaper packages"));

    package->addDirectoryDefinition("widgets", QStringLiteral("widgets/"), i18n("Images for widgets"));
    package->addFileDefinition("widgets/background", QStringLiteral("widgets/background.svg"),
                               i18n("Background image for widgets"));
    package->addFileDefinition("widgets/background", QStringLiteral("widgets/background.svgz"),
                               i18n("Background image for widgets"));
    package->addFileDefinition("widgets/clock", QStringLiteral("widgets/clock.svg"),
                               i18n("Analog clock face"));
    package->addFileDefinition("widgets/clock", QStringLiteral("widgets/clock.svgz"),
                               i18n("Analog clock face"));
    package->addFileDefinition("widgets/panel-background", QStringLiteral("widgets/panel-background.svg"),
                               i18n("Background image for panels"));
    package->addFileDefinition("widgets/panel-background", QStringLiteral("widgets/panel-background.svgz"),
                               i18n("Background image for panels"));
    package->addFileDefinition("widgets/plot-background", QStringLiteral("widgets/plot-background.svg"),
                               i18n("Background for graphing widgets"));
    package->addFileDefinition("widgets/plot-background", QStringLiteral("widgets/plot-background.svgz"),
                               i18n("Background for graphing widgets"));
    package->addFileDefinition("widgets/tooltip", QStringLiteral("widgets/tooltip.svg"),
                               i18n("Background image for tooltips"));
    package->addFileDefinition("widgets/tooltip", QStringLiteral("widgets/tooltip.svgz"),
                               i18n("Background image for tooltips"));

    package->addDirectoryDefinition("opaque/dialogs", QStringLiteral("opaque/dialogs/"), i18n("Opaque images for dialogs"));
    package->addFileDefinition("opaque/dialogs/background", QStringLiteral("opaque/dialogs/background.svg"),
                               i18n("Opaque generic dialog background"));
    package->addFileDefinition("opaque/dialogs/background", QStringLiteral("opaque/dialogs/background.svgz"),
                               i18n("Opaque generic dialog background"));
    package->addFileDefinition("opaque/dialogs/shutdowndialog", QStringLiteral("opaque/dialogs/shutdowndialog.svg"),
                               i18n("Opaque theme for the logout dialog"));
    package->addFileDefinition("opaque/dialogs/shutdowndialog", QStringLiteral("opaque/dialogs/shutdowndialog.svgz"),
                               i18n("Opaque theme for the logout dialog"));

    package->addDirectoryDefinition("opaque/widgets", QStringLiteral("opaque/widgets/"), i18n("Opaque images for widgets"));
    package->addFileDefinition("opaque/widgets/panel-background", QStringLiteral("opaque/widgets/panel-background.svg"),
                               i18n("Opaque background image for panels"));
    package->addFileDefinition("opaque/widgets/panel-background", QStringLiteral("opaque/widgets/panel-background.svgz"),
                               i18n("Opaque background image for panels"));
    package->addFileDefinition("opaque/widgets/tooltip", QStringLiteral("opaque/widgets/tooltip.svg"),
                               i18n("Opaque background image for tooltips"));
    package->addFileDefinition("opaque/widgets/tooltip", QStringLiteral("opaque/widgets/tooltip.svgz"),
                               i18n("Opaque background image for tooltips"));

    package->addDirectoryDefinition("locolor/dialogs", QStringLiteral("locolor/dialogs/"),
                                    i18n("Low color images for dialogs"));
    package->addFileDefinition("locolor/dialogs/background", QStringLiteral("locolor/dialogs/background.svg"),
                               i18n("Low color generic dialog background"));
    package->addFileDefinition("locolor/dialogs/background", QStringLiteral("locolor/dialogs/background.svgz"),
                               i18n("Low color generic dialog background"));
    package->addFileDefinition("locolor/dialogs/shutdowndialog", QStringLiteral("locolor/dialogs/shutdowndialog.svg"),
                               i18n("Low color theme for the logout dialog"));
    package->addFileDefinition("locolor/dialogs/shutdowndialog", QStringLiteral("locolor/dialogs/shutdowndialog.svgz"),
                               i18n("Low color theme for the logout dialog"));

    package->addDirectoryDefinition("locolor/widgets", QStringLiteral("locolor/widgets/"), i18n("Images for widgets"));
    package->addFileDefinition("locolor/widgets/background", QStringLiteral("locolor/widgets/background.svg"),
                               i18n("Low color background image for widgets"));
    package->addFileDefinition("locolor/widgets/background", QStringLiteral("locolor/widgets/background.svgz"),
                               i18n("Low color background image for widgets"));
    package->addFileDefinition("locolor/widgets/clock", QStringLiteral("locolor/widgets/clock.svg"),
                               i18n("Low color analog clock face"));
    package->addFileDefinition("locolor/widgets/clock", QStringLiteral("locolor/widgets/clock.svgz"),
                               i18n("Low color analog clock face"));
    package->addFileDefinition("locolor/widgets/panel-background", QStringLiteral("locolor/widgets/panel-background.svg"),
                               i18n("Low color background image for panels"));
    package->addFileDefinition("locolor/widgets/panel-background", QStringLiteral("locolor/widgets/panel-background.svgz"),
                               i18n("Low color background image for panels"));
    package->addFileDefinition("locolor/widgets/plot-background", QStringLiteral("locolor/widgets/plot-background.svg"),
                               i18n("Low color background for graphing widgets"));
    package->addFileDefinition("locolor/widgets/plot-background", QStringLiteral("locolor/widgets/plot-background.svgz"),
                               i18n("Low color background for graphing widgets"));
    package->addFileDefinition("locolor/widgets/tooltip", QStringLiteral("locolor/widgets/tooltip.svg"),
                               i18n("Low color background image for tooltips"));
    package->addFileDefinition("locolor/widgets/tooltip", QStringLiteral("locolor/widgets/tooltip.svgz"),
                               i18n("Low color background image for tooltips"));

    package->addFileDefinition("colors", QStringLiteral("colors"), i18n("KColorScheme configuration file"));

    QStringList mimetypes;
    mimetypes << QStringLiteral("image/svg+xml");
    package->setDefaultMimeTypes(mimetypes);
}

void ContainmentActionsPackage::initPackage(KPackage::Package *package)
{
    ChangeableMainScriptPackage::initPackage(package);
    package->setDefaultPackageRoot(QStringLiteral(PLASMA_RELATIVE_DATA_INSTALL_DIR "/containmentactions/"));
}

} // namespace Plasma

