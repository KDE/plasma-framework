/*
 *   Copyright 2010 Ryan Rix <ry@n.rix.si>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
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

#include "pluginloader.h"

#include <QStandardPaths>

#include <QDebug>
#include <kservice.h>
#include <kservicetypetrader.h>
#include <kplugintrader.h>
#include <KPluginLoader>
#include <kpackage/packageloader.h>

#include "config-plasma.h"

#if !PLASMA_NO_KIO
#include <kio/job.h>
#endif

#include "applet.h"
#include "containment.h"
#include "containmentactions.h"
#include "dataengine.h"
#include "package.h"
#include "private/applet_p.h"
#include "private/packages_p.h"
#include "private/service_p.h" // for NullService
#include "private/storage_p.h"
#include "private/package_p.h"
#include "private/packagestructure_p.h"

namespace Plasma
{

static PluginLoader *s_pluginLoader = 0;

class PluginLoaderPrivate
{
public:
    PluginLoaderPrivate()
        : isDefaultLoader(false),
          packageRE("[^a-zA-Z0-9\\-_]")
    {
        KPackage::PackageLoader::self()->addKnownPackageStructure("Plasma/Applet", new PlasmoidPackage());
        KPackage::PackageLoader::self()->addKnownPackageStructure("Plasma/DataEngine", new DataEnginePackage());
        KPackage::PackageLoader::self()->addKnownPackageStructure("Plasma/Theme", new ThemePackage());
        KPackage::PackageLoader::self()->addKnownPackageStructure("Plasma/ContainmentActions", new ContainmentActionsPackage());
        KPackage::PackageLoader::self()->addKnownPackageStructure("Plasma/Generic", new GenericPackage());
    }

    static QSet<QString> knownCategories();
    static QString parentAppConstraint(const QString &parentApp = QString());

    static QSet<QString> s_customCategories;
    QHash<QString, QWeakPointer<PackageStructure> > structures;
    bool isDefaultLoader;

    static QString s_dataEnginePluginDir;
    static QString s_packageStructurePluginDir;
    static QString s_plasmoidsPluginDir;
    static QString s_servicesPluginDir;
    static QString s_containmentActionsPluginDir;
    QRegExp packageRE;
};

QSet<QString> PluginLoaderPrivate::s_customCategories;

QString PluginLoaderPrivate::s_dataEnginePluginDir("plasma/dataengine");
QString PluginLoaderPrivate::s_packageStructurePluginDir("plasma/packagestructure");
QString PluginLoaderPrivate::s_plasmoidsPluginDir("plasma/applets");
QString PluginLoaderPrivate::s_servicesPluginDir("plasma/services");
QString PluginLoaderPrivate::s_containmentActionsPluginDir("plasma/containmentactions");

QSet<QString> PluginLoaderPrivate::knownCategories()
{
    // this is to trick the tranlsation tools into making the correct
    // strings for translation
    QSet<QString> categories = s_customCategories;
    categories << QString(I18N_NOOP("Accessibility")).toLower()
               << QString(I18N_NOOP("Application Launchers")).toLower()
               << QString(I18N_NOOP("Astronomy")).toLower()
               << QString(I18N_NOOP("Date and Time")).toLower()
               << QString(I18N_NOOP("Development Tools")).toLower()
               << QString(I18N_NOOP("Education")).toLower()
               << QString(I18N_NOOP("Environment and Weather")).toLower()
               << QString(I18N_NOOP("Examples")).toLower()
               << QString(I18N_NOOP("File System")).toLower()
               << QString(I18N_NOOP("Fun and Games")).toLower()
               << QString(I18N_NOOP("Graphics")).toLower()
               << QString(I18N_NOOP("Language")).toLower()
               << QString(I18N_NOOP("Mapping")).toLower()
               << QString(I18N_NOOP("Miscellaneous")).toLower()
               << QString(I18N_NOOP("Multimedia")).toLower()
               << QString(I18N_NOOP("Online Services")).toLower()
               << QString(I18N_NOOP("Productivity")).toLower()
               << QString(I18N_NOOP("System Information")).toLower()
               << QString(I18N_NOOP("Utilities")).toLower()
               << QString(I18N_NOOP("Windows and Tasks")).toLower();
    return categories;
}

QString PluginLoaderPrivate::parentAppConstraint(const QString &parentApp)
{
    if (parentApp.isEmpty()) {
        QCoreApplication *app = QCoreApplication::instance();
        if (!app) {
            return QString();
        }

        return QString("((not exist [X-KDE-ParentApp] or [X-KDE-ParentApp] == '') or [X-KDE-ParentApp] == '%1')")
               .arg(app->applicationName());
    }

    return QString("[X-KDE-ParentApp] == '%1'").arg(parentApp);
}

PluginLoader::PluginLoader()
    : d(new PluginLoaderPrivate)
{
}

PluginLoader::~PluginLoader()
{
    typedef QWeakPointer<PackageStructure> pswp;
    foreach (pswp wp, d->structures) {
        delete wp.data();
    }
    delete d;
}

void PluginLoader::setPluginLoader(PluginLoader *loader)
{
    if (!s_pluginLoader) {
        s_pluginLoader = loader;
    } else {
#ifndef NDEBUG
        // qDebug() << "Cannot set pluginLoader, already set!" << s_pluginLoader;
#endif
    }
}

PluginLoader *PluginLoader::self()
{
    if (!s_pluginLoader) {
        // we have been called before any PluginLoader was set, so just use the default
        // implementation. this prevents plugins from nefariously injecting their own
        // plugin loader if the app doesn't
        s_pluginLoader = new PluginLoader;
        s_pluginLoader->d->isDefaultLoader = true;
    }

    return s_pluginLoader;
}

Applet *PluginLoader::loadApplet(const QString &name, uint appletId, const QVariantList &args)
{
    if (name.isEmpty()) {
        return 0;
    }

    Applet *applet = d->isDefaultLoader ? 0 : internalLoadApplet(name, appletId, args);
    if (applet) {
        return applet;
    }

    if (appletId == 0) {
        appletId = ++AppletPrivate::s_maxAppletId;
    }



    // Look for C++ plugins first
    auto filter = [&name](const KPluginMetaData &md) -> bool
    {
        return md.pluginId() == name;
    };
    QVector<KPluginMetaData> plugins = KPluginLoader::findPlugins(PluginLoaderPrivate::s_plasmoidsPluginDir, filter);

    if (plugins.count()) {
        KPluginInfo::List lst = KPluginInfo::fromMetaData(plugins);
        KPluginLoader loader(lst.first().libraryPath());
        if (!Plasma::isPluginVersionCompatible(loader.pluginVersion())) {
            return 0;
        }
        KPluginFactory *factory = loader.factory();
        if (factory) {
            QVariantList allArgs;
            allArgs << loader.metaData().toVariantMap() << appletId << args;
            applet = factory->create<Plasma::Applet>(0, allArgs);
        }
    }
    if (applet) {
        return applet;
    }


    KPackage::Package p = KPackage::PackageLoader::self()->loadPackage("Plasma/Applet", name);
    if (!p.isValid()) {
        //some applets have actually the root path from another package, such as icontasks
        //try to do a fallback package with X-Plasma-RootPath root
        p.setRequired("mainscript", false);
        p.setPath(name);

        KPluginMetaData md(p.filePath("metadata"));
        const KPackage::Package fp = KPackage::PackageLoader::self()->loadPackage("Plasma/Applet", md.value("X-Plasma-RootPath"));
        p.setFallbackPackage(fp);

        if (!fp.isValid()) {
            return 0;
        }
    }

    // backwards compatibility: search in the root plugins directory
    // TODO: remove when Plasma 5.4 is released
    {
        KPluginInfo info = KPluginInfo::fromMetaData(p.metadata());
        KPluginLoader loader(info.libraryPath());
        if (!Plasma::isPluginVersionCompatible(loader.pluginVersion())) {
            return 0;
        }
        KPluginFactory *factory = loader.factory();
        if (factory) {
            QVariantList allArgs;
            allArgs << loader.metaData().toVariantMap() << appletId << args;
            applet = factory->create<Plasma::Applet>(0, allArgs);
        }
        if (applet) {
            return applet;
        }
    }


    if (!applet) {
        qWarning() << "Could not load applet" << name << "Falling back to an empty one";

        QVariantList allArgs;
        allArgs << p.metadata().fileName() << appletId << args;

        if (p.metadata().serviceTypes().contains("Plasma/Containment")) {
            return new Containment(0, allArgs);
        } else {
            return new Applet(0, allArgs);
        }

    }

    return applet;
}

DataEngine *PluginLoader::loadDataEngine(const QString &name)
{
    DataEngine *engine = d->isDefaultLoader ? 0 : internalLoadDataEngine(name);
    if (engine) {
        return engine;
    }

    // Look for C++ plugins first
    auto filter = [&name](const KPluginMetaData &md) -> bool
    {
        return md.pluginId() == name;
    };
    QVector<KPluginMetaData> plugins = KPluginLoader::findPlugins(PluginLoaderPrivate::s_dataEnginePluginDir, filter);

    if (plugins.count()) {
        KPluginInfo::List lst = KPluginInfo::fromMetaData(plugins);
        KPluginLoader loader(lst.first().libraryPath());
        const QVariantList argsWithMetaData = QVariantList() << loader.metaData().toVariantMap();
        KPluginFactory *factory = loader.factory();
        if (factory) {
            engine = factory->create<Plasma::DataEngine>(0, argsWithMetaData);
        }
    }
    if (engine) {
        return engine;
    }

    const KPackage::Package p = KPackage::PackageLoader::self()->loadPackage("Plasma/Applet", name);
    if (!p.isValid()) {
        return 0;
    }

    return new DataEngine(KPluginInfo(p.metadata().fileName()), 0);
}

QStringList PluginLoader::listAllEngines(const QString &parentApp)
{
    //HACK: we actually need an instance for this to work correctly
    self();

    QStringList engines;
    // Look for C++ plugins first
    auto filter = [&parentApp](const KPluginMetaData &md) -> bool
    {
        return md.value("X-KDE-ParentApp") == parentApp;
    };
    QVector<KPluginMetaData> plugins;
    if (parentApp.isEmpty()) {
        plugins = KPluginLoader::findPlugins(PluginLoaderPrivate::s_dataEnginePluginDir);
    } else {
        plugins = KPluginLoader::findPlugins(PluginLoaderPrivate::s_dataEnginePluginDir, filter);
    }

    for (auto plugin : plugins) {
        engines << plugin.pluginId();
    }

    const QList<KPluginMetaData> packagePlugins = KPackage::PackageLoader::self()->listPackages("Plasma/DataEngine");
    for (auto plugin : packagePlugins) {
        engines << plugin.pluginId();
    }

    return engines;
}

KPluginInfo::List PluginLoader::listEngineInfo(const QString &parentApp)
{
    //HACK: we actually need an instance for this to work correctly
    self();
    return PluginLoader::self()->listDataEngineInfo(parentApp);
}

KPluginInfo::List PluginLoader::listEngineInfoByCategory(const QString &category, const QString &parentApp)
{
    //HACK: we actually need an instance for this to work correctly
    self();

    KPluginInfo::List list;

    // Look for C++ plugins first
    auto filterNormal = [&category](const KPluginMetaData &md) -> bool
    {
        return md.value("X-KDE-PluginInfo-Category") == category;
    };
    auto filterParentApp = [&category, &parentApp](const KPluginMetaData &md) -> bool
    {
        return md.value("X-KDE-ParentApp") == parentApp && md.value("X-KDE-PluginInfo-Category") == category;
    };
    QVector<KPluginMetaData> plugins;
    if (parentApp.isEmpty()) {
        plugins = KPluginLoader::findPlugins(PluginLoaderPrivate::s_dataEnginePluginDir, filterNormal);
    } else {
        plugins = KPluginLoader::findPlugins(PluginLoaderPrivate::s_dataEnginePluginDir, filterParentApp);
    }

    list = KPluginInfo::fromMetaData(plugins);


    //TODO FIXME: PackageLoader needs to have a function to inject packageStructures
    const QList<KPluginMetaData> packagePlugins = KPackage::PackageLoader::self()->listPackages("Plasma/DataEngine");
    list << KPluginInfo::fromMetaData(packagePlugins.toVector());

    return list;
}

Service *PluginLoader::loadService(const QString &name, const QVariantList &args, QObject *parent)
{
    Service *service = d->isDefaultLoader ? 0 : internalLoadService(name, args, parent);
    if (service) {
        return service;
    }

    //TODO: scripting API support
    if (name.isEmpty()) {
        return new NullService(QString(), parent);
    } else if (name == "org.kde.servicestorage") {
        return new Storage(parent);
    }


    // Look for C++ plugins first
    auto filter = [&name](const KPluginMetaData &md) -> bool
    {
        return md.pluginId() == name;
    };
    QVector<KPluginMetaData> plugins = KPluginLoader::findPlugins(PluginLoaderPrivate::s_servicesPluginDir, filter);

    if (plugins.count()) {
        KPluginInfo::List lst = KPluginInfo::fromMetaData(plugins);
        KPluginLoader loader(lst.first().libraryPath());
        if (!Plasma::isPluginVersionCompatible(loader.pluginVersion())) {
            return 0;
        }
        KPluginFactory *factory = loader.factory();
        if (factory) {
            service = factory->create<Plasma::Service>(0, args);
        }
    }

    if (service) {
        if (service->name().isEmpty()) {
            service->setName(name);
        }
        return service;
    } else {
        return new NullService(name, parent);
    }
}

ContainmentActions *PluginLoader::loadContainmentActions(Containment *parent, const QString &name, const QVariantList &args)
{
    if (name.isEmpty()) {
        return 0;
    }

    ContainmentActions *actions = d->isDefaultLoader ? 0 : internalLoadContainmentActions(parent, name, args);
    if (actions) {
        return actions;
    }


    // Look for C++ plugins first
    auto filter = [&name](const KPluginMetaData &md) -> bool
    {
        return md.pluginId() == name;
    };
    QVector<KPluginMetaData> plugins = KPluginLoader::findPlugins(PluginLoaderPrivate::s_containmentActionsPluginDir, filter);

    if (plugins.count()) {
        KPluginInfo::List lst = KPluginInfo::fromMetaData(plugins);
        KPluginLoader loader(lst.first().libraryPath());
        const QVariantList argsWithMetaData = QVariantList() << loader.metaData().toVariantMap();
        KPluginFactory *factory = loader.factory();
        if (factory) {
            actions = factory->create<Plasma::ContainmentActions>(0, argsWithMetaData);
        }
    }
    if (actions) {
        return actions;
    }

    //FIXME: this is only for backwards compatibility, but probably will have to stay
    //for the time being
    QString constraint = QString("[X-KDE-PluginInfo-Name] == '%1'").arg(name);
    KService::List offers = KServiceTypeTrader::self()->query("Plasma/ContainmentActions", constraint);

    if (offers.isEmpty()) {
#ifndef NDEBUG
        qDebug() << "offers is empty for " << name;
#endif
        return 0;
    }

    KService::Ptr offer = offers.first();
    KPluginLoader plugin(*offer);

    if (!Plasma::isPluginVersionCompatible(plugin.pluginVersion())) {
        return 0;
    }

    QVariantList allArgs;
    allArgs << offer->storageId() << args;
    QString error;
    actions = offer->createInstance<Plasma::ContainmentActions>(parent, allArgs, &error);

    if (!actions) {
#ifndef NDEBUG
        // qDebug() << "Couldn't load containmentActions \"" << name << "\"! reason given: " << error;
#endif
    }

    return actions;
}

Package PluginLoader::loadPackage(const QString &packageFormat, const QString &specialization)
{
    if (!d->isDefaultLoader) {
        Package p = internalLoadPackage(packageFormat, specialization);
        if (p.hasValidStructure()) {
            return p;
        }
    }

    if (packageFormat.isEmpty()) {
        return Package();
    }

    const QString hashkey = packageFormat + '%' + specialization;
    PackageStructure *structure = d->structures.value(hashkey).data();

    KPackage::PackageStructure *internalStructure = 0;
    if (structure) {
        return Package(structure);
    }

    if (packageFormat.startsWith("Plasma")) {
        if (packageFormat.endsWith("/Applet")) {
            internalStructure = new PlasmoidPackage();
        } else if (packageFormat.endsWith("/DataEngine")) {
            internalStructure = new DataEnginePackage();
        } else if (packageFormat.endsWith("/Theme")) {
            internalStructure = new ThemePackage();
        } else if (packageFormat.endsWith("/ContainmentActions")) {
            internalStructure = new ContainmentActionsPackage();
        } else if (packageFormat.endsWith("/Generic")) {
            internalStructure = new GenericPackage();
        }

        if (internalStructure) {
            structure = new PackageStructure();
            structure->d->internalStructure = internalStructure;
            d->structures.insert(hashkey, structure);
            return Package(structure);
        }
    }

    internalStructure = KPackage::PackageLoader::self()->loadPackageStructure(packageFormat);

    if (internalStructure) {
        structure = new PackageStructure();
        structure->d->internalStructure = internalStructure;
    //fallback to old structures
    } else {
        const QString constraint = QString("[X-KDE-PluginInfo-Name] == '%1'").arg(packageFormat);
        structure = KPluginTrader::createInstanceFromQuery<Plasma::PackageStructure>(PluginLoaderPrivate::s_packageStructurePluginDir, "Plasma/PackageStructure", constraint, 0);
        if (structure) {
            structure->d->internalStructure = new PackageStructureWrapper(structure);
        }
    }


    if (structure) {
        d->structures.insert(hashkey, structure);
        return Package(structure);
    }

#ifndef NDEBUG
        // qDebug() << "Couldn't load Package for" << packageFormat << "! reason given: " << error;
#endif

    return Package();
}

KPluginInfo::List PluginLoader::listAppletInfo(const QString &category, const QString &parentApp)
{
    KPluginInfo::List list;

    if (!d->isDefaultLoader && (parentApp.isEmpty() || parentApp == QCoreApplication::instance()->applicationName())) {
        list = internalAppletInfo(category);
    }

    //FIXME: this assumes we are always use packages.. no pure c++
    if (category.isEmpty()) { //use all but the excluded categories
        KConfigGroup group(KSharedConfig::openConfig(), "General");
        QStringList excluded = group.readEntry("ExcludeCategories", QStringList());

        auto filter = [&excluded, &parentApp](const KPluginMetaData &md) -> bool
        {
            const QString pa = md.value("X-KDE-ParentApp");
            return (pa.isEmpty() || pa == parentApp) && !excluded.contains(md.category());
        };
        return KPluginInfo::fromMetaData(KPackage::PackageLoader::self()->findPackages("Plasma/Applet", QString(), filter).toVector());

    } else { //specific category (this could be an excluded one - is that bad?)

        auto filter = [&category, &parentApp](const KPluginMetaData &md) -> bool
        {
            const QString pa = md.value("X-KDE-ParentApp");
            if (category == "Miscellaneous") {
                return (pa.isEmpty() || pa == parentApp) && (md.category() == category || md.category().isEmpty());
            } else {
                return (pa.isEmpty() || pa == parentApp) && md.category() == category;
            }
        };
        return KPluginInfo::fromMetaData(KPackage::PackageLoader::self()->findPackages("Plasma/Applet", QString(), filter).toVector());
    }
}

KPluginInfo::List PluginLoader::listAppletInfoForMimeType(const QString &mimeType)
{
    auto filter = [&mimeType](const KPluginMetaData &md) -> bool
    {
        return md.value("X-Plasma-DropMimeTypes").contains(mimeType);
    };
    return KPluginInfo::fromMetaData(KPackage::PackageLoader::self()->findPackages("Plasma/Applet", QString(), filter).toVector());
}

KPluginInfo::List PluginLoader::listAppletInfoForUrl(const QUrl &url)
{
    QString parentApp;
    QCoreApplication *app = QCoreApplication::instance();
    if (app) {
        parentApp = app->applicationName();
    }

    auto filter = [&parentApp](const KPluginMetaData &md) -> bool
    {
        const QString pa = md.value("X-KDE-ParentApp");
        return (pa.isEmpty() || pa == parentApp) && !md.value("X-Plasma-DropUrlPatterns").isEmpty();
    };
    KPluginInfo::List allApplets =  KPluginInfo::fromMetaData(KPackage::PackageLoader::self()->findPackages("Plasma/Applet", QString(), filter).toVector());

    KPluginInfo::List filtered;
    foreach (const KPluginInfo &info, allApplets) {
        QStringList urlPatterns = info.property("X-Plasma-DropUrlPatterns").toStringList();
        foreach (const QString &glob, urlPatterns) {
            QRegExp rx(glob);
            rx.setPatternSyntax(QRegExp::Wildcard);
            if (rx.exactMatch(url.toString())) {
#ifndef NDEBUG
                // qDebug() << info.name() << "matches" << glob << url;
#endif
                filtered << info;
            }
        }
    }

    return filtered;
}

QStringList PluginLoader::listAppletCategories(const QString &parentApp, bool visibleOnly)
{
    KConfigGroup group(KSharedConfig::openConfig(), "General");
    const QStringList excluded = group.readEntry("ExcludeCategories", QStringList());
    auto filter = [&parentApp, &excluded, visibleOnly](const KPluginMetaData &md) -> bool
    {
        const QString pa = md.value("X-KDE-ParentApp");
        return (pa.isEmpty() || pa == parentApp) && (excluded.isEmpty() || excluded.contains(md.value("X-KDE-PluginInfo-Category"))) && (!visibleOnly || !md.isHidden());
    };
    QList<KPluginMetaData> allApplets = KPackage::PackageLoader::self()->findPackages("Plasma/Applet", QString(), filter);


    QStringList categories;
    for (auto plugin : allApplets) {
        if (plugin.category().isEmpty()) {
            if (!categories.contains(i18nc("misc category", "Miscellaneous"))) {
                categories << i18nc("misc category", "Miscellaneous");
            }
        } else {
            categories << plugin.category();
        }
    }
    return categories;
}

void PluginLoader::setCustomAppletCategories(const QStringList &categories)
{
    PluginLoaderPrivate::s_customCategories = QSet<QString>::fromList(categories);
}

QStringList PluginLoader::customAppletCategories() const
{
    return PluginLoaderPrivate::s_customCategories.toList();
}

QString PluginLoader::appletCategory(const QString &appletName)
{
    if (appletName.isEmpty()) {
        return QString();
    }

    const KPackage::Package p = KPackage::PackageLoader::self()->loadPackage("Plasma/Applet", appletName);
    if (!p.isValid()) {
        return QString();
    }

    return p.metadata().category();
}

KPluginInfo::List PluginLoader::listContainments(const QString &category,
        const QString &parentApp)
{
    return listContainmentsOfType(QString(), category, parentApp);
}

KPluginInfo::List PluginLoader::listContainmentsOfType(const QString &type,
        const QString &category,
        const QString &parentApp)
{
    //HACK: we actually need an instance for this to work correctly
    self();

    KConfigGroup group(KSharedConfig::openConfig(), "General");
    const QStringList excluded = group.readEntry("ExcludeCategories", QStringList());
    auto filter = [&type, &category, &parentApp](const KPluginMetaData &md) -> bool
    {
        if (!md.serviceTypes().contains("Plasma/Containment")) {
            return false;
        }
        const QString pa = md.value("X-KDE-ParentApp");
        if (!pa.isEmpty() && pa != parentApp) {
            return false;
        }

        if (!type.isEmpty() && md.value("X-Plasma-ContainmentType") != type) {
            return false;
        }

        if (!category.isEmpty() && md.value("X-KDE-PluginInfo-Category") != category) {
            return false;
        }

        return true;
    };

    return KPluginInfo::fromMetaData(KPackage::PackageLoader::self()->findPackages("Plasma/Applet", QString(), filter).toVector());
}

KPluginInfo::List PluginLoader::listContainmentsForMimeType(const QString &mimeType)
{
    //HACK: we actually need an instance for this to work correctly
    self();
    auto filter = [&mimeType](const KPluginMetaData &md) -> bool
    {
        return md.value("X-KDE-ServiceTypes").contains("Plasma/Containment") && md.value("X-Plasma-DropMimeTypes").contains(mimeType);
    };

    return KPluginInfo::fromMetaData(KPackage::PackageLoader::self()->findPackages("Plasma/Applet", QString(), filter).toVector());
}

QStringList PluginLoader::listContainmentTypes()
{
    //HACK: we actually need an instance for this to work correctly
    self();
    KPluginInfo::List containmentInfos = listContainments();
    QSet<QString> types;

    foreach (const KPluginInfo &containmentInfo, containmentInfos) {
        QStringList theseTypes = containmentInfo.service()->property("X-Plasma-ContainmentType").toStringList();
        foreach (const QString &type, theseTypes) {
            types.insert(type);
        }
    }

    return types.toList();
}

KPluginInfo::List PluginLoader::listDataEngineInfo(const QString &parentApp)
{
    KPluginInfo::List list;

    if (!d->isDefaultLoader && (parentApp.isEmpty() || parentApp == QCoreApplication::instance()->applicationName())) {
        list = internalDataEngineInfo();
    }

    QString constraint;
    if (parentApp.isEmpty()) {
        constraint.append("not exist [X-KDE-ParentApp]");
    } else {
        constraint.append("[X-KDE-ParentApp] == '").append(parentApp).append("'");
    }

    list.append(KPluginTrader::self()->query(PluginLoaderPrivate::s_dataEnginePluginDir, "Plasma/DataEngine", constraint));
    return list;
}

KPluginInfo::List PluginLoader::listContainmentActionsInfo(const QString &parentApp)
{
    KPluginInfo::List list;

    if (!d->isDefaultLoader && (parentApp.isEmpty() || parentApp == QCoreApplication::instance()->applicationName())) {
        list = internalContainmentActionsInfo();
    }

    QString constraint;
    if (parentApp.isEmpty()) {
        constraint.append("not exist [X-KDE-ParentApp]");
    } else {
        constraint.append("[X-KDE-ParentApp] == '").append(parentApp).append("'");
    }

    list.append(KPluginTrader::self()->query(PluginLoaderPrivate::s_containmentActionsPluginDir, "Plasma/ContainmentActions", constraint));

    //FIXME: this is only for backwards compatibility, but probably will have to stay
    //for the time being
    KService::List offers = KServiceTypeTrader::self()->query("Plasma/ContainmentActions", constraint);
    list.append(KPluginInfo::fromServices(offers));
    return list;
}

Applet *PluginLoader::internalLoadApplet(const QString &name, uint appletId, const QVariantList &args)
{
    Q_UNUSED(name)
    Q_UNUSED(appletId)
    Q_UNUSED(args)
    return 0;
}

DataEngine *PluginLoader::internalLoadDataEngine(const QString &name)
{
    Q_UNUSED(name)
    return 0;
}

ContainmentActions *PluginLoader::internalLoadContainmentActions(Containment *containment, const QString &name, const QVariantList &args)
{
    Q_UNUSED(containment)
    Q_UNUSED(name)
    Q_UNUSED(args)
    return 0;
}

Service *PluginLoader::internalLoadService(const QString &name, const QVariantList &args, QObject *parent)
{
    Q_UNUSED(name)
    Q_UNUSED(args)
    Q_UNUSED(parent)
    return 0;
}

Package PluginLoader::internalLoadPackage(const QString &name, const QString &specialization)
{
    Q_UNUSED(name);
    Q_UNUSED(specialization);
    return Package();
}

KPluginInfo::List PluginLoader::internalAppletInfo(const QString &category) const
{
    Q_UNUSED(category)
    return KPluginInfo::List();
}

KPluginInfo::List PluginLoader::internalDataEngineInfo() const
{
    return KPluginInfo::List();
}

KPluginInfo::List PluginLoader::internalServiceInfo() const
{
    return KPluginInfo::List();
}

KPluginInfo::List PluginLoader::internalContainmentActionsInfo() const
{
    return KPluginInfo::List();
}

static KPluginInfo::List standardInternalInfo(const QString &type, const QString &category = QString())
{
    QStringList files = QStandardPaths::locateAll(QStandardPaths::GenericDataLocation,
                        PLASMA_RELATIVE_DATA_INSTALL_DIR "/internal/" + type + "/*.desktop",
                        QStandardPaths::LocateFile);

    KPluginInfo::List allInfo = KPluginInfo::fromFiles(files);

    if (category.isEmpty() || allInfo.isEmpty()) {
        return allInfo;
    }

    KPluginInfo::List matchingInfo;
    foreach (const KPluginInfo &info, allInfo) {
        if (info.category().compare(category, Qt::CaseInsensitive) == 0) {
            matchingInfo << info;
        }
    }

    return matchingInfo;
}

KPluginInfo::List PluginLoader::standardInternalAppletInfo(const QString &category) const
{
    return standardInternalInfo("applets", category);
}

KPluginInfo::List PluginLoader::standardInternalDataEngineInfo() const
{
    return standardInternalInfo("dataengines");
}

KPluginInfo::List PluginLoader::standardInternalServiceInfo() const
{
    return standardInternalInfo("services");
}

} // Plasma Namespace

