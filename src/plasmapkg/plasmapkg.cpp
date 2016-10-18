/******************************************************************************
*   Copyright 2008 Aaron Seigo <aseigo@kde.org>                               *
*   Copyright 2012-2013 Sebastian Kügler <sebas@kde.org>                      *
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

#include "plasmapkg.h"

#include <QDebug>
#include <kservice.h>
#include <kplugintrader.h>
#include <kshell.h>
#include <kconfig.h>
#include <ksycoca.h>
#include <klocalizedstring.h>
#include <KPluginMetaData>

#include <plasma/packagestructure.h>
#include <plasma/package.h>
#include <plasma/pluginloader.h>
#include <kpackage/packageloader.h>
#include <kjob.h>

#include <qcommandlineparser.h>
#include <QDir>
#include <QDBusInterface>
#include <QFileInfo>
#include <QMap>
#include <QStandardPaths>
#include <QStringList>
#include <QTimer>

#include <iostream>
#include <iomanip>

#include "config-plasma.h"

static QTextStream cout(stdout);

namespace Plasma
{
class PlasmaPkgPrivate
{
public:
    QString packageRoot;
    QString packageFile;
    QString package;
    QString servicePrefix;
    QStringList pluginTypes;
    Plasma::PackageStructure *structure;
    Plasma::Package *installer;
    KPluginInfo metadata;
    QString installPath;
    void output(const QString &msg);
    void runKbuildsycoca();
    QStringList packages(const QStringList &types);
    void renderTypeTable(const QMap<QString, QStringList> &plugins);
    void listTypes();
    void coutput(const QString &msg);
    QCommandLineParser *parser;

};

PlasmaPkg::PlasmaPkg(int &argc, char **argv, QCommandLineParser *parser) :
    QCoreApplication(argc, argv)
{
    d = new PlasmaPkgPrivate;
    d->parser = parser;
    QTimer::singleShot(0, this, SLOT(runMain()));
}

PlasmaPkg::~PlasmaPkg()
{
    delete d;
}

void PlasmaPkg::runMain()
{
    Plasma::PackageStructure *structure = new Plasma::PackageStructure;
    if (d->parser->isSet(QStringLiteral("hash"))) {
        const QString path = d->parser->value(QStringLiteral("hash"));
        Plasma::Package package(structure);
        package.setPath(path);
        const QString hash = package.contentsHash();
        if (hash.isEmpty()) {
            d->coutput(i18n("Failed to generate a Package hash for %1", path));
            exit(9);
        } else {
            d->coutput(i18n("SHA1 hash for Package at %1: '%2'", package.path(), hash));
            exit(0);
        }
        return;
    }

    if (d->parser->isSet(QStringLiteral("list-types"))) {
        d->listTypes();
        exit(0);
        return;
    }

    QString type = d->parser->value(QStringLiteral("type"));
    d->pluginTypes.clear();
    d->installer = 0;

    if (d->parser->isSet(QStringLiteral("remove"))) {
        d->package = d->parser->value(QStringLiteral("remove"));
    } else if (d->parser->isSet(QStringLiteral("upgrade"))) {
        d->package = d->parser->value(QStringLiteral("upgrade"));
    } else if (d->parser->isSet(QStringLiteral("install"))) {
        d->package = d->parser->value(QStringLiteral("install"));
    } else if (d->parser->isSet(QStringLiteral("show"))) {
        d->package = d->parser->value(QStringLiteral("show"));
    }

    if (!QDir::isAbsolutePath(d->package)) {
        d->packageFile = QDir(QDir::currentPath() + '/' + d->package).absolutePath();
        d->packageFile = QFileInfo(d->packageFile).canonicalFilePath();
        if (d->parser->isSet(QStringLiteral("upgrade"))) {
            d->package = d->packageFile;
        }
    } else {
        d->packageFile = d->package;
    }

    if (!d->packageFile.isEmpty() && (!d->parser->isSet(QStringLiteral("type")) ||
                                      type.compare(i18nc("package type", "wallpaper"), Qt::CaseInsensitive) == 0 ||
                                      type.compare(QLatin1String("wallpaper"), Qt::CaseInsensitive) == 0)) {
        // Check type for common plasma packages
        Plasma::Package package(structure);
        QString serviceType;
        if (d->parser->isSet(QStringLiteral("remove"))) {
            package.setPath(d->package);
        } else {
            package.setPath(d->packageFile);
        }
        if (package.isValid() && package.metadata().isValid()) {
            serviceType = package.metadata().property(QStringLiteral("X-Plasma-ServiceType")).toString();
        }

        if (!serviceType.isEmpty()) {
            if (serviceType.contains(QStringLiteral("Plasma/Applet")) ||
                    //serviceType.contains("Plasma/PopupApplet") ||
                    serviceType.contains(QStringLiteral("Plasma/Containment"))) {
                type = QStringLiteral("plasmoid");
            } else if (serviceType == QLatin1String("Plasma/Generic")) {
                type = QStringLiteral("package");
            } else if (serviceType == QLatin1String("Plasma/DataEngine")) {
                type = QStringLiteral("dataengine");
            } else if (serviceType == QLatin1String("Plasma/Runner")) {
                type = QStringLiteral("runner");
            } else if (serviceType == QLatin1String("Plasma/LookAndFeel")) {
                type = QStringLiteral("lookandfeel");
            } else if (serviceType == QLatin1String("Plasma/Shell")) {
                type = QStringLiteral("shell");
            } else if (serviceType == QLatin1String("Plasma/Wallpaper")) {
                // This also changes type to wallpaperplugin when --type wallpaper
                // was specified and we have wallpaper plugin package (instead of
                // wallpaper image package)
                type = QStringLiteral("wallpaperplugin");
            } else if (serviceType == QLatin1String("KWin/WindowSwitcher")) {
                type = QStringLiteral("windowswitcher");
            } else if (serviceType == QLatin1String("KWin/Effect")) {
                type = QStringLiteral("kwineffect");
            } else if (serviceType == QLatin1String("KWin/Script")) {
                type = QStringLiteral("kwinscript");
            } else if (serviceType == QLatin1String("Plasma/LayoutTemplate")) {
                type = QStringLiteral("layout-template");
            } else {
                type = serviceType;
                //qDebug() << "fallthrough type is" << serviceType;
            }
        } else {
            if (type.compare(i18nc("package type", "wallpaper"), Qt::CaseInsensitive) == 0) {
                serviceType = QStringLiteral("Plasma/Wallpaper");
            }
        }
    }

    if (type.compare(i18nc("package type", "plasmoid"), Qt::CaseInsensitive) == 0 ||
            type.compare(QLatin1String("plasmoid"), Qt::CaseInsensitive) == 0) {
        d->packageRoot = QStringLiteral(PLASMA_RELATIVE_DATA_INSTALL_DIR "/plasmoids/");
        d->servicePrefix = QStringLiteral("plasma-applet-");
        d->pluginTypes << QStringLiteral("Plasma/Applet");
        d->pluginTypes << QStringLiteral("Plasma/Containment");
    } else if (type.compare(i18nc("package type", "package"), Qt::CaseInsensitive) == 0 ||
               type.compare(QLatin1String("package"), Qt::CaseInsensitive) == 0) {
        d->packageRoot = QStringLiteral(PLASMA_RELATIVE_DATA_INSTALL_DIR "/packages/");
        d->servicePrefix = QStringLiteral("plasma-package-");
        d->pluginTypes << QStringLiteral("Plasma/Generic");
    } else if (type.compare(i18nc("package type", "theme"), Qt::CaseInsensitive) == 0 ||
               type.compare(QLatin1String("theme"), Qt::CaseInsensitive) == 0) {
        d->packageRoot = QStringLiteral(PLASMA_RELATIVE_DATA_INSTALL_DIR "/desktoptheme/");
        d->pluginTypes << QStringLiteral("Plasma/Theme");
    } else if (type.compare(i18nc("package type", "wallpaper"), Qt::CaseInsensitive) == 0 ||
               type.compare(QLatin1String("wallpaper"), Qt::CaseInsensitive) == 0) {
        d->pluginTypes << QStringLiteral("Plasma/ImageWallpaper"); // we'll catch that later
        d->packageRoot = QStringLiteral("wallpapers/");
        d->servicePrefix = QStringLiteral("plasma-wallpaper-");
    } else if (type.compare(i18nc("package type", "dataengine"), Qt::CaseInsensitive) == 0 ||
               type.compare(QLatin1String("dataengine"), Qt::CaseInsensitive) == 0) {
        d->packageRoot = QStringLiteral(PLASMA_RELATIVE_DATA_INSTALL_DIR "/dataengines/");
        d->servicePrefix = QStringLiteral("plasma-dataengine-");
        d->pluginTypes << QStringLiteral("Plasma/DataEngine");
    } else if (type.compare(i18nc("package type", "runner"), Qt::CaseInsensitive) == 0 ||
               type.compare(QLatin1String("runner"), Qt::CaseInsensitive) == 0) {
        d->packageRoot = QStringLiteral(PLASMA_RELATIVE_DATA_INSTALL_DIR "/runners/");
        d->servicePrefix = QStringLiteral("plasma-runner-");
        d->pluginTypes << QStringLiteral("Plasma/Runner");
    } else if (type.compare(i18nc("package type", "wallpaperplugin"), Qt::CaseInsensitive) == 0 ||
               type.compare(QLatin1String("wallpaperplugin"), Qt::CaseInsensitive) == 0) {
        d->packageRoot = QStringLiteral(PLASMA_RELATIVE_DATA_INSTALL_DIR "/wallpapers/");
        d->servicePrefix = QStringLiteral("plasma-wallpaper-");
        d->pluginTypes << QStringLiteral("Plasma/Wallpaper");
    } else if (type.compare(i18nc("package type", "lookandfeel"), Qt::CaseInsensitive) == 0 ||
               type.compare(QLatin1String("lookandfeel"), Qt::CaseInsensitive) == 0) {
        d->packageRoot = QStringLiteral(PLASMA_RELATIVE_DATA_INSTALL_DIR "/look-and-feel/");
        d->servicePrefix = QStringLiteral("plasma-lookandfeel-");
        d->pluginTypes << QStringLiteral("Plasma/LookAndFeel");
    } else if (type.compare(i18nc("package type", "shell"), Qt::CaseInsensitive) == 0 ||
               type.compare(QLatin1String("shell"), Qt::CaseInsensitive) == 0) {
        d->packageRoot = QStringLiteral(PLASMA_RELATIVE_DATA_INSTALL_DIR "/shells/");
        d->servicePrefix = QStringLiteral("plasma-shell-");
        d->pluginTypes << QStringLiteral("Plasma/Shell");
    } else if (type.compare(i18nc("package type", "layout-template"), Qt::CaseInsensitive) == 0 ||
               type.compare(QLatin1String("layout-template"), Qt::CaseInsensitive) == 0) {
        d->packageRoot = QStringLiteral(PLASMA_RELATIVE_DATA_INSTALL_DIR "/layout-templates/");
        d->servicePrefix = QStringLiteral("plasma-layout-");
        d->pluginTypes << QStringLiteral("Plasma/LayoutTemplate");
    } else if (type.compare(i18nc("package type", "kwineffect"), Qt::CaseInsensitive) == 0 ||
               type.compare(QLatin1String("kwineffect"), Qt::CaseInsensitive) == 0) {
        d->packageRoot = QStringLiteral("kwin/effects/");
        d->servicePrefix = QStringLiteral("kwin-effect-");
        d->pluginTypes << QStringLiteral("KWin/Effect");
    } else if (type.compare(i18nc("package type", "windowswitcher"), Qt::CaseInsensitive) == 0 ||
               type.compare(QLatin1String("windowswitcher"), Qt::CaseInsensitive) == 0) {
        d->packageRoot = QStringLiteral("kwin/tabbox/");
        d->servicePrefix = QStringLiteral("kwin-windowswitcher-");
        d->pluginTypes << QStringLiteral("KWin/WindowSwitcher");
    } else if (type.compare(i18nc("package type", "kwinscript"), Qt::CaseInsensitive) == 0 ||
               type.compare(QLatin1String("kwinscript"), Qt::CaseInsensitive) == 0) {
        d->packageRoot = QStringLiteral("kwin/scripts/");
        d->servicePrefix = QStringLiteral("kwin-script-");
        d->pluginTypes << QStringLiteral("KWin/Script");

    //do it trough normal plugin loading
    } else {
        Plasma::Package p = Plasma::PluginLoader::self()->loadPackage(type);
        if (!p.hasValidStructure()) {
            d->coutput(i18n("Could not find a suitable installer for package of type %1", type));
            exit(5);
            return;
        }

        d->installer = new Plasma::Package(p);

        //d->packageRoot = d->installer->defaultPackageRoot();
        d->pluginTypes << type;
    }
    if (d->parser->isSet(QStringLiteral("show"))) {
        const QString pluginName = d->package;
        showPackageInfo(pluginName);
        exit(0);
        return;
    }

    if (d->parser->isSet(QStringLiteral("list"))) {
        d->coutput(i18n("Listing service types: %1", d->pluginTypes.join(QStringLiteral(", "))));
        listPackages(d->pluginTypes);
        exit(0);
    } else {
        // install, remove or upgrade
        if (!d->installer) {

            d->installer = new Plasma::Package(new Plasma::PackageStructure());
            d->installer->setServicePrefix(d->servicePrefix);
        }

        d->packageRoot = findPackageRoot(d->package, d->packageRoot);

        if (d->parser->isSet(QStringLiteral("remove")) || d->parser->isSet(QStringLiteral("upgrade"))) {
            QString pkgPath;
            foreach (const QString &t, d->pluginTypes) {
                Plasma::Package pkg = Plasma::PluginLoader::self()->loadPackage(t);
                pkg.setPath(d->package);
                if (pkg.isValid()) {
                    pkgPath = pkg.path();
                    if (pkgPath.isEmpty() && !d->packageFile.isEmpty()) {
                        pkgPath = d->packageFile;
                    }
                    continue;
                }
            }
            if (pkgPath.isEmpty()) {
                pkgPath = d->package;
            }

            if (d->parser->isSet(QStringLiteral("upgrade"))) {
                d->installer->setPath(d->package);
            }
            QString _p = d->packageRoot;
            if (!_p.endsWith('/')) {
                _p.append('/');
            }
            _p.append(d->package);
            d->installer->setDefaultPackageRoot(d->packageRoot);
            d->installer->setPath(pkgPath);
            QString pluginName;
            if (d->installer->isValid()) {
                d->metadata = d->installer->metadata();
                if (!d->metadata.isValid()) {
                    pluginName = d->package;
                } else if (!d->metadata.isValid() && d->metadata.pluginName().isEmpty()) {
                    // plugin name given in command line
                    pluginName = d->package;
                } else {
                    // Parameter was a plasma package, get plugin name from the package
                    pluginName = d->metadata.pluginName();
                }
            }
            QStringList installed = d->packages(d->pluginTypes);

            if (QFile::exists(d->packageFile)) {
                d->installer->setPath(d->packageFile);
                if (d->installer->isValid() && d->installer->metadata().isValid()) {
                    pluginName = d->installer->metadata().pluginName();
                }
            }
            // Uninstalling ...
            if (installed.contains(pluginName)) { // Assume it's a plugin name
                d->installer->setPath(pluginName);
                KJob *uninstallJob = d->installer->uninstall(pluginName, d->packageRoot);
                connect(uninstallJob, SIGNAL(result(KJob*)), SLOT(packageUninstalled(KJob*)));
                return;
            } else {
                d->coutput(i18n("Error: Plugin %1 is not installed.", pluginName));
                exit(2);
            }
        }
        if (d->parser->isSet(QStringLiteral("install"))) {
            KJob *installJob = d->installer->install(d->packageFile, d->packageRoot);
            connect(installJob, SIGNAL(result(KJob*)), SLOT(packageInstalled(KJob*)));
            return;
        }
        if (d->package.isEmpty()) {
            qWarning() << i18nc("No option was given, this is the error message telling the user he needs at least one, do not translate install, remove, upgrade nor list", "One of install, remove, upgrade or list is required.");
            exit(6);
        } else {
            d->runKbuildsycoca();
        }
    }
    delete d->installer;
}

void PlasmaPkgPrivate::coutput(const QString &msg)
{
    cout << msg.toLocal8Bit().constData() << endl;
}

void PlasmaPkgPrivate::runKbuildsycoca()
{
    return;
    if (KSycoca::isAvailable()) {
        QDBusInterface dbus(QStringLiteral("org.kde.kded5"), QStringLiteral("/kbuildsycoca"), QStringLiteral("org.kde.kbuildsycoca"));
        dbus.call(QDBus::NoBlock, QStringLiteral("recreate"));
    }
}

QStringList PlasmaPkgPrivate::packages(const QStringList &types)
{
    QStringList result;

    foreach (const QString &type, types) {

        if (type.compare(QLatin1String("Plasma/Generic"), Qt::CaseInsensitive) == 0) {
            const QStringList &packs = QStandardPaths::locateAll(QStandardPaths::GenericDataLocation, QStringLiteral(PLASMA_RELATIVE_DATA_INSTALL_DIR "/packages/"), QStandardPaths::LocateDirectory);
            foreach (const QString &ppath, packs) {
                const QDir cd(ppath);
                const QStringList &entries = cd.entryList(QDir::Dirs);
                foreach (const QString& pack, entries) {
                    if ((pack != QLatin1String(".") && pack != QLatin1String("..")) &&
                            (QFile::exists(ppath + '/' + pack + "/metadata.desktop"))) {

                        result << pack;
                    }
                }
            }
        }

        if (type.compare(QLatin1String("Plasma/ImageWallpaper"), Qt::CaseInsensitive) == 0) {
            const QStringList &packs = QStandardPaths::locateAll(QStandardPaths::GenericDataLocation, QStringLiteral("wallpapers/"), QStandardPaths::LocateDirectory);
            foreach (const QString &ppath, packs) {
                const QDir cd(ppath);
                const QStringList &entries = cd.entryList(QDir::Dirs);
                foreach (const QString& pack, entries) {
                    if ((pack != QLatin1String(".") && pack != QLatin1String("..")) &&
                            (QFile::exists(ppath + '/' + pack + "/metadata.desktop"))) {

                        result << pack;
                    }
                }
            }
        }

        if (type.compare(QLatin1String("Plasma/Theme"), Qt::CaseInsensitive) == 0) {
            const QStringList &packs = QStandardPaths::locateAll(QStandardPaths::GenericDataLocation, QStringLiteral(PLASMA_RELATIVE_DATA_INSTALL_DIR "/desktoptheme/"), QStandardPaths::LocateDirectory);
            foreach (const QString &ppath, packs) {
                const QDir cd(ppath);
                const QStringList &entries = cd.entryList(QDir::Dirs);
                foreach (const QString& pack, entries) {
                    if ((pack != QLatin1String(".") && pack != QLatin1String("..")) &&
                            (QFile::exists(ppath + '/' + pack + "/metadata.desktop"))) {

                        result << pack;
                    }
                }
            }
        }

        //Loading a package of the given type, caches the proper
        //packagestructures in the packageloader, making the listing
        //of packages succeed
        Plasma::PluginLoader::self()->loadPackage(type);
        const QList<KPluginMetaData> plugins = KPackage::PackageLoader::self()->listPackages(type);
        for (auto& plugin : plugins) {
            const QString _plugin = plugin.pluginId();
            if (!result.contains(_plugin)) {
                result << _plugin;
            }
        }
    }

    return result;
}

void PlasmaPkg::showPackageInfo(const QString &pluginName)
{
    QString type = QStringLiteral("Plasma/Applet");
    if (!d->pluginTypes.contains(type) && d->pluginTypes.count() > 0) {
        type = d->pluginTypes.at(0);
    }
    Plasma::Package pkg = Plasma::PluginLoader::self()->loadPackage(type);

    pkg.setDefaultPackageRoot(d->packageRoot);

    if (QFile::exists(d->packageFile)) {
        pkg.setPath(d->packageFile);
    } else {
        pkg.setPath(pluginName);
    }

    KPluginInfo i = pkg.metadata();
    if (!i.isValid()) {
        d->coutput(i18n("Error: Can't find plugin metadata: %1", pluginName));
        exit(3);
    }
    d->coutput(i18n("Showing info for package: %1", pluginName));
    d->coutput(i18n("      Name : %1", i.name()));
    d->coutput(i18n("   Comment : %1", i.comment()));
    d->coutput(i18n("    Plugin : %1", i.pluginName()));
    d->coutput(i18n("    Author : %1", i.author()));
    d->coutput(i18n("      Path : %1", pkg.path()));

    exit(0);
}

QString PlasmaPkg::findPackageRoot(const QString &pluginName, const QString &prefix)
{
    Q_UNUSED(pluginName)
    Q_UNUSED(prefix)
    QString packageRoot;
    if (d->parser->isSet(QStringLiteral("packageroot")) && d->parser->isSet(QStringLiteral("global"))) {
        qWarning() << i18nc("The user entered conflicting options packageroot and global, this is the error message telling the user he can use only one", "The packageroot and global options conflict each other, please select only one.");
        ::exit(7);
    } else if (d->parser->isSet(QStringLiteral("packageroot"))) {
        packageRoot = d->parser->value(QStringLiteral("packageroot"));
        //qDebug() << "(set via arg) d->packageRoot is: " << d->packageRoot;
    } else if (d->parser->isSet(QStringLiteral("global"))) {
        packageRoot = QStandardPaths::locateAll(QStandardPaths::GenericDataLocation, d->packageRoot, QStandardPaths::LocateDirectory).last();
    } else {
        packageRoot = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QLatin1Char('/') + d->packageRoot;
    }
    return packageRoot;
}

void PlasmaPkg::listPackages(const QStringList &types)
{
    QStringList list = d->packages(types);
    list.sort();
    foreach (const QString &package, list) {
        d->coutput(package);
    }
    exit(0);
}

void PlasmaPkgPrivate::renderTypeTable(const QMap<QString, QStringList> &plugins)
{
    const QString nameHeader = i18n("Addon Name");
    const QString pluginHeader = i18n("Service Type");
    const QString pathHeader = i18n("Path");
    const QString typeHeader = i18n("Type Argument");
    int nameWidth = nameHeader.length();
    int pluginWidth = pluginHeader.length();
    int pathWidth = pathHeader.length();
    int typeWidth = typeHeader.length();

    QMapIterator<QString, QStringList> pluginIt(plugins);
    while (pluginIt.hasNext()) {
        pluginIt.next();
        if (pluginIt.key().length() > nameWidth) {
            nameWidth = pluginIt.key().length();
        }

        if (pluginIt.value()[0].length() > pluginWidth) {
            pluginWidth = pluginIt.value()[0].length();
        }

        if (pluginIt.value()[1].length() > pathWidth) {
            pathWidth = pluginIt.value()[1].length();
        }

        if (pluginIt.value()[2].length() > typeWidth) {
            typeWidth = pluginIt.value()[2].length();
        }
    }

    std::cout << nameHeader.toLocal8Bit().constData() << std::setw(nameWidth - nameHeader.length() + 2) << ' '
              << pluginHeader.toLocal8Bit().constData() << std::setw(pluginWidth - pluginHeader.length() + 2) << ' '
              << pathHeader.toLocal8Bit().constData() << std::setw(pathWidth - pathHeader.length() + 2) << ' '
              << typeHeader.toLocal8Bit().constData() << std::endl;
    std::cout << std::setfill('-') << std::setw(nameWidth) << '-' << "  "
              << std::setw(pluginWidth) << '-' << "  "
              << std::setw(pathWidth) << '-' << "  "
              << std::setw(typeWidth) << '-' << std::endl;
    std::cout << std::setfill(' ');

    pluginIt.toFront();
    while (pluginIt.hasNext()) {
        pluginIt.next();
        std::cout << pluginIt.key().toLocal8Bit().constData() << std::setw(nameWidth - pluginIt.key().length() + 2) << ' '
                  << pluginIt.value()[0].toLocal8Bit().constData() << std::setw(pluginWidth - pluginIt.value()[0].length() + 2) << ' '
                  << pluginIt.value()[1].toLocal8Bit().constData() << std::setw(pathWidth - pluginIt.value()[1].length() + 2) << ' '
                  << pluginIt.value()[2].toLocal8Bit().constData() << std::endl;
    }
}

void PlasmaPkgPrivate::listTypes()
{
    coutput(i18n("Package types that are installable with this tool:"));
    coutput(i18n("Built in:"));

    QMap<QString, QStringList> builtIns;
    builtIns.insert(i18n("DataEngine"), QStringList() << QStringLiteral("Plasma/DataEngine") << QStringLiteral(PLASMA_RELATIVE_DATA_INSTALL_DIR "/dataengines/") << QStringLiteral("dataengine"));
    builtIns.insert(i18n("Layout Template"), QStringList() << QStringLiteral("Plasma/LayoutTemplate") << QStringLiteral(PLASMA_RELATIVE_DATA_INSTALL_DIR "/layout-templates/") << QStringLiteral("layout-template"));
    builtIns.insert(i18n("Look and Feel"), QStringList() << QStringLiteral("Plasma/LookAndFeel") << QStringLiteral(PLASMA_RELATIVE_DATA_INSTALL_DIR "/look-and-feel/") << QStringLiteral("lookandfeel"));
    builtIns.insert(i18n("Package"), QStringList() << QStringLiteral("Plasma/Generic") << QStringLiteral(PLASMA_RELATIVE_DATA_INSTALL_DIR "/packages/") << QStringLiteral("package"));
    builtIns.insert(i18n("Plasmoid"), QStringList() << QStringLiteral("Plasma/Applet") << QStringLiteral(PLASMA_RELATIVE_DATA_INSTALL_DIR "/plasmoids/") << QStringLiteral("plasmoid"));
    builtIns.insert(i18n("Runner"), QStringList() << QStringLiteral("Plasma/Runner") << QStringLiteral(PLASMA_RELATIVE_DATA_INSTALL_DIR "/runners/") << QStringLiteral("runner"));
    builtIns.insert(i18n("Shell"), QStringList() << QStringLiteral("Plasma/Shell") << QStringLiteral(PLASMA_RELATIVE_DATA_INSTALL_DIR "/shells/") << QStringLiteral("shell"));
    builtIns.insert(i18n("Theme"), QStringList() << "" << QStringLiteral(PLASMA_RELATIVE_DATA_INSTALL_DIR "/desktoptheme/") << QStringLiteral("theme"));
    builtIns.insert(i18n("Wallpaper Images"), QStringList() << "" << QStringLiteral("wallpapers/") << QStringLiteral("wallpaper"));
    builtIns.insert(i18n("Animated Wallpaper"), QStringList() << QStringLiteral("Plasma/Wallpaper") << QStringLiteral(PLASMA_RELATIVE_DATA_INSTALL_DIR "/wallpapers/") << QStringLiteral("wallpaperplugin"));
    builtIns.insert(i18n("KWin Effect"), QStringList() << QStringLiteral("KWin/Effect") << QStringLiteral("kwin/effects/") << QStringLiteral("kwineffect"));
    builtIns.insert(i18n("KWin Window Switcher"), QStringList() << QStringLiteral("KWin/WindowSwitcher") << QStringLiteral("kwin/tabbox/") << QStringLiteral("windowswitcher"));
    builtIns.insert(i18n("KWin Script"), QStringList() << QStringLiteral("KWin/Script") << QStringLiteral("kwin/scripts/") << QStringLiteral("kwinscript"));
    renderTypeTable(builtIns);

    const KPluginInfo::List offers = KPluginTrader::self()->query(QStringLiteral("kpackage/packagestructure"), QStringLiteral("KPackage/PackageStructure"));

    if (!offers.isEmpty()) {
        std::cout << std::endl;
        coutput(i18n("Provided by plugins:"));

        QMap<QString, QStringList> plugins;
        for (auto& info : offers) {
            //const QString proot = "";
            //Plasma::PackageStructure* structure = Plasma::PackageStructure::load(info.pluginName());
            QString name = info.name();
            QString comment = info.comment();
            QString plugin = info.pluginName();
            //QString path = structure->defaultPackageRoot();
            //QString path = defaultPackageRoot;
            plugins.insert(name, QStringList() << name << plugin << comment);
            //qDebug() << "KService stuff:" << name << plugin << comment;
        }

        renderTypeTable(plugins);
    }

    QStringList desktopFiles = QStandardPaths::locateAll(QStandardPaths::DataLocation, QStringLiteral(PLASMA_RELATIVE_DATA_INSTALL_DIR "/packageformats/*rc"), QStandardPaths::LocateFile);

    if (!desktopFiles.isEmpty()) {
        coutput(i18n("Provided by .desktop files:"));
        Plasma::PackageStructure structure;
        QMap<QString, QStringList> plugins;
        foreach (const QString &file, desktopFiles) {
            // extract the type
            KConfig config(file, KConfig::SimpleConfig);
#pragma message("read config here")
            // structure.read(&config);
            // get the name based on the rc file name, just as Plasma::PackageStructure does
            // const QString name = file.left(file.length() - 2);
            //plugins.insert(name, QStringList() << structure.type() << structure.defaultPackageRoot());
        }
    }
}

void PlasmaPkg::packageInstalled(KJob *job)
{
    bool success = (job->error() == KJob::NoError);
    int exitcode = 0;
    if (success) {
        if (d->parser->isSet(QStringLiteral("upgrade"))) {
            d->coutput(i18n("Successfully upgraded %1", d->packageFile));
        } else {
            d->coutput(i18n("Successfully installed %1", d->packageFile));
        }
    } else {
        d->coutput(i18n("Error: Installation of %1 failed: %2", d->packageFile, job->errorText()));
        exitcode = 4;
    }
    exit(exitcode);
}

void PlasmaPkg::packageUninstalled(KJob *job)
{
    bool success = (job->error() == KJob::NoError);
    int exitcode = 0;
    if (success) {
        if (d->parser->isSet(QStringLiteral("upgrade"))) {
            d->coutput(i18n("Upgrading package from file: %1", d->packageFile));
            KJob *installJob = d->installer->install(d->packageFile, d->packageRoot);
            connect(installJob, SIGNAL(result(KJob*)), SLOT(packageInstalled(KJob*)));
            return;
        }
        d->coutput(i18n("Successfully uninstalled %1", d->packageFile));
    } else {
        d->coutput(i18n("Error: Uninstallation of %1 failed: %2", d->packageFile, job->errorText()));
        exitcode = 7;
    }
    exit(exitcode);
}

} // namespace Plasma

#include "moc_plasmapkg.cpp"

