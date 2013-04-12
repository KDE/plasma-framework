/******************************************************************************
*   Copyright 2008 Aaron Seigo <aseigo@kde.org>                               *
*   Copyright 2012 Sebastian Kügler <sebas@kde.org>                           *
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

#include <kcmdlineargs.h>
#include <kdebug.h>
#include <kservice.h>
#include <kservicetypetrader.h>
#include <kshell.h>
#include <KSycoca>
#include <klocalizedstring.h>

#include <plasma/packagestructure.h>
#include <plasma/package.h>
#include <plasma/pluginloader.h>
#include <KGlobal>
#include <klocale.h>
#include <kjob.h>

#include <QDir>
#include <QDBusInterface>
#include <QFileInfo>
#include <QMap>
#include <QStandardPaths>
#include <QStringList>
#include <QTimer>

#include <iostream>
#include <iomanip>


namespace Plasma
{
class PlasmaPkgPrivate {
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
    QStringList packages(const QStringList& types);
    void renderTypeTable(const QMap<QString, QStringList> &plugins);
    void listTypes();
    void coutput(const QString &msg);
    KCmdLineArgs *args;

};

PlasmaPkg::PlasmaPkg(int& argc, char** argv) :
    QCoreApplication(argc, argv)
{
    d = new PlasmaPkgPrivate;
    QTimer::singleShot(0, this, SLOT(runMain()));
}

PlasmaPkg::~PlasmaPkg()
{
    delete d;
}

void PlasmaPkg::runMain()
{
    d->args = KCmdLineArgs::parsedArgs();

    Plasma::PackageStructure* structure = new Plasma::PackageStructure;
    if (d->args->isSet("hash")) {
        const QString path = d->args->getOption("hash");
        Plasma::Package package(structure);
        package.setPath(path);
        const QString hash = package.contentsHash();
        if (hash.isEmpty()) {
            d->coutput(i18n("Failed to generate a Package hash for %1", path));
            exit(1);
        } else {
            d->coutput(i18n("SHA1 hash for Package at %1: '%2'", package.path(), hash));
            exit(0);
        }
        return;
    }

    if (d->args->isSet("list-types")) {
        d->listTypes();
        exit(0);
        return;
    }

    QString type = d->args->getOption("type");
    QString packageRoot = type;
    d->pluginTypes.clear();
    d->installer = 0;

    if (d->args->isSet("remove")) {
        d->package = d->args->getOption("remove");
    } else if (d->args->isSet("upgrade")) {
        d->package = d->args->getOption("upgrade");
    } else if (d->args->isSet("install")) {
        d->package = d->args->getOption("install");
    } else if (d->args->isSet("show")) {
        d->package = d->args->getOption("show");
    }

    if (d->args->isSet("show")) {
        const QString pluginName = d->package;
        showPackageInfo(pluginName);
        exit(0);
        return;
    }

    if (!QDir::isAbsolutePath(d->package)) {
        d->packageFile = QDir(QDir::currentPath() + '/' + d->package).absolutePath();
        d->packageFile = QFileInfo(d->packageFile).canonicalFilePath();
        if (d->args->isSet("upgrade")) {
            d->package = d->packageFile;
        }
    } else {
        d->packageFile = d->package;
    }

    if (!d->packageFile.isEmpty() && (!d->args->isSet("type") ||
        type.compare(i18nc("package type", "wallpaper"), Qt::CaseInsensitive) == 0 ||
        type.compare("wallpaper", Qt::CaseInsensitive) == 0)) {
        // Check type for common plasma packages
        Plasma::Package package(structure);
        QString serviceType;
        if (d->args->isSet("remove")) {
            package.setPath(d->package);
        } else {
            package.setPath(d->packageFile);
        }
        if (package.isValid()) {
            serviceType = package.metadata().property("X-Plasma-ServiceType").toString();
        }

        if (!serviceType.isEmpty()) {
            if (serviceType.contains("Plasma/Applet") ||
                serviceType.contains("Plasma/PopupApplet") ||
                serviceType.contains("Plasma/Containment")) {
                type = "plasmoid";
            } else if (serviceType == "Plasma/Generic") {
                type = "package";
            } else if (serviceType == "Plasma/DataEngine") {
                type = "dataengine";
            } else if (serviceType == "Plasma/Runner") {
                type = "runner";
            } else if (serviceType == "Plasma/Wallpaper") {
                // This also changes type to wallpaperplugin when --type wallpaper
                // was specified and we have wallpaper plugin package (instead of
                // wallpaper image package)
                type = "wallpaperplugin";
            } else if (serviceType == "KWin/WindowSwitcher") {
                type = "windowswitcher";
            } else if (serviceType == "KWin/Effect") {
                type = "kwineffect";
            } else if (serviceType == "KWin/Script") {
                type = "kwinscript";
            } else if (serviceType == "Plasma/LayoutTemplate") {
                type = "layout-template";
            } else {
                type = serviceType;
                //qDebug() << "fallthrough type is" << serviceType;
            }
        } else {
            if (type.compare(i18nc("package type", "wallpaper"), Qt::CaseInsensitive) == 0) {
                serviceType = "Plasma/Wallpaper";
            }
        }
    }

    if (type.compare(i18nc("package type", "plasmoid"), Qt::CaseInsensitive) == 0 ||
        type.compare("plasmoid", Qt::CaseInsensitive) == 0) {
        d->packageRoot = "plasma/plasmoids/";
        d->servicePrefix = "plasma-applet-";
        d->pluginTypes << "Plasma/Applet";
        d->pluginTypes << "Plasma/PopupApplet";
        d->pluginTypes << "Plasma/Containment";
    } else if (type.compare(i18nc("package type", "package"), Qt::CaseInsensitive) == 0 ||
               type.compare("theme", Qt::CaseInsensitive) == 0) {
        d->packageRoot = "plasma/packages/";
        d->servicePrefix = "plasma-package-";
        d->pluginTypes << "Plasma/Generic";
    } else if (type.compare(i18nc("package type", "theme"), Qt::CaseInsensitive) == 0 ||
               type.compare("theme", Qt::CaseInsensitive) == 0) {
        d->packageRoot = "desktoptheme/";
    } else if (type.compare(i18nc("package type", "wallpaper"), Qt::CaseInsensitive) == 0 ||
               type.compare("wallpaper", Qt::CaseInsensitive) == 0) {
        d->pluginTypes << "Plasma/Wallpaper";
        d->packageRoot = "plasma/wallpapers/";
        d->servicePrefix = "plasma-wallpaper-";
    } else if (type.compare(i18nc("package type", "dataengine"), Qt::CaseInsensitive) == 0 ||
               type.compare("dataengine", Qt::CaseInsensitive) == 0) {
        d->packageRoot = "plasma/dataengines/";
        d->servicePrefix = "plasma-dataengine-";
        d->pluginTypes << "Plasma/DataEngine";
    } else if (type.compare(i18nc("package type", "runner"), Qt::CaseInsensitive) == 0 ||
               type.compare("runner", Qt::CaseInsensitive) == 0) {
        d->packageRoot = "plasma/runners/";
        d->servicePrefix = "plasma-runner-";
        d->pluginTypes << "Plasma/Runner";
    } else if (type.compare(i18nc("package type", "wallpaperplugin"), Qt::CaseInsensitive) == 0 ||
               type.compare("wallpaperplugin", Qt::CaseInsensitive) == 0) {
        d->packageRoot = "plasma/wallpapers/";
        d->servicePrefix = "plasma-wallpaper-";
        d->pluginTypes << "Plasma/Wallpaper";
    } else if (type.compare(i18nc("package type", "layout-template"), Qt::CaseInsensitive) == 0 ||
               type.compare("layout-template", Qt::CaseInsensitive) == 0) {
        d->packageRoot = "plasma/layout-templates/";
        d->servicePrefix = "plasma-layout-";
        d->pluginTypes << "Plasma/LayoutTemplate";
    } else if (type.compare(i18nc("package type", "kwineffect"), Qt::CaseInsensitive) == 0 ||
               type.compare("kwineffect", Qt::CaseInsensitive) == 0) {
        d->packageRoot = "kwin/effects/";
        d->servicePrefix = "kwin-effect-";
        d->pluginTypes << "KWin/Effect";
    } else if (type.compare(i18nc("package type", "windowswitcher"), Qt::CaseInsensitive) == 0 ||
               type.compare("windowswitcher", Qt::CaseInsensitive) == 0) {
        d->packageRoot = "kwin/tabbox/";
        d->servicePrefix = "kwin-windowswitcher-";
        d->pluginTypes << "KWin/WindowSwitcher";
    } else if (type.compare(i18nc("package type", "kwinscript"), Qt::CaseInsensitive) == 0 ||
               type.compare("kwinscript", Qt::CaseInsensitive) == 0) {
        d->packageRoot = "kwin/scripts/";
        d->servicePrefix = "kwin-script-";
        d->pluginTypes << "KWin/Script";
    } else /* if (KSycoca::isAvailable()) */ {
        const QString constraint = QString("[X-KDE-PluginInfo-Name] == '%1'").arg(type);
        KService::List offers = KServiceTypeTrader::self()->query("Plasma/PackageStructure", constraint);
        if (offers.isEmpty()) {
            d->coutput(i18n("Could not find a suitable installer for package of type %1", type));
            exit(1);
            return;
        }
        kWarning() << "custom PackageStructure plugins not ported";
        KService::Ptr offer = offers.first();
        QString error;

        d->installer = new Plasma::Package(offer->createInstance<Plasma::PackageStructure>(0, QVariantList(), &error));

        if (!d->installer) {
            d->coutput(i18n("Could not load installer for package of type %1. Error reported was: %2",
                        d->args->getOption("type"), error));
            return;
        }

        //d->packageRoot = d->installer->defaultPackageRoot();
        //pluginTypes << d->installer->type();
    }

    if (d->args->isSet("list")) {
        qDebug() << "listing " << d->pluginTypes << "!";
        listPackages(d->pluginTypes);
        exit(0);
    } else {
        // install, remove or upgrade
        if (!d->installer) {

            d->installer = new Plasma::Package(new Plasma::PackageStructure());
            //d->installer->setServicePrefix(d->servicePrefix);
        }

        d->packageRoot = findPackageRoot(d->package, d->packageRoot);

        if (d->args->isSet("remove") || d->args->isSet("upgrade")) {
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

            if (d->args->isSet("upgrade")) {
                d->installer->setPath(d->package);
            }
            QString _p = d->packageRoot;
            if (!_p.endsWith('/')) {
                _p.append('/');
            }
            _p.append(d->package);
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
            //qDebug() << "installed wallpapers:"  << installed << d->pluginTypes;
            if (installed.contains(pluginName)) {
                d->installer->setPath(pluginName);
                KJob *uninstallJob = d->installer->uninstall(pluginName, d->packageRoot);
                connect(uninstallJob, SIGNAL(result(KJob*)), SLOT(packageUninstalled(KJob*)));
                return;
            } else {
                d->coutput(i18n("Plugin %1 is not installed.", pluginName));
                exit(1);
            }
        }
        if (d->args->isSet("install")) {
            KJob *installJob = d->installer->install(d->packageFile, d->packageRoot);
            connect(installJob, SIGNAL(result(KJob*)), SLOT(packageInstalled(KJob*)));
            return;
        }
        if (d->package.isEmpty()) {
            KCmdLineArgs::usageError(i18nc("No option was given, this is the error message telling the user he needs at least one, do not translate install, remove, upgrade nor list", "One of install, remove, upgrade or list is required."));
            exit(1);
        } else {
            d->runKbuildsycoca();
        }
    }
    delete d->installer;
}

void PlasmaPkgPrivate::coutput(const QString &msg)
{
    std::cout << msg.toLocal8Bit().constData() << std::endl;
}

void PlasmaPkgPrivate::runKbuildsycoca()
{
    return;
    if (KSycoca::isAvailable()) {
        QDBusInterface dbus("org.kde.kded", "/kbuildsycoca", "org.kde.kbuildsycoca");
        dbus.call(QDBus::NoBlock, "recreate");
    }
}

QStringList PlasmaPkgPrivate::packages(const QStringList& types)
{
    QStringList result;

    foreach (const QString& type, types) {
        if (type.compare("Plasma/Wallpaper", Qt::CaseInsensitive) == 0) {
            const QStringList &wallies = QStandardPaths::locateAll(QStandardPaths::GenericDataLocation, "plasma/wallpapers/", QStandardPaths::LocateDirectory);
            foreach (const QString &wpath, wallies) {
                const QDir cd(wpath);
                const QStringList &entries = cd.entryList(QDir::Dirs);
                foreach (const QString wallpap, entries) {
                    if ((wallpap != "." && wallpap != "..") &&
                        (QFile::exists(wpath+'/'+wallpap+"/metadata.desktop"))) {

                        result << wallpap;
                    }
                }
            }
        }
        if (type.compare("Plasma/Generic", Qt::CaseInsensitive) == 0) {
            const QStringList &packs = QStandardPaths::locateAll(QStandardPaths::GenericDataLocation, "plasma/packages/", QStandardPaths::LocateDirectory);
            foreach (const QString &ppath, packs) {
                const QDir cd(ppath);
                const QStringList &entries = cd.entryList(QDir::Dirs);
                foreach (const QString pack, entries) {
                    if ((pack != "." && pack != "..") &&
                        (QFile::exists(ppath+'/'+pack+"/metadata.desktop"))) {

                        result << pack;
                    }
                }
            }
        }
        const KService::List services = KServiceTypeTrader::self()->query(type);
        foreach (const KService::Ptr &service, services) {
            const QString _plugin = service->property("X-KDE-PluginInfo-Name", QVariant::String).toString();
            if (!result.contains(_plugin)) {
                result << _plugin;
            }
        }
    }

    return result;
}

void PlasmaPkg::showPackageInfo(const QString& pluginName)
{
//     qDebug() << "showPackageInfo" << pluginName;
    Plasma::Package pkg = Plasma::PluginLoader::self()->loadPackage("Plasma/Applet");

    //QString p = findPackageRoot("org.kde.microblog-qml", "plasma/plasmoids/");

    pkg.setPath(pluginName);

    KPluginInfo i = pkg.metadata();
    if (!i.isValid()) {
        d->coutput(i18n("Can't find plugin metadata: %1", pluginName));
        exit(1);
    }
    d->coutput(i18n("Showing info for package: %1", pluginName));
    d->coutput(i18n("      Name : %1", i.name()));
    d->coutput(i18n("   Comment : %1", i.comment()));
    d->coutput(i18n("    Plugin : %1", i.pluginName()));
    d->coutput(i18n("    Author : %1", i.author()));
    d->coutput(i18n("      Path : %1", pkg.path()));

    exit(0);
}

QString PlasmaPkg::findPackageRoot(const QString& pluginName, const QString& prefix)
{
    QString packageRoot;
    if (d->args->isSet("packageroot") && d->args->isSet("global")) {
        KCmdLineArgs::usageError(i18nc("The user entered conflicting options packageroot and global, this is the error message telling the user he can use only one", "The packageroot and global options conflict each other, please select only one."));
    } else if (d->args->isSet("packageroot")) {
        packageRoot = d->args->getOption("packageroot");
        //qDebug() << "(set via arg) d->packageRoot is: " << d->packageRoot;
    } else if (d->args->isSet("global")) {
        packageRoot = QStandardPaths::locateAll(QStandardPaths::GenericDataLocation, d->packageRoot, QStandardPaths::LocateDirectory).last();
        //qDebug() << "(set via locateAll) d->packageRoot is: " << d->packageRoot;
    } else {
        packageRoot = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QLatin1Char('/') +  d->packageRoot;
        //qDebug() << "(set via locateLocal) d->packageRoot is: " << d->packageRoot;
    }
    return packageRoot;
}

void PlasmaPkg::listPackages(const QStringList& types)
{
    QStringList list = d->packages(types);
    list.sort();
    foreach (const QString& package, list) {
        d->coutput(package);
    }
    exit(0);
}

void PlasmaPkgPrivate::renderTypeTable(const QMap<QString, QStringList> &plugins)
{
    const QString nameHeader = i18n("Addon Name");
    const QString pluginHeader = i18n("Service Type");
    const QString pathHeader = i18n("Path");
    int nameWidth = nameHeader.length();
    int pluginWidth = pluginHeader.length();
    int pathWidth = pathHeader.length();

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
    }

    std::cout << nameHeader.toLocal8Bit().constData() << std::setw(nameWidth - nameHeader.length() + 2) << ' '
              << pluginHeader.toLocal8Bit().constData() << std::setw(pluginWidth - pluginHeader.length() + 2) << ' '
              << pathHeader.toLocal8Bit().constData() << std::endl;
    std::cout << std::setfill('-') << std::setw(nameWidth) << '-' << "  "
              << std::setw(pluginWidth) << '-' << "  "
              << std::setw(pathWidth) << '-' << std::endl;
    std::cout << std::setfill(' ');

    pluginIt.toFront();
    while (pluginIt.hasNext()) {
        pluginIt.next();
        std::cout << pluginIt.key().toLocal8Bit().constData() << std::setw(nameWidth - pluginIt.key().length() + 2) << ' '
                  << pluginIt.value()[0].toLocal8Bit().constData() << std::setw(pluginWidth - pluginIt.value()[0].length() + 2) << ' '
                  << pluginIt.value()[1].toLocal8Bit().constData() << std::endl;
    }
}

void PlasmaPkgPrivate::listTypes()
{
    coutput(i18n("Package types that are installable with this tool:"));
    coutput(i18n("Built in:"));

    QMap<QString, QStringList> builtIns;
    builtIns.insert(i18n("DataEngine"), QStringList() << "Plasma/DataEngine" << "plasma/dataengines/");
    builtIns.insert(i18n("Layout Template"), QStringList() << "Plasma/LayoutTemplate" << "plasma/layout-templates/");
    builtIns.insert(i18n("Packages"), QStringList() << "Plasma/Generic" << "plasma/packages/");
    builtIns.insert(i18n("Plasmoid"), QStringList() << "Plasma/Applet" << "plasma/plasmoids/");
    builtIns.insert(i18n("Runner"), QStringList() << "Plasma/Runner" << "plasma/runners/");
    builtIns.insert(i18n("Theme"), QStringList() << "" << "desktoptheme/");
    builtIns.insert(i18n("Wallpaper Images"), QStringList() << "" << "wallpapers/");
    builtIns.insert(i18n("Wallpaper Plugin"), QStringList() << "Plasma/Wallpaper" << "plasma/wallpapers/");
    builtIns.insert(i18n("KWin Effect"), QStringList() << "KWin/Effect" << "kwin/effects/");
    builtIns.insert(i18n("KWin Window Switcher"), QStringList() << "KWin/WindowSwitcher" << "kwin/tabbox/");
    builtIns.insert(i18n("KWin Script"), QStringList() << "KWin/Script" << "kwin/scripts/");
    renderTypeTable(builtIns);

    KService::List offers;
    //if (KSycoca::isAvailable()) {
        offers = KServiceTypeTrader::self()->query("Plasma/PackageStructure");
    //}
    if (!offers.isEmpty()) {
        std::cout << std::endl;
        coutput(i18n("Provided by plugins:"));

        QMap<QString, QStringList> plugins;
        foreach (const KService::Ptr service, offers) {
            KPluginInfo info(service);
            //const QString proot = "";
            //Plasma::PackageStructure* structure = Plasma::PackageStructure::load(info.pluginName());
            QString name = info.name();
            QString comment = info.comment();
            QString plugin = info.pluginName();
            //QString path = structure->defaultPackageRoot();
            //QString path = defaultPackageRoot;
            plugins.insert(name, QStringList() << plugin);
            qDebug() << "KService stuff:" << name << plugin << comment;
        }

        renderTypeTable(plugins);
    }

    QStringList desktopFiles = QStandardPaths::locateAll(QStandardPaths::DataLocation, "plasma/packageformats/*rc", QStandardPaths::LocateFile);

    if (!desktopFiles.isEmpty()) {
        coutput(i18n("Provided by .desktop files:"));
        Plasma::PackageStructure structure;
        QMap<QString, QStringList> plugins;
        foreach (const QString &file, desktopFiles) {
            // extract the type
            KConfig config(file, KConfig::SimpleConfig);
            #warning "read config here"
            // structure.read(&config);
            // get the name based on the rc file name, just as Plasma::PackageStructure does
            const QString name = file.left(file.length() - 2);
            //plugins.insert(name, QStringList() << structure.type() << structure.defaultPackageRoot());
        }
    }
}

void PlasmaPkg::packageInstalled(KJob *job)
{
    bool success = (job->error() == KJob::NoError);
    int exitcode = 0;
    if (success) {
        if (d->args->isSet("upgrade")) {
            d->coutput(i18n("Successfully upgraded %1", d->packageFile));
        } else {
            d->coutput(i18n("Successfully installed %1", d->packageFile));
        }
    } else {
        d->coutput(i18n("Installation of %1 failed: %2", d->packageFile, job->errorText()));
        exitcode = 1;
    }
    exit(exitcode);
}

void PlasmaPkg::packageUninstalled(KJob *job)
{
    bool success = (job->error() == KJob::NoError);
    int exitcode = 0;
    if (success) {
        if (d->args->isSet("upgrade")) {
            d->coutput(i18n("Upgrading package from file: %1", d->packageFile));
            KJob *installJob = d->installer->install(d->packageFile, d->packageRoot);
            connect(installJob, SIGNAL(result(KJob*)), SLOT(packageInstalled(KJob*)));
            return;
        }
        d->coutput(i18n("Successfully uninstalled %1", d->packageFile));
    } else {
        d->coutput(i18n("Uninstallation of %1 failed: %2", d->packageFile, job->errorText()));
        exitcode = 1;
    }
    exit(exitcode);
}

} // namespace Plasma

#include "moc_plasmapkg.cpp"

