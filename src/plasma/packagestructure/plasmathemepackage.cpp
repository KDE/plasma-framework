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

#include <KLocalizedString>
#include <kpackage/package.h>
#include <kpackage/packagestructure.h>
#include "config-plasma.h"

class ThemePackage : public KPackage::PackageStructure
{
    Q_OBJECT
public:
    ThemePackage(QObject *parent = 0, const QVariantList &args = QVariantList()) : KPackage::PackageStructure(parent, args) {}

    void initPackage(KPackage::Package *package) Q_DECL_OVERRIDE
    {
        // by default the packages have "contents/" as contentsPrefixPaths
        // but for the themes we don't want that, so unset it.
        package->setContentsPrefixPaths(QStringList());
        package->setDefaultPackageRoot(QStringLiteral(PLASMA_RELATIVE_DATA_INSTALL_DIR "/desktoptheme/"));

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
};

K_EXPORT_KPACKAGE_PACKAGE_WITH_JSON(ThemePackage, "plasmatheme-packagestructure.json")

#include "plasmathemepackage.moc"

