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
#include "private/service_p.h" // for NullService
#include "private/storage_p.h"
#include "private/package_p.h"
#include "private/packagestructure_p.h"
#include <plasma/version.h>
#include "debug_p.h"

namespace Plasma
{

static PluginLoader *s_pluginLoader = 0;

class PluginLoaderPrivate
{
public:
    PluginLoaderPrivate()
        : isDefaultLoader(false)
    {
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
};

QSet<QString> PluginLoaderPrivate::s_customCategories;

QString PluginLoaderPrivate::s_dataEnginePluginDir = QStringLiteral("plasma/dataengine");
QString PluginLoaderPrivate::s_packageStructurePluginDir = QStringLiteral("plasma/packagestructure");
QString PluginLoaderPrivate::s_plasmoidsPluginDir = QStringLiteral("plasma/applets");
QString PluginLoaderPrivate::s_servicesPluginDir = QStringLiteral("plasma/services");
QString PluginLoaderPrivate::s_containmentActionsPluginDir = QStringLiteral("plasma/containmentactions");

QSet<QString> PluginLoaderPrivate::knownCategories()
{
    // this is to trick the tranlsation tools into making the correct
    // strings for translation
    QSet<QString> categories = s_customCategories;
    categories << QStringLiteral(I18N_NOOP("Accessibility")).toLower()
               << QStringLiteral(I18N_NOOP("Application Launchers")).toLower()
               << QStringLiteral(I18N_NOOP("Astronomy")).toLower()
               << QStringLiteral(I18N_NOOP("Date and Time")).toLower()
               << QStringLiteral(I18N_NOOP("Development Tools")).toLower()
               << QStringLiteral(I18N_NOOP("Education")).toLower()
               << QStringLiteral(I18N_NOOP("Environment and Weather")).toLower()
               << QStringLiteral(I18N_NOOP("Examples")).toLower()
               << QStringLiteral(I18N_NOOP("File System")).toLower()
               << QStringLiteral(I18N_NOOP("Fun and Games")).toLower()
               << QStringLiteral(I18N_NOOP("Graphics")).toLower()
               << QStringLiteral(I18N_NOOP("Language")).toLower()
               << QStringLiteral(I18N_NOOP("Mapping")).toLower()
               << QStringLiteral(I18N_NOOP("Miscellaneous")).toLower()
               << QStringLiteral(I18N_NOOP("Multimedia")).toLower()
               << QStringLiteral(I18N_NOOP("Online Services")).toLower()
               << QStringLiteral(I18N_NOOP("Productivity")).toLower()
               << QStringLiteral(I18N_NOOP("System Information")).toLower()
               << QStringLiteral(I18N_NOOP("Utilities")).toLower()
               << QStringLiteral(I18N_NOOP("Windows and Tasks")).toLower()
               << QStringLiteral(I18N_NOOP("Clipboard")).toLower()
               << QStringLiteral(I18N_NOOP("Tasks")).toLower();
    return categories;
}

QString PluginLoaderPrivate::parentAppConstraint(const QString &parentApp)
{
    if (parentApp.isEmpty()) {
        QCoreApplication *app = QCoreApplication::instance();
        if (!app) {
            return QString();
        }

        return QStringLiteral("((not exist [X-KDE-ParentApp] or [X-KDE-ParentApp] == '') or [X-KDE-ParentApp] == '%1')")
               .arg(app->applicationName());
    }

    return QStringLiteral("[X-KDE-ParentApp] == '%1'").arg(parentApp);
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
        // qCDebug(LOG_PLASMA) << "Cannot set pluginLoader, already set!" << s_pluginLoader;
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


    //if name wasn't a path, pluginName == name
    const QString pluginName = name.split('/').last();

    // Look for C++ plugins first
    auto filter = [&pluginName](const KPluginMetaData &md) -> bool
    {
        return md.pluginId() == pluginName;
    };
    QVector<KPluginMetaData> plugins = KPluginLoader::findPlugins(PluginLoaderPrivate::s_plasmoidsPluginDir, filter);
    if (plugins.isEmpty()) {
        // COMPAT CODE for applets installed into the toplevel plugins dir by mistake.
        plugins = KPluginLoader::findPlugins(QString(), filter);
    }

    if (!plugins.isEmpty()) {
        KPluginLoader loader(plugins.first().fileName());
        if (!isPluginVersionCompatible(loader)) {
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


    const KPackage::Package p = KPackage::PackageLoader::self()->loadPackage(QStringLiteral("Plasma/Applet"), name);
    if (!applet) {
        //qCDebug(LOG_PLASMA) << name << "not a C++ applet: Falling back to an empty one";

        QVariantList allArgs;
        allArgs << p.metadata().fileName() << appletId << args;

        if (p.metadata().serviceTypes().contains(QStringLiteral("Plasma/Containment"))) {
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

    if (!plugins.isEmpty()) {
        KPluginLoader loader(plugins.first().fileName());
        const QVariantList argsWithMetaData = QVariantList() << loader.metaData().toVariantMap();
        KPluginFactory *factory = loader.factory();
        if (factory) {
            engine = factory->create<Plasma::DataEngine>(0, argsWithMetaData);
        }
    }
    if (engine) {
        return engine;
    }

    const KPackage::Package p = KPackage::PackageLoader::self()->loadPackage(QStringLiteral("Plasma/DataEngine"), name);
    if (!p.isValid()) {
        return 0;
    }

    return new DataEngine(KPluginInfo(p.metadata().fileName()), 0);
}

QStringList PluginLoader::listAllEngines(const QString &parentApp)
{
    QStringList engines;
    // Look for C++ plugins first
    auto filter = [&parentApp](const KPluginMetaData &md) -> bool
    {
        return md.value(QStringLiteral("X-KDE-ParentApp")) == parentApp;
    };
    QVector<KPluginMetaData> plugins;
    if (parentApp.isEmpty()) {
        plugins = KPluginLoader::findPlugins(PluginLoaderPrivate::s_dataEnginePluginDir);
    } else {
        plugins = KPluginLoader::findPlugins(PluginLoaderPrivate::s_dataEnginePluginDir, filter);
    }

    foreach (auto& plugin, plugins) {
        engines << plugin.pluginId();
    }

    const QList<KPluginMetaData> packagePlugins = KPackage::PackageLoader::self()->listPackages(QStringLiteral("Plasma/DataEngine"));
    for (auto& plugin : packagePlugins) {
        engines << plugin.pluginId();
    }

    return engines;
}

KPluginInfo::List PluginLoader::listEngineInfo(const QString &parentApp)
{
    return PluginLoader::self()->listDataEngineInfo(parentApp);
}

KPluginInfo::List PluginLoader::listEngineInfoByCategory(const QString &category, const QString &parentApp)
{
    KPluginInfo::List list;

    // Look for C++ plugins first
    auto filterNormal = [&category](const KPluginMetaData &md) -> bool
    {
        return md.value(QStringLiteral("X-KDE-PluginInfo-Category")) == category;
    };
    auto filterParentApp = [&category, &parentApp](const KPluginMetaData &md) -> bool
    {
        return md.value(QStringLiteral("X-KDE-ParentApp")) == parentApp
            && md.value(QStringLiteral("X-KDE-PluginInfo-Category")) == category;
    };
    QVector<KPluginMetaData> plugins;
    if (parentApp.isEmpty()) {
        plugins = KPluginLoader::findPlugins(PluginLoaderPrivate::s_dataEnginePluginDir, filterNormal);
    } else {
        plugins = KPluginLoader::findPlugins(PluginLoaderPrivate::s_dataEnginePluginDir, filterParentApp);
    }

    list = KPluginInfo::fromMetaData(plugins);


    //TODO FIXME: PackageLoader needs to have a function to inject packageStructures
    const QList<KPluginMetaData> packagePlugins = KPackage::PackageLoader::self()->listPackages(QStringLiteral("Plasma/DataEngine"));
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
    } else if (name == QLatin1String("org.kde.servicestorage")) {
        return new Storage(parent);
    }


    // Look for C++ plugins first
    auto filter = [&name](const KPluginMetaData &md) -> bool
    {
        return md.pluginId() == name;
    };
    QVector<KPluginMetaData> plugins = KPluginLoader::findPlugins(PluginLoaderPrivate::s_servicesPluginDir, filter);

    if (!plugins.isEmpty()) {
        KPluginLoader loader(plugins.first().fileName());
        if (!isPluginVersionCompatible(loader)) {
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

    if (!plugins.isEmpty()) {
        KPluginLoader loader(plugins.first().fileName());
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
    QString constraint = QStringLiteral("[X-KDE-PluginInfo-Name] == '%1'").arg(name);
    KService::List offers = KServiceTypeTrader::self()->query(QStringLiteral("Plasma/ContainmentActions"), constraint);

    if (offers.isEmpty()) {
#ifndef NDEBUG
        qCDebug(LOG_PLASMA) << "offers is empty for " << name;
#endif
        return 0;
    }

    KService::Ptr offer = offers.first();
    KPluginLoader plugin(*offer);

    if (!isPluginVersionCompatible(plugin)) {
        return 0;
    }

    QVariantList allArgs;
    allArgs << offer->storageId() << args;
    QString error;
    actions = offer->createInstance<Plasma::ContainmentActions>(parent, allArgs, &error);

    if (!actions) {
#ifndef NDEBUG
        // qCDebug(LOG_PLASMA) << "Couldn't load containmentActions \"" << name << "\"! reason given: " << error;
#endif
    }

    return actions;
}

#ifndef PLASMA_NO_DEPRECATED
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

    if (structure) {
        return Package(structure);
    }

    KPackage::PackageStructure *internalStructure = KPackage::PackageLoader::self()->loadPackageStructure(packageFormat);

    if (internalStructure) {
        structure = new PackageStructure();
        structure->d->internalStructure = internalStructure;
    //fallback to old structures
    } else {
        const QString constraint = QStringLiteral("[X-KDE-PluginInfo-Name] == '%1'").arg(packageFormat);
        structure = KPluginTrader::createInstanceFromQuery<Plasma::PackageStructure>(PluginLoaderPrivate::s_packageStructurePluginDir, QStringLiteral("Plasma/PackageStructure"), constraint, 0);
        if (structure) {
            structure->d->internalStructure = new PackageStructureWrapper(structure);
        }
    }


    if (structure) {
        d->structures.insert(hashkey, structure);
        return Package(structure);
    }

#ifndef NDEBUG
        // qCDebug(LOG_PLASMA) << "Couldn't load Package for" << packageFormat << "! reason given: " << error;
#endif

    return Package();
}
#endif

QList<KPluginMetaData> PluginLoader::listAppletMetaData(const QString &category, const QString &parentApp)
{
    //FIXME: this assumes we are always use packages.. no pure c++
    std::function<bool(const KPluginMetaData&)> filter;
    if (category.isEmpty()) { //use all but the excluded categories
        KConfigGroup group(KSharedConfig::openConfig(), "General");
        QStringList excluded = group.readEntry("ExcludeCategories", QStringList());

        filter = [excluded, parentApp](const KPluginMetaData &md) -> bool
        {
            const QString pa = md.value(QStringLiteral("X-KDE-ParentApp"));
            return (pa.isEmpty() || pa == parentApp) && !excluded.contains(md.category());
        };
    } else { //specific category (this could be an excluded one - is that bad?)

        filter = [category, parentApp](const KPluginMetaData &md) -> bool
        {
            const QString pa = md.value(QStringLiteral("X-KDE-ParentApp"));
            if (category == QLatin1String("Miscellaneous")) {
                return (pa.isEmpty() || pa == parentApp) && (md.category() == category || md.category().isEmpty());
            } else {
                return (pa.isEmpty() || pa == parentApp) && md.category() == category;
            }
        };
    }

    QList<KPluginMetaData> list;
    if (!d->isDefaultLoader && (parentApp.isEmpty() || parentApp == QCoreApplication::instance()->applicationName())) {
        list = KPluginInfo::toMetaData(internalAppletInfo(category)).toList();
    }
    return KPackage::PackageLoader::self()->findPackages(QStringLiteral("Plasma/Applet"), QString(), filter);
}

KPluginInfo::List PluginLoader::listAppletInfo(const QString &category, const QString &parentApp)
{
    KPluginInfo::List list;
    const auto plugins = listAppletMetaData(category, parentApp);

    //NOTE: it still produces kplugininfos from KServices because some user code expects
    //info.sevice() to be valid and would crash ohtherwise
    foreach (auto& md, plugins) {
        auto pi = md.metaDataFileName().endsWith(".json") ? KPluginInfo(md) : KPluginInfo(KService::serviceByStorageId(md.metaDataFileName()));
        if (!pi.isValid()) {
            qCWarning(LOG_PLASMA) << "Could not load plugin info for plugin :" << md.pluginId() << "skipping plugin";
            continue;
        }
        list << pi;
    }
    return list;
}

QList<KPluginMetaData> PluginLoader::listAppletMetaDataForMimeType(const QString &mimeType)
{
    auto filter = [&mimeType](const KPluginMetaData &md) -> bool
    {
        return KPluginMetaData::readStringList(md.rawData(), QStringLiteral("X-Plasma-DropMimeTypes")).contains(mimeType);
    };
    return KPackage::PackageLoader::self()->findPackages(QStringLiteral("Plasma/Applet"), QString(), filter);
}

KPluginInfo::List PluginLoader::listAppletInfoForMimeType(const QString &mimeType)
{
    return KPluginInfo::fromMetaData(listAppletMetaDataForMimeType(mimeType).toVector());
}

QList<KPluginMetaData> PluginLoader::listAppletMetaDataForUrl(const QUrl &url)
{
    QString parentApp;
    QCoreApplication *app = QCoreApplication::instance();
    if (app) {
        parentApp = app->applicationName();
    }

    auto filter = [&parentApp](const KPluginMetaData &md) -> bool
    {
        const QString pa = md.value(QStringLiteral("X-KDE-ParentApp"));
        return (pa.isEmpty() || pa == parentApp) && !KPluginMetaData::readStringList(md.rawData(), QStringLiteral("X-Plasma-DropUrlPatterns")).isEmpty();
    };
    const QList<KPluginMetaData> allApplets = KPackage::PackageLoader::self()->findPackages(QStringLiteral("Plasma/Applet"), QString(), filter);

    QList<KPluginMetaData> filtered;
    foreach (const KPluginMetaData &md, allApplets) {
        QStringList urlPatterns = KPluginMetaData::readStringList(md.rawData(), QStringLiteral("X-Plasma-DropUrlPatterns"));
        foreach (const QString &glob, urlPatterns) {
            QRegExp rx(glob);
            rx.setPatternSyntax(QRegExp::Wildcard);
            if (rx.exactMatch(url.toString())) {
#ifndef NDEBUG
                // qCDebug(LOG_PLASMA) << md.name() << "matches" << glob << url;
#endif
                filtered << md;
            }
        }
    }

    return filtered;
}

KPluginInfo::List PluginLoader::listAppletInfoForUrl(const QUrl &url)
{
    return KPluginInfo::fromMetaData(listAppletMetaDataForUrl(url).toVector());
}

QStringList PluginLoader::listAppletCategories(const QString &parentApp, bool visibleOnly)
{
    KConfigGroup group(KSharedConfig::openConfig(), "General");
    const QStringList excluded = group.readEntry("ExcludeCategories", QStringList());
    auto filter = [&parentApp, &excluded, visibleOnly](const KPluginMetaData &md) -> bool
    {
        const QString pa = md.value(QStringLiteral("X-KDE-ParentApp"));
        return (pa.isEmpty() || pa == parentApp)
            && (excluded.isEmpty() || excluded.contains(md.value(QStringLiteral("X-KDE-PluginInfo-Category"))))
            && (!visibleOnly || !md.isHidden());
    };
    const QList<KPluginMetaData> allApplets = KPackage::PackageLoader::self()->findPackages(QStringLiteral("Plasma/Applet"), QString(), filter);


    QStringList categories;
    foreach (auto& plugin, allApplets) {
        if (plugin.category().isEmpty()) {
            if (!categories.contains(i18nc("misc category", "Miscellaneous"))) {
                categories << i18nc("misc category", "Miscellaneous");
            }
        } else {
            categories << plugin.category();
        }
    }
    categories.sort();
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

    const KPackage::Package p = KPackage::PackageLoader::self()->loadPackage(QStringLiteral("Plasma/Applet"), appletName);
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
    KConfigGroup group(KSharedConfig::openConfig(), "General");
    const QStringList excluded = group.readEntry("ExcludeCategories", QStringList());
    auto filter = [&type, &category, &parentApp](const KPluginMetaData &md) -> bool
    {
        if (!md.serviceTypes().contains(QStringLiteral("Plasma/Containment"))) {
            return false;
        }
        const QString pa = md.value(QStringLiteral("X-KDE-ParentApp"));
        if (!pa.isEmpty() && pa != parentApp) {
            return false;
        }

        if (!type.isEmpty() && md.value(QStringLiteral("X-Plasma-ContainmentType")) != type) {
            return false;
        }

        if (!category.isEmpty() && md.value(QStringLiteral("X-KDE-PluginInfo-Category")) != category) {
            return false;
        }

        return true;
    };

    return KPluginInfo::fromMetaData(KPackage::PackageLoader::self()->findPackages(QStringLiteral("Plasma/Applet"), QString(), filter).toVector());
}

KPluginInfo::List PluginLoader::listContainmentsForMimeType(const QString &mimeType)
{
    auto filter = [&mimeType](const KPluginMetaData &md) -> bool
    {
        return md.serviceTypes().contains(QLatin1String("Plasma/Containment"))
            && KPluginMetaData::readStringList(md.rawData(), QStringLiteral("X-Plasma-DropMimeTypes")).contains(mimeType);
    };

    return KPluginInfo::fromMetaData(KPackage::PackageLoader::self()->findPackages(QStringLiteral("Plasma/Applet"), QString(), filter).toVector());
}

QStringList PluginLoader::listContainmentTypes()
{
    KPluginInfo::List containmentInfos = listContainments();
    QSet<QString> types;

    foreach (const KPluginInfo &containmentInfo, containmentInfos) {
        QStringList theseTypes = containmentInfo.service()->property(QStringLiteral("X-Plasma-ContainmentType")).toStringList();
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
        constraint = QStringLiteral("not exist [X-KDE-ParentApp]");
    } else {
        constraint = QLatin1String("[X-KDE-ParentApp] == '") + parentApp + '\'';
    }

    list.append(KPluginTrader::self()->query(PluginLoaderPrivate::s_dataEnginePluginDir, QStringLiteral("Plasma/DataEngine"), constraint));
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
        constraint = QStringLiteral("not exist [X-KDE-ParentApp]");
    } else {
        constraint = QLatin1String("[X-KDE-ParentApp] == '") + parentApp + '\'';
    }

    list.append(KPluginTrader::self()->query(PluginLoaderPrivate::s_containmentActionsPluginDir, QStringLiteral("Plasma/ContainmentActions"), constraint));

    QSet<QString> knownPlugins;
    foreach (const KPluginInfo &p, list) {
        knownPlugins.insert(p.pluginName());
    }

    //FIXME: this is only for backwards compatibility, but probably will have to stay
    //for the time being
    KService::List offers = KServiceTypeTrader::self()->query(QStringLiteral("Plasma/ContainmentActions"), constraint);
    foreach (KService::Ptr s, offers) {
        if (!knownPlugins.contains(s->pluginKeyword())) {
            list.append(KPluginInfo(s));
        }
    }
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


#ifndef PLASMA_NO_DEPRECATED
Package PluginLoader::internalLoadPackage(const QString &name, const QString &specialization)
{
    Q_UNUSED(name);
    Q_UNUSED(specialization);
    return Package();
}
#endif

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
                        QLatin1String(PLASMA_RELATIVE_DATA_INSTALL_DIR "/internal/") + type + QLatin1String("/*.desktop"),
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
    return standardInternalInfo(QStringLiteral("applets"), category);
}

KPluginInfo::List PluginLoader::standardInternalDataEngineInfo() const
{
    return standardInternalInfo(QStringLiteral("dataengines"));
}

KPluginInfo::List PluginLoader::standardInternalServiceInfo() const
{
    return standardInternalInfo(QStringLiteral("services"));
}

bool PluginLoader::isPluginVersionCompatible(KPluginLoader &loader)
{
    const quint32 version = loader.pluginVersion();
    if (version == quint32(-1)) {
        // unversioned, just let it through
        qCWarning(LOG_PLASMA) << loader.fileName() << "unversioned plugin detected, may result in instability";
        return true;
    }

    // we require PLASMA_VERSION_MAJOR and PLASMA_VERSION_MINOR
    const quint32 minVersion = PLASMA_MAKE_VERSION(PLASMA_VERSION_MAJOR, 0, 0);
    const quint32 maxVersion = PLASMA_MAKE_VERSION(PLASMA_VERSION_MAJOR, PLASMA_VERSION_MINOR, 60);

    if (version < minVersion || version > maxVersion) {
#ifndef NDEBUG
        qCDebug(LOG_PLASMA) << loader.fileName() << ": this plugin is compiled against incompatible Plasma version" << version
                << "This build is compatible with" << PLASMA_VERSION_MAJOR << ".0.0 (" << minVersion
                << ") to" << PLASMA_VERSION_STRING << "(" << maxVersion << ")";
#endif
        return false;
    }

    return true;
}

} // Plasma Namespace

