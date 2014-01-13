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

#include <kconfiggroup.h>
#include <kdesktopfile.h>
#include <klocalizedstring.h>
#include <kmessagebox.h>

#include <kdeclarative/kdeclarative.h>
#include "package.h"
#include "config-plasma.h"

namespace Plasma
{

void ChangeableMainScriptPackage::initPackage(Package *package)
{
    package->addFileDefinition("mainscript", "ui/main.qml", i18n("Main Script File"));
    package->setRequired("mainscript", true);
}

QString ChangeableMainScriptPackage::mainScriptConfigKey() const
{
    return QLatin1String("X-Plasma-MainScript");
}

void ChangeableMainScriptPackage::pathChanged(Package *package)
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

void GenericPackage::initPackage(Package *package)
{
    ChangeableMainScriptPackage::initPackage(package);

    QStringList platform = KDeclarative::KDeclarative::runtimePlatform();
    if (!platform.isEmpty()) {
        QMutableStringListIterator it(platform);
        while (it.hasNext()) {
            it.next();
            it.setValue("platformcontents/" + it.value());
        }

        platform.append("contents");
        package->setContentsPrefixPaths(platform);
    }

    package->setDefaultPackageRoot("plasma/packages/");

    package->addDirectoryDefinition("images", "images", i18n("Images"));
    package->addDirectoryDefinition("theme", "theme", i18n("Themed Images"));
    QStringList mimetypes;
    mimetypes << "image/svg+xml" << "image/png" << "image/jpeg";
    package->setMimeTypes("images", mimetypes);
    package->setMimeTypes("theme", mimetypes);

    package->addDirectoryDefinition("config", "config", i18n("Configuration Definitions"));
    mimetypes.clear();
    mimetypes << "text/xml";
    package->setMimeTypes("config", mimetypes);

    package->addDirectoryDefinition("ui", "ui", i18n("User Interface"));

    package->addDirectoryDefinition("data", "data", i18n("Data Files"));

    package->addDirectoryDefinition("scripts", "code", i18n("Executable Scripts"));
    mimetypes.clear();
    mimetypes << "text/plain";
    package->setMimeTypes("scripts", mimetypes);

    package->addDirectoryDefinition("translations", "locale", i18n("Translations"));
}

void PlasmoidPackage::initPackage(Package *package)
{
    GenericPackage::initPackage(package);
    package->setServicePrefix("plasma-applet-");
    package->setDefaultPackageRoot("plasma/plasmoids/");

    package->addFileDefinition("configmodel", "config/config.qml", i18n("Configuration UI pages model"));
    package->addFileDefinition("mainconfigxml", "config/main.xml", i18n("Configuration XML file"));
}

QString ContainmentPackage::mainScriptConfigKey() const
{
    return QLatin1String("X-Plasma-Containment-MainScript");
}

void DataEnginePackage::initPackage(Package *package)
{
    ChangeableMainScriptPackage::initPackage(package);
    package->setServicePrefix("plasma-dataengine-");
    package->setDefaultPackageRoot("plasma/dataengines/");

    package->addDirectoryDefinition("data", "data", i18n("Data Files"));

    package->addDirectoryDefinition("scripts", "code", i18n("Executable Scripts"));
    QStringList mimetypes;
    mimetypes << "text/plain";
    package->setMimeTypes("scripts", mimetypes);

    package->addDirectoryDefinition("services", "services/", i18n("Service Descriptions"));
    package->setMimeTypes("services", mimetypes);

    package->addDirectoryDefinition("translations", "locale", i18n("Translations"));
}

void RunnerPackage::initPackage(Package *package)
{
    ChangeableMainScriptPackage::initPackage(package);
    package->setServicePrefix("plasma-runner-");
    package->setDefaultPackageRoot("plasma/runners/");

    package->addDirectoryDefinition("data", "data", i18n("Data Files"));

    package->addDirectoryDefinition("scripts", "code", i18n("Executable Scripts"));
    QStringList mimetypes;
    mimetypes << "text/plain";
    package->setMimeTypes("scripts", mimetypes);

    package->addDirectoryDefinition("translations", "locale", i18n("Translations"));
}

void ThemePackage::initPackage(Package *package)
{
    package->addDirectoryDefinition("dialogs", "dialogs/", i18n("Images for dialogs"));
    package->addFileDefinition("dialogs/background", "dialogs/background.svg",
                      i18n("Generic dialog background"));
    package->addFileDefinition("dialogs/background", "dialogs/background.svgz",
                      i18n("Generic dialog background"));
    package->addFileDefinition("dialogs/shutdowndialog", "dialogs/shutdowndialog.svg",
                      i18n("Theme for the logout dialog"));
    package->addFileDefinition("dialogs/shutdowndialog", "dialogs/shutdowndialog.svgz",
                      i18n("Theme for the logout dialog"));

    package->addDirectoryDefinition("wallpapers", "wallpapers/", i18n("Wallpaper packages"));

    package->addDirectoryDefinition("widgets", "widgets/", i18n("Images for widgets"));
    package->addFileDefinition("widgets/background", "widgets/background.svg",
                      i18n("Background image for widgets"));
    package->addFileDefinition("widgets/background", "widgets/background.svgz",
                      i18n("Background image for widgets"));
    package->addFileDefinition("widgets/clock", "widgets/clock.svg",
                      i18n("Analog clock face"));
    package->addFileDefinition("widgets/clock", "widgets/clock.svgz",
                      i18n("Analog clock face"));
    package->addFileDefinition("widgets/panel-background", "widgets/panel-background.svg",
                      i18n("Background image for panels"));
    package->addFileDefinition("widgets/panel-background", "widgets/panel-background.svgz",
                      i18n("Background image for panels"));
    package->addFileDefinition("widgets/plot-background", "widgets/plot-background.svg",
                      i18n("Background for graphing widgets"));
    package->addFileDefinition("widgets/plot-background", "widgets/plot-background.svgz",
                      i18n("Background for graphing widgets"));
    package->addFileDefinition("widgets/tooltip", "widgets/tooltip.svg",
                      i18n("Background image for tooltips"));
    package->addFileDefinition("widgets/tooltip", "widgets/tooltip.svgz",
                      i18n("Background image for tooltips"));

    package->addDirectoryDefinition("opaque/dialogs", "opaque/dialogs/", i18n("Opaque images for dialogs"));
    package->addFileDefinition("opaque/dialogs/background", "opaque/dialogs/background.svg",
                      i18n("Opaque generic dialog background"));
    package->addFileDefinition("opaque/dialogs/background", "opaque/dialogs/background.svgz",
                      i18n("Opaque generic dialog background"));
    package->addFileDefinition("opaque/dialogs/shutdowndialog", "opaque/dialogs/shutdowndialog.svg",
                      i18n("Opaque theme for the logout dialog"));
    package->addFileDefinition("opaque/dialogs/shutdowndialog", "opaque/dialogs/shutdowndialog.svgz",
                      i18n("Opaque theme for the logout dialog"));

    package->addDirectoryDefinition("opaque/widgets", "opaque/widgets/", i18n("Opaque images for widgets"));
    package->addFileDefinition("opaque/widgets/panel-background", "opaque/widgets/panel-background.svg",
                      i18n("Opaque background image for panels"));
    package->addFileDefinition("opaque/widgets/panel-background", "opaque/widgets/panel-background.svgz",
                      i18n("Opaque background image for panels"));
    package->addFileDefinition("opaque/widgets/tooltip", "opaque/widgets/tooltip.svg",
                      i18n("Opaque background image for tooltips"));
    package->addFileDefinition("opaque/widgets/tooltip", "opaque/widgets/tooltip.svgz",
                      i18n("Opaque background image for tooltips"));

    package->addDirectoryDefinition("locolor/dialogs", "locolor/dialogs/",
                           i18n("Low color images for dialogs"));
    package->addFileDefinition("locolor/dialogs/background", "locolor/dialogs/background.svg",
                      i18n("Low color generic dialog background"));
    package->addFileDefinition("locolor/dialogs/background", "locolor/dialogs/background.svgz",
                      i18n("Low color generic dialog background"));
    package->addFileDefinition("locolor/dialogs/shutdowndialog", "locolor/dialogs/shutdowndialog.svg",
                      i18n("Low color theme for the logout dialog"));
    package->addFileDefinition("locolor/dialogs/shutdowndialog", "locolor/dialogs/shutdowndialog.svgz",
                      i18n("Low color theme for the logout dialog"));

    package->addDirectoryDefinition("locolor/widgets", "locolor/widgets/", i18n("Images for widgets"));
    package->addFileDefinition("locolor/widgets/background", "locolor/widgets/background.svg",
                      i18n("Low color background image for widgets"));
    package->addFileDefinition("locolor/widgets/background", "locolor/widgets/background.svgz",
                      i18n("Low color background image for widgets"));
    package->addFileDefinition("locolor/widgets/clock", "locolor/widgets/clock.svg",
                      i18n("Low color analog clock face"));
    package->addFileDefinition("locolor/widgets/clock", "locolor/widgets/clock.svgz",
                      i18n("Low color analog clock face"));
    package->addFileDefinition("locolor/widgets/panel-background", "locolor/widgets/panel-background.svg",
                      i18n("Low color background image for panels"));
    package->addFileDefinition("locolor/widgets/panel-background", "locolor/widgets/panel-background.svgz",
                      i18n("Low color background image for panels"));
    package->addFileDefinition("locolor/widgets/plot-background", "locolor/widgets/plot-background.svg",
                      i18n("Low color background for graphing widgets"));
    package->addFileDefinition("locolor/widgets/plot-background", "locolor/widgets/plot-background.svgz",
                      i18n("Low color background for graphing widgets"));
    package->addFileDefinition("locolor/widgets/tooltip", "locolor/widgets/tooltip.svg",
                      i18n("Low color background image for tooltips"));
    package->addFileDefinition("locolor/widgets/tooltip", "locolor/widgets/tooltip.svgz",
                      i18n("Low color background image for tooltips"));

    package->addFileDefinition("colors", "colors", i18n("KColorScheme configuration file"));

    QStringList mimetypes;
    mimetypes << "image/svg+xml";
    package->setDefaultMimeTypes(mimetypes);
}

void ContainmentActionsPackage::initPackage(Package *package)
{
    ChangeableMainScriptPackage::initPackage(package);
    package->setDefaultPackageRoot("plasma/containmentactions/");
}

void ShellPackage::initPackage(Plasma::Package *package)
{
    package->setDefaultPackageRoot("plasma/shells/");

    //Directories
    package->addDirectoryDefinition("applet", "applet", i18n("Applets furniture"));
    package->addDirectoryDefinition("configuration", "configuration", i18n("Applets furniture"));
    package->addDirectoryDefinition("explorer", "explorer", i18n("Explorer UI for adding widgets"));
    package->addDirectoryDefinition("views", "views", i18n("User interface for the views that will show containments"));

    package->setMimeTypes("applet", QStringList() << "text/x-qml");
    package->setMimeTypes("configuration", QStringList() << "text/x-qml");
    package->setMimeTypes("views", QStringList() << "text/x-qml");

    //Files
    //Default layout
    package->addFileDefinition("defaultlayout", "layout.js", i18n("Default layout file"));
    package->addFileDefinition("defaults", "defaults", i18n("Default plugins for containments, containmentActions etc"));
    package->setMimeTypes("defaultlayout", QStringList() << "application/javascript");
    package->setMimeTypes("defaults", QStringList() << "text/plain");

    //Applet furniture
    package->addFileDefinition("appleterror", "applet/AppletError.qml", i18n("Error message shown when an applet fails to load"));
    package->addFileDefinition("compactapplet", "applet/CompactApplet.qml", i18n("QML component that shows an applet in a popup"));
    package->addFileDefinition("defaultcompactrepresentation", "applet/DefaultCompactRepresentation.qml", i18n("Compact representation of an applet when collapsed in a popup, for instance as an icon. applets can override this component."));

    //Configuration
    package->addFileDefinition("appletconfigurationui", "configuration/AppletConfiguration.qml", i18n("QML component for the configuration dialog for applets"));
    package->addFileDefinition("containmentconfigurationui", "configuration/ContainmentConfiguration.qml", i18n("QML component for the configuration dialog for containments"));
    package->addFileDefinition("panelconfigurationui", "configuration/PanelConfiguration.qml", i18n("Panel configuration UI"));

    //Widget explorer
    package->addFileDefinition("widgetexplorer", "explorer/WidgetExplorer.qml", i18n("Widgets explorer UI"));


    //package->setRequired("mainscript", true);
}

} // namespace Plasma

