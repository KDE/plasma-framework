/*
    SPDX-FileCopyrightText: 2007-2009 Aaron Seigo <aseigo@kde.org>
    SPDX-FileCopyrightText: 2013 Sebastian Kügler <sebas@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "packages.h"

#include <QDebug>

#include <KLocalizedString>
#include <kdeclarative/kdeclarative.h>

#include "../plasma/config-plasma.h"
#include <Plasma/Package>

void LookAndFeelPackage::initPackage(Plasma::Package *package)
{
    // https://community.kde.org/Plasma/lookAndFeelPackage
    package->setDefaultPackageRoot(QStringLiteral(PLASMA_RELATIVE_DATA_INSTALL_DIR "/look-and-feel/"));

    // Defaults
    package->addFileDefinition("defaults", QStringLiteral("defaults"), i18n("Default settings for theme, etc."));
    // Colors
    package->addFileDefinition("colors", QStringLiteral("colors"), i18n("Color scheme to use for applications."));

    // Directories
    package->addDirectoryDefinition("previews", QStringLiteral("previews"), i18n("Preview Images"));
    package->addFileDefinition("loginmanagerpreview", QStringLiteral("previews/loginmanager.png"), i18n("Preview for the Login Manager"));
    package->addFileDefinition("lockscreenpreview", QStringLiteral("previews/lockscreen.png"), i18n("Preview for the Lock Screen"));
    package->addFileDefinition("userswitcherpreview", QStringLiteral("previews/userswitcher.png"), i18n("Preview for the Userswitcher"));
    package->addFileDefinition("desktopswitcherpreview", QStringLiteral("previews/desktopswitcher.png"), i18n("Preview for the Virtual Desktop Switcher"));
    package->addFileDefinition("splashpreview", QStringLiteral("previews/splash.png"), i18n("Preview for Splash Screen"));
    package->addFileDefinition("runcommandpreview", QStringLiteral("previews/runcommand.png"), i18n("Preview for KRunner"));
    package->addFileDefinition("windowdecorationpreview", QStringLiteral("previews/windowdecoration.png"), i18n("Preview for the Window Decorations"));
    package->addFileDefinition("windowswitcherpreview", QStringLiteral("previews/windowswitcher.png"), i18n("Preview for Window Switcher"));

    package->addDirectoryDefinition("loginmanager", QStringLiteral("loginmanager"), i18n("Login Manager"));
    package->addFileDefinition("loginmanagermainscript", QStringLiteral("loginmanager/LoginManager.qml"), i18n("Main Script for Login Manager"));

    package->addDirectoryDefinition("logout", QStringLiteral("logout"), i18n("Logout Dialog"));
    package->addFileDefinition("logoutmainscript", QStringLiteral("logout/Logout.qml"), i18n("Main Script for Logout Dialog"));

    package->addDirectoryDefinition("lockscreen", QStringLiteral("lockscreen"), i18n("Screenlocker"));
    package->addFileDefinition("lockscreenmainscript", QStringLiteral("lockscreen/LockScreen.qml"), i18n("Main Script for Lock Screen"));

    package->addDirectoryDefinition("userswitcher", QStringLiteral("userswitcher"), i18n("UI for fast user switching"));
    package->addFileDefinition("userswitchermainscript", QStringLiteral("userswitcher/UserSwitcher.qml"), i18n("Main Script for User Switcher"));

    package->addDirectoryDefinition("desktopswitcher", QStringLiteral("desktopswitcher"), i18n("Virtual Desktop Switcher"));
    package->addFileDefinition("desktopswitchermainscript",
                               QStringLiteral("desktopswitcher/DesktopSwitcher.qml"),
                               i18n("Main Script for Virtual Desktop Switcher"));

    package->addDirectoryDefinition("osd", QStringLiteral("osd"), i18n("On-Screen Display Notifications"));
    package->addFileDefinition("osdmainscript", QStringLiteral("osd/Osd.qml"), i18n("Main Script for On-Screen Display Notifications"));

    package->addDirectoryDefinition("splash", QStringLiteral("splash"), i18n("Splash Screen"));
    package->addFileDefinition("splashmainscript", QStringLiteral("splash/Splash.qml"), i18n("Main Script for Splash Screen"));

    package->addDirectoryDefinition("runcommand", QStringLiteral("runcommand"), i18n("KRunner UI"));
    package->addFileDefinition("runcommandmainscript", QStringLiteral("runcommand/RunCommand.qml"), i18n("Main Script KRunner"));

    package->addDirectoryDefinition("windowdecoration", QStringLiteral("windowdecoration"), i18n("Window Decoration"));
    package->addFileDefinition("windowdecorationmainscript",
                               QStringLiteral("windowdecoration/WindowDecoration.qml"),
                               i18n("Main Script for Window Decoration"));

    package->addDirectoryDefinition("windowswitcher", QStringLiteral("windowswitcher"), i18n("Window Switcher"));
    package->addFileDefinition("windowswitchermainscript", QStringLiteral("windowswitcher/WindowSwitcher.qml"), i18n("Main Script for Window Switcher"));
}

void QmlWallpaperPackage::initPackage(Plasma::Package *package)
{
    package->addFileDefinition("mainscript", QStringLiteral("ui/main.qml"), i18n("Main Script File"));
    package->setRequired("mainscript", true);

    QStringList platform = KDeclarative::KDeclarative::runtimePlatform();
    if (!platform.isEmpty()) {
        QMutableStringListIterator it(platform);
        while (it.hasNext()) {
            it.next();
            it.setValue(QStringLiteral("platformcontents/") + it.value());
        }

        platform.append(QStringLiteral("contents"));
        package->setContentsPrefixPaths(platform);
    }

    package->setDefaultPackageRoot(QStringLiteral(PLASMA_RELATIVE_DATA_INSTALL_DIR "/wallpapers/"));

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

    package->addDirectoryDefinition("translations", QStringLiteral("locale"), i18n("Translations"));
}

void LayoutTemplatePackage::initPackage(Plasma::Package *package)
{
    package->setServicePrefix(QStringLiteral("plasma-layout-template"));
    package->setDefaultPackageRoot(QStringLiteral(PLASMA_RELATIVE_DATA_INSTALL_DIR "/layout-templates"));
    package->addFileDefinition("mainscript", QStringLiteral("layout.js"), i18n("Main Script File"));
    package->setRequired("mainscript", true);
}
