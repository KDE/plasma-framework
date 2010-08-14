/*
 * Copyright 2008 Aaron Seigo <aseigo@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as
 *   published by the Free Software Foundation; either version 2,
 *   or (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include <iostream>

#include <QDir>
#include <QDBusInterface>

#include <KApplication>
#include <KAboutData>
#include <KCmdLineArgs>
#include <KLocale>
#include <KPluginInfo>
#include <KService>
#include <KServiceTypeTrader>
#include <KShell>
#include <KStandardDirs>

#include <Plasma/PackageStructure>
#include <Plasma/Package>
#include <Plasma/PackageMetadata>

static const char description[] = I18N_NOOP("Install, list, remove Plasma packages");
static const char version[] = "0.1";

void output(const QString &msg)
{
    std::cout << msg.toLocal8Bit().constData() << std::endl;
}

void runKbuildsycoca()
{
    QDBusInterface dbus("org.kde.kded", "/kbuildsycoca", "org.kde.kbuildsycoca");
    dbus.call(QDBus::NoBlock, "recreate");
}

QStringList packages(const QStringList& types)
{
    QStringList result;

    foreach (const QString& type, types) {
        const KService::List services = KServiceTypeTrader::self()->query(type);
        foreach (const KService::Ptr &service, services) {
            result << service->property("X-KDE-PluginInfo-Name", QVariant::String).toString();
        }
    }

    return result;
}

void listPackages(const QStringList& types)
{
    QStringList list = packages(types);
    list.sort();
    foreach (const QString& package, list) {
        output(package);
    }
}

void listTypes()
{
    output(i18n("Package types that are installable with this tool:"));
    output(i18n("Built in:"));
    output(i18n("    dataengine: Plasma DataEngine plugin"));
    output(i18n("    layout-template: Plasma containment and widget layout script"));
    output(i18n("    plasmoid: Plasma widget"));
    output(i18n("    runner: Search plugin (KRunner, etc)"));
    output(i18n("    theme: Plasma SVG theme"));
    output(i18n("    wallpaper: Image pack for use with wallpaper backgrounds"));
    output(i18n("    wallpaperplugin: Wallpaper plugin"));

    KService::List offers = KServiceTypeTrader::self()->query("Plasma/PackageStructure");
    if (!offers.isEmpty()) {
        std::cout << std::endl;
        output(i18n("Provided by plugins:"));
        foreach (const KService::Ptr service, offers) {
            KPluginInfo info(service);
            output(i18nc("Plugin name and the kind of Plasma related content it provides, both from the plugin's desktop file",
                        "    %1: %2", info.pluginName(), info.name()));
        }
    }

    QStringList desktopFiles = KGlobal::dirs()->findAllResources("data", "plasma/packageformats/*rc", KStandardDirs::NoDuplicates);
    if (!desktopFiles.isEmpty()) {
        output(i18n("Provided by .desktop files:"));
        Plasma::PackageStructure structure;
        foreach (const QString &file, desktopFiles) {
            // extract the type
            KConfig config(file, KConfig::SimpleConfig);
            structure.read(&config);
            // get the name based on the rc file name, just as Plasma::PackageStructure does
            const QString name = file.left(file.length() - 2);
            output(i18nc("Plugin name and the kind of Plasma related content it provides, both from the plugin's desktop file",
                        "    %1: %2", name, structure.type()));
        }
    }
}

int main(int argc, char **argv)
{
    KAboutData aboutData("plasmapkg", 0, ki18n("Plasma Package Manager"),
                         version, ki18n(description), KAboutData::License_GPL,
                         ki18n("(C) 2008, Aaron Seigo"));
    aboutData.addAuthor( ki18n("Aaron Seigo"),
                         ki18n("Original author"),
                        "aseigo@kde.org" );

    KComponentData componentData(aboutData);

    KCmdLineArgs::init( argc, argv, &aboutData );

    KCmdLineOptions options;
    options.add("g");
    options.add("global", ki18n("For install or remove, operates on packages installed for all users."));
    options.add("t");
    options.add("type <type>",
                ki18nc("theme, wallpaper, etc. are keywords, but they may be translated, as both versions "
                       "are recognized by the application "
                       "(if translated, should be same as messages with 'package type' context below)",
                       "The type of package, e.g. theme, wallpaper, plasmoid, dataengine, runner, layout-template, etc."),
                "plasmoid");
    options.add("s");
    options.add("i");
    options.add("install <path>", ki18nc("Do not translate <path>", "Install the package at <path>"));
    options.add("u");
    options.add("upgrade <path>", ki18nc("Do not translate <path>", "Upgrade the package at <path>"));
    options.add("l");
    options.add("list", ki18n("List installed packages"));
    options.add("list-types", ki18n("lists all known Package types that can be installed"));
    options.add("r");
    options.add("remove <name>", ki18nc("Do not translate <name>", "Remove the package named <name>"));
    options.add("p");
    options.add("packageroot <path>", ki18n("Absolute path to the package root. If not supplied, then the standard data directories for this KDE session will be searched instead."));
    KCmdLineArgs::addCmdLineOptions( options );

    KApplication app;

    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
    QString type = args->getOption("type").toLower();
    QString packageRoot = type;
    QString servicePrefix;
    QStringList pluginTypes;
    Plasma::PackageStructure *installer = 0;
    QString package;
    QString packageFile;

    if (args->isSet("list-types")) {
        listTypes();
        exit(0);
    }

    if (args->isSet("remove")) {
        package = args->getOption("remove");
    } else if (args->isSet("upgrade")) {
        package = args->getOption("upgrade");
    } else if (args->isSet("install")) {
        package = args->getOption("install");
    }
    if (!QDir::isAbsolutePath(package)) {
        packageFile = QDir(QDir::currentPath() + '/' + package).absolutePath();
    } else {
        packageFile = package;
    }

    if (!packageFile.isEmpty() && (!args->isSet("type") ||
        type == i18nc("package type", "wallpaper").toLower() || type == "wallpaper")) {
        // Check type for common plasma packages
        Plasma::PackageStructure package;
        package.setPath(packageFile);
        QString serviceType = package.metadata().serviceType();
        if (!serviceType.isEmpty()) {
            if (    serviceType == "Plasma/Applet" 
                    || serviceType == "Plasma/Containment"
                    || serviceType == "Plasma/PopupApplet" ) 
            {
                type = "plasmoid";
            } else if (serviceType == "Plasma/DataEngine") {
                type = "dataengine";
            } else if (serviceType == "Plasma/Runner") {
                type = "runner";
            } else if (serviceType == "Plasma/Wallpaper") {
                // This also changes type to wallpaperplugin when --type wallpaper
                // was specified and we have wallpaper plugin package (instead of
                // wallpaper image package)
                type = "wallpaperplugin";
            }
        }
    }

    if (type == i18nc("package type", "plasmoid").toLower() || type == "plasmoid") {
        packageRoot = "plasma/plasmoids/";
        servicePrefix = "plasma-applet-";
        pluginTypes << "Plasma/Applet";
        pluginTypes << "Plasma/PopupApplet";
        pluginTypes << "Plasma/Containment";
    } else if (type == i18nc("package type", "theme").toLower() || type == "theme") {
        packageRoot = "desktoptheme/";
    } else if (type == i18nc("package type", "wallpaper").toLower() || type == "wallpaper") {
        packageRoot = "wallpapers/";
    } else if (type == i18nc("package type", "dataengine").toLower() || type == "dataengine") {
        packageRoot = "plasma/dataengines/";
        servicePrefix = "plasma-dataengine-";
        pluginTypes << "Plasma/DataEngine";
    } else if (type == i18nc("package type", "runner").toLower() || type == "runner") {
        packageRoot = "plasma/runners/";
        servicePrefix = "plasma-runner-";
        pluginTypes << "Plasma/Runner";
    } else if (type == i18nc("package type", "wallpaperplugin").toLower() || type == "wallpaperplugin") {
        packageRoot = "plasma/wallpapers/";
        servicePrefix = "plasma-wallpaper-";
        pluginTypes << "Plasma/Wallpaper";
    } else if (type == i18nc("package type", "layout-template").toLower() || type == "layout-template") {
        packageRoot = "plasma/layout-templates/";
        servicePrefix = "plasma-layout-";
        pluginTypes << "Plasma/LayoutTemplate";
    } else {
        const QString constraint = QString("[X-KDE-PluginInfo-Name] == '%1'").arg(args->getOption("type"));
        KService::List offers = KServiceTypeTrader::self()->query("Plasma/PackageStructure", constraint);
        if (offers.isEmpty()) {
            output(i18n("Could not find a suitable installer for package of type %1", args->getOption("type")));
            return 1;
        }

        KService::Ptr offer = offers.first();
        QString error;
        installer = offer->createInstance<Plasma::PackageStructure>(0, QVariantList(), &error);

        if (!installer) {
            output(i18n("Could not load installer for package of type %1. Error reported was: %2",
                        args->getOption("type"), error));
            return 1;
        }

        packageRoot = installer->defaultPackageRoot();
        pluginTypes << installer->type();
    }

    if (args->isSet("list")) {
        listPackages(pluginTypes);
    } else {
        // install, remove or upgrade
        if (!installer) {
            installer = new Plasma::PackageStructure();
            installer->setServicePrefix(servicePrefix);
        }

        if (args->isSet("packageroot") && args->isSet("global")) {
            KCmdLineArgs::usageError(i18nc("The user entered conflicting options packageroot and global, this is the error message telling the user he can use only one", "The packageroot and global options conflict each other, please select only one."));
        } else if (args->isSet("packageroot")) {
            packageRoot = args->getOption("packageroot");
        } else if (args->isSet("global")) {
            packageRoot = KStandardDirs::locate("data", packageRoot);
        } else {
            packageRoot = KStandardDirs::locateLocal("data", packageRoot);
        }

        if (args->isSet("remove") || args->isSet("upgrade")) {
            installer->setPath(packageFile);
            Plasma::PackageMetadata metadata = installer->metadata();

            QString pluginName;
            if (metadata.pluginName().isEmpty()) {
                // plugin name given in command line
                pluginName = package;
            } else {
                // Parameter was a plasma package, get plugin name from the package
                pluginName = metadata.pluginName();
            }

            QStringList installed = packages(pluginTypes);
            if (installed.contains(pluginName)) {
                if (installer->uninstallPackage(pluginName, packageRoot)) {
                    output(i18n("Successfully removed %1", pluginName));
                } else if (!args->isSet("upgrade")) {
                    output(i18n("Removal of %1 failed.", pluginName));
                    delete installer;
                    return 1;
                }
            } else {
                output(i18n("Plugin %1 is not installed.", pluginName));
            }
        }
        if (args->isSet("install") || args->isSet("upgrade")) {
            if (installer->installPackage(packageFile, packageRoot)) {
                output(i18n("Successfully installed %1", packageFile));
            } else {
                output(i18n("Installation of %1 failed.", packageFile));
                delete installer;
                return 1;
            }
        }
        if (package.isEmpty()) {
            KCmdLineArgs::usageError(i18nc("No option was given, this is the error message telling the user he needs at least one, do not translate install, remove, upgrade nor list", "One of install, remove, upgrade or list is required."));
        } else {
            runKbuildsycoca();
        }
    }
    delete installer;
    return 0;
}

