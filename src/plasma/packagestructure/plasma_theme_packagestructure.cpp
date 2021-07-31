/*
    SPDX-FileCopyrightText: 2007-2009 Aaron Seigo <aseigo@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "plasma.h"

#include "config-plasma.h"
#include <KLocalizedString>
#include <kpackage/package.h>
#include <kpackage/packagestructure.h>

class ThemePackage : public KPackage::PackageStructure
{
    Q_OBJECT
public:
    ThemePackage(QObject *parent = nullptr, const QVariantList &args = QVariantList())
        : KPackage::PackageStructure(parent, args)
    {
    }

    void initPackage(KPackage::Package *package) override
    {
        // by default the packages have "contents/" as contentsPrefixPaths
        // but for the themes we don't want that, so unset it.
        package->setContentsPrefixPaths(QStringList());
        package->setDefaultPackageRoot(QStringLiteral(PLASMA_RELATIVE_DATA_INSTALL_DIR "/desktoptheme/"));

        package->addDirectoryDefinition("dialogs", QStringLiteral("dialogs/"), i18n("Images for dialogs"));
        package->addFileDefinition("dialogs/background", QStringLiteral("dialogs/background.svg"), i18n("Generic dialog background"));
        package->addFileDefinition("dialogs/background", QStringLiteral("dialogs/background.svgz"), i18n("Generic dialog background"));
        package->addFileDefinition("dialogs/shutdowndialog", QStringLiteral("dialogs/shutdowndialog.svg"), i18n("Theme for the logout dialog"));
        package->addFileDefinition("dialogs/shutdowndialog", QStringLiteral("dialogs/shutdowndialog.svgz"), i18n("Theme for the logout dialog"));

        package->addDirectoryDefinition("wallpapers", QStringLiteral("wallpapers/"), i18n("Wallpaper packages"));

        package->addDirectoryDefinition("widgets", QStringLiteral("widgets/"), i18n("Images for widgets"));
        package->addFileDefinition("widgets/background", QStringLiteral("widgets/background.svg"), i18n("Background image for widgets"));
        package->addFileDefinition("widgets/background", QStringLiteral("widgets/background.svgz"), i18n("Background image for widgets"));
        package->addFileDefinition("widgets/clock", QStringLiteral("widgets/clock.svg"), i18n("Analog clock face"));
        package->addFileDefinition("widgets/clock", QStringLiteral("widgets/clock.svgz"), i18n("Analog clock face"));
        package->addFileDefinition("widgets/panel-background", QStringLiteral("widgets/panel-background.svg"), i18n("Background image for panels"));
        package->addFileDefinition("widgets/panel-background", QStringLiteral("widgets/panel-background.svgz"), i18n("Background image for panels"));
        package->addFileDefinition("widgets/plot-background", QStringLiteral("widgets/plot-background.svg"), i18n("Background for graphing widgets"));
        package->addFileDefinition("widgets/plot-background", QStringLiteral("widgets/plot-background.svgz"), i18n("Background for graphing widgets"));
        package->addFileDefinition("widgets/tooltip", QStringLiteral("widgets/tooltip.svg"), i18n("Background image for tooltips"));
        package->addFileDefinition("widgets/tooltip", QStringLiteral("widgets/tooltip.svgz"), i18n("Background image for tooltips"));

        package->addDirectoryDefinition("opaque/dialogs", QStringLiteral("opaque/dialogs/"), i18n("Opaque images for dialogs"));
        package->addFileDefinition("opaque/dialogs/background", QStringLiteral("opaque/dialogs/background.svg"), i18n("Opaque generic dialog background"));
        package->addFileDefinition("opaque/dialogs/background", QStringLiteral("opaque/dialogs/background.svgz"), i18n("Opaque generic dialog background"));
        package->addFileDefinition("opaque/dialogs/shutdowndialog",
                                   QStringLiteral("opaque/dialogs/shutdowndialog.svg"),
                                   i18n("Opaque theme for the logout dialog"));
        package->addFileDefinition("opaque/dialogs/shutdowndialog",
                                   QStringLiteral("opaque/dialogs/shutdowndialog.svgz"),
                                   i18n("Opaque theme for the logout dialog"));

        package->addDirectoryDefinition("opaque/widgets", QStringLiteral("opaque/widgets/"), i18n("Opaque images for widgets"));
        package->addFileDefinition("opaque/widgets/panel-background",
                                   QStringLiteral("opaque/widgets/panel-background.svg"),
                                   i18n("Opaque background image for panels"));
        package->addFileDefinition("opaque/widgets/panel-background",
                                   QStringLiteral("opaque/widgets/panel-background.svgz"),
                                   i18n("Opaque background image for panels"));
        package->addFileDefinition("opaque/widgets/tooltip", QStringLiteral("opaque/widgets/tooltip.svg"), i18n("Opaque background image for tooltips"));
        package->addFileDefinition("opaque/widgets/tooltip", QStringLiteral("opaque/widgets/tooltip.svgz"), i18n("Opaque background image for tooltips"));

        package->addFileDefinition("colors", QStringLiteral("colors"), i18n("KColorScheme configuration file"));

        QStringList mimetypes;
        mimetypes << QStringLiteral("image/svg+xml");
        package->setDefaultMimeTypes(mimetypes);
    }
};

K_PLUGIN_CLASS_WITH_JSON(ThemePackage, "plasma_theme_packagestructure.json")

#include "plasma_theme_packagestructure.moc"
