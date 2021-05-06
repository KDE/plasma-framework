/*
    SPDX-FileCopyrightText: 2010 Ryan Rix <ry@n.rix.si>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "pluginloader.h"

#include <QPointer>
#include <QStandardPaths>

#include <KPluginLoader>
#include <KService>
#include <KServiceTypeTrader>
#include <QDebug>
#include <kdeclarative/kdeclarative.h>
#include <kpackage/packageloader.h>

#include "config-plasma.h"

#include "applet.h"
#include "containment.h"
#include "containmentactions.h"
#include "dataengine.h"
#include "debug_p.h"
#include "package.h"
#include "private/applet_p.h"
#if PLASMA_BUILD_DEPRECATED_SINCE(5, 83)
#include "private/package_p.h"
#include "private/packagestructure_p.h"
#endif
#include "private/service_p.h" // for NullService
#include "private/storage_p.h"
#include <plasma/version.h>

namespace Plasma
{
static PluginLoader *s_pluginLoader = nullptr;

class PluginLoaderPrivate
{
public:
    PluginLoaderPrivate()
        : isDefaultLoader(false)
    {
    }

    static QSet<QString> knownCategories();

    static QSet<QString> s_customCategories;
#if PLASMA_BUILD_DEPRECATED_SINCE(5, 83)
    QHash<QString, QPointer<PackageStructure>> structures;
#endif
    bool isDefaultLoader;

    static QString s_dataEnginePluginDir;
    static QString s_packageStructurePluginDir;
    static QString s_plasmoidsPluginDir;
    static QString s_servicesPluginDir;
    static QString s_containmentActionsPluginDir;

    class Cache
    {
        // We only use this cache during start of the process to speed up many consecutive calls
        // After that, we're too afraid to produce race conditions and it's not that time-critical anyway
        // the 20 seconds here means that the cache is only used within 20sec during startup, after that,
        // complexity goes up and we'd have to update the cache in order to avoid subtle bugs
        // just not using the cache is way easier then, since it doesn't make *that* much of a difference,
        // anyway
        int maxCacheAge = 20;
        qint64 pluginCacheAge = 0;
        QHash<QString, QVector<KPluginMetaData>> plugins;

    public:
        QVector<KPluginMetaData> findPluginsById(const QString &name, const QStringList &dirs);
    };
    Cache plasmoidCache;
    Cache dataengineCache;
    Cache containmentactionCache;
};

QSet<QString> PluginLoaderPrivate::s_customCategories;

QString PluginLoaderPrivate::s_dataEnginePluginDir = QStringLiteral("plasma/dataengine");
QString PluginLoaderPrivate::s_packageStructurePluginDir = QStringLiteral("plasma/packagestructure");
QString PluginLoaderPrivate::s_plasmoidsPluginDir = QStringLiteral("plasma/applets");
QString PluginLoaderPrivate::s_servicesPluginDir = QStringLiteral("plasma/services");
QString PluginLoaderPrivate::s_containmentActionsPluginDir = QStringLiteral("plasma/containmentactions");

QSet<QString> PluginLoaderPrivate::knownCategories()
{
    // this is to trick the translation tools into making the correct
    // strings for translation
    QSet<QString> categories = s_customCategories;
    /* clang-format off */
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
    /* clang-format on */
    return categories;
}

PluginLoader::PluginLoader()
    : d(new PluginLoaderPrivate)
{
}

PluginLoader::~PluginLoader()
{
#if PLASMA_BUILD_DEPRECATED_SINCE(5, 83)
    for (const auto &wp : qAsConst(d->structures)) {
        delete wp;
    }
#endif
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
        return nullptr;
    }

    Applet *applet = d->isDefaultLoader ? nullptr : internalLoadApplet(name, appletId, args);
    if (applet) {
        return applet;
    }

    if (appletId == 0) {
        appletId = ++AppletPrivate::s_maxAppletId;
    }

    // Need to pass the empty directory because it's where plasmoids used to be
    auto plugins = d->plasmoidCache.findPluginsById(name, {PluginLoaderPrivate::s_plasmoidsPluginDir, {}});

    const KPackage::Package p = KPackage::PackageLoader::self()->loadPackage(QStringLiteral("Plasma/Applet"), name);

    // If the applet is using another applet package, search for the plugin of the other applet
    if (plugins.isEmpty()) {
        const QString parentPlugin = p.metadata().value(QStringLiteral("X-Plasma-RootPath"));
        if (!parentPlugin.isEmpty()) {
            plugins = d->plasmoidCache.findPluginsById(parentPlugin, {PluginLoaderPrivate::s_plasmoidsPluginDir, {}});
        }
    }

    if (!plugins.isEmpty()) {
        KPluginLoader loader(plugins.first().fileName());
        if (!isPluginVersionCompatible(loader)) {
            return nullptr;
        }
        KPluginFactory *factory = loader.factory();
        if (factory) {
            QVariantList allArgs;
            allArgs << QVariant::fromValue(p) << loader.metaData().toVariantMap() << appletId << args;
            applet = factory->create<Plasma::Applet>(nullptr, allArgs);
        }
    }
    if (applet) {
        return applet;
    }

    if (!applet) {
        // qCDebug(LOG_PLASMA) << name << "not a C++ applet: Falling back to an empty one";

        QVariantList allArgs;
        allArgs << QVariant::fromValue(p) << p.metadata().fileName() << appletId << args;

        if (p.metadata().serviceTypes().contains(QLatin1String("Plasma/Containment"))) {
            applet = new Containment(nullptr, allArgs);
        } else {
            applet = new Applet(nullptr, allArgs);
        }
    }

    const QString localePath = p.filePath("translations");
    if (!localePath.isEmpty()) {
        KLocalizedString::addDomainLocaleDir(QByteArray("plasma_applet_") + name.toLatin1(), localePath);
    }
    return applet;
}

DataEngine *PluginLoader::loadDataEngine(const QString &name)
{
    DataEngine *engine = d->isDefaultLoader ? nullptr : internalLoadDataEngine(name);
    if (engine) {
        return engine;
    }

    // Look for C++ plugins first
    const QVector<KPluginMetaData> plugins = d->dataengineCache.findPluginsById(name, {PluginLoaderPrivate::s_dataEnginePluginDir});
    if (!plugins.isEmpty()) {
        KPluginLoader loader(plugins.constFirst().fileName());
        const QVariantList argsWithMetaData = QVariantList() << loader.metaData().toVariantMap();
        KPluginFactory *factory = loader.factory();
        return factory ? factory->create<Plasma::DataEngine>(nullptr, argsWithMetaData) : nullptr;
    }
    if (engine) {
        return engine;
    }

    const KPackage::Package p = KPackage::PackageLoader::self()->loadPackage(QStringLiteral("Plasma/DataEngine"), name);
    if (!p.isValid()) {
        return nullptr;
    }

    return new DataEngine(p.metadata(), nullptr);
}

QStringList PluginLoader::listAllEngines(const QString &parentApp)
{
    QStringList engines;
    // Look for C++ plugins first
    auto filter = [&parentApp](const KPluginMetaData &md) -> bool {
        return md.value(QStringLiteral("X-KDE-ParentApp")) == parentApp;
    };
    QVector<KPluginMetaData> plugins;
    if (parentApp.isEmpty()) {
        plugins = KPluginLoader::findPlugins(PluginLoaderPrivate::s_dataEnginePluginDir);
    } else {
        plugins = KPluginLoader::findPlugins(PluginLoaderPrivate::s_dataEnginePluginDir, filter);
    }

    for (auto &plugin : qAsConst(plugins)) {
        engines << plugin.pluginId();
    }

    const QList<KPluginMetaData> packagePlugins = KPackage::PackageLoader::self()->listPackages(QStringLiteral("Plasma/DataEngine"));
    for (const auto &plugin : packagePlugins) {
        engines << plugin.pluginId();
    }

    return engines;
}

#if PLASMA_BUILD_DEPRECATED_SINCE(5, 77)
KPluginInfo::List PluginLoader::listEngineInfo(const QString &parentApp)
{
    return PluginLoader::self()->listDataEngineInfo(parentApp);
}
#endif

#if PLASMA_BUILD_DEPRECATED_SINCE(5, 81)
KPluginInfo::List PluginLoader::listEngineInfoByCategory(const QString &category, const QString &parentApp)
{
    KPluginInfo::List list;

    // Look for C++ plugins first
    auto filterNormal = [&category](const KPluginMetaData &md) -> bool {
        return md.value(QStringLiteral("X-KDE-PluginInfo-Category")) == category;
    };
    auto filterParentApp = [&category, &parentApp](const KPluginMetaData &md) -> bool {
        return md.value(QStringLiteral("X-KDE-ParentApp")) == parentApp //
            && md.value(QStringLiteral("X-KDE-PluginInfo-Category")) == category;
    };
    QVector<KPluginMetaData> plugins;
    if (parentApp.isEmpty()) {
        plugins = KPluginLoader::findPlugins(PluginLoaderPrivate::s_dataEnginePluginDir, filterNormal);
    } else {
        plugins = KPluginLoader::findPlugins(PluginLoaderPrivate::s_dataEnginePluginDir, filterParentApp);
    }

    list = KPluginInfo::fromMetaData(plugins);

    // TODO FIXME: PackageLoader needs to have a function to inject packageStructures
    const QList<KPluginMetaData> packagePlugins = KPackage::PackageLoader::self()->listPackages(QStringLiteral("Plasma/DataEngine"));
    list << KPluginInfo::fromMetaData(packagePlugins.toVector());

    return list;
}
#endif

Service *PluginLoader::loadService(const QString &name, const QVariantList &args, QObject *parent)
{
    Service *service = d->isDefaultLoader ? nullptr : internalLoadService(name, args, parent);
    if (service) {
        return service;
    }

    // TODO: scripting API support
    if (name.isEmpty()) {
        return new NullService(QString(), parent);
    } else if (name == QLatin1String("org.kde.servicestorage")) {
        return new Storage(parent);
    }

    // Look for C++ plugins first
    auto filter = [&name](const KPluginMetaData &md) -> bool {
        return md.pluginId() == name;
    };
    QVector<KPluginMetaData> plugins = KPluginLoader::findPlugins(PluginLoaderPrivate::s_servicesPluginDir, filter);

    if (!plugins.isEmpty()) {
        KPluginLoader loader(plugins.first().fileName());
        if (!isPluginVersionCompatible(loader)) {
            return nullptr;
        }
        KPluginFactory *factory = loader.factory();
        if (factory) {
            service = factory->create<Plasma::Service>(nullptr, args);
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
        return nullptr;
    }

    ContainmentActions *actions = d->isDefaultLoader ? nullptr : internalLoadContainmentActions(parent, name, args);
    if (actions) {
        return actions;
    }

    const QVector<KPluginMetaData> plugins = d->containmentactionCache.findPluginsById(name, {PluginLoaderPrivate::s_containmentActionsPluginDir});

    if (!plugins.isEmpty()) {
        KPluginLoader loader(plugins.first().fileName());
        KPluginFactory *factory = loader.factory();
        if (factory) {
            actions = factory->create<Plasma::ContainmentActions>(nullptr, {QVariant::fromValue(plugins.first())});
        }
    }
    if (actions) {
        return actions;
    }

    // FIXME: this is only for backwards compatibility, but probably will have to stay
    // for the time being
    QString constraint = QStringLiteral("[X-KDE-PluginInfo-Name] == '%1'").arg(name);
    KService::List offers = KServiceTypeTrader::self()->query(QStringLiteral("Plasma/ContainmentActions"), constraint);

    if (offers.isEmpty()) {
#ifndef NDEBUG
        qCDebug(LOG_PLASMA) << "offers is empty for " << name;
#endif
        return nullptr;
    }

    KService::Ptr offer = offers.first();
    KPluginLoader plugin(*offer);

    if (!isPluginVersionCompatible(plugin)) {
        return nullptr;
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

#if PLASMA_BUILD_DEPRECATED_SINCE(5, 83)
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

    const QString hashkey = packageFormat + QLatin1Char('%') + specialization;
    PackageStructure *structure = d->structures.value(hashkey).data();

    if (structure) {
        return Package(structure);
    }

    KPackage::PackageStructure *internalStructure = KPackage::PackageLoader::self()->loadPackageStructure(packageFormat);

    if (internalStructure) {
        structure = new PackageStructure();
        structure->d->internalStructure = internalStructure;
        // fallback to old structures
    } else {
        auto filter = [packageFormat](const KPluginMetaData &md) -> bool {
            return md.value(QStringLiteral("X-KDE-PluginInfo-Name")) == packageFormat;
        };

        const QVector<KPluginMetaData> plugins = KPluginLoader::findPlugins(PluginLoaderPrivate::s_packageStructurePluginDir, filter);

        if (!plugins.isEmpty()) {
            KPluginLoader loader(plugins.first().fileName());
            KPluginFactory *factory = loader.factory();
            if (!factory) {
                qWarning() << "Error loading plugin:" << loader.errorString();
            } else {
                structure = factory->create<Plasma::PackageStructure>();
            }

            if (structure) {
                structure->d->internalStructure = new PackageStructureWrapper(structure);
            }
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
    auto platforms = KDeclarative::KDeclarative::runtimePlatform();
    // For now desktop always lists everything
    if (platforms.contains(QStringLiteral("desktop"))) {
        platforms.clear();
    }

    // FIXME: this assumes we are always use packages.. no pure c++
    std::function<bool(const KPluginMetaData &)> filter;
    if (category.isEmpty()) { // use all but the excluded categories
        KConfigGroup group(KSharedConfig::openConfig(), "General");
        QStringList excluded = group.readEntry("ExcludeCategories", QStringList());

        filter = [excluded, parentApp, platforms](const KPluginMetaData &md) -> bool {
            if (!platforms.isEmpty() && !md.formFactors().isEmpty()) {
                bool found = false;
                for (const auto &plat : platforms) {
                    if (md.formFactors().contains(plat)) {
                        found = true;
                        break;
                    }
                }

                if (!found) {
                    return false;
                }
            }

            const QString pa = md.value(QStringLiteral("X-KDE-ParentApp"));
            return (parentApp.isEmpty() || pa == parentApp) && !excluded.contains(md.category());
        };
    } else { // specific category (this could be an excluded one - is that bad?)

        filter = [category, parentApp, platforms](const KPluginMetaData &md) -> bool {
            if (!platforms.isEmpty() && !md.formFactors().isEmpty()) {
                bool found = false;
                for (const auto &plat : platforms) {
                    if (md.formFactors().contains(plat)) {
                        found = true;
                        break;
                    }
                }

                if (!found) {
                    return false;
                }
            }

            const QString pa = md.value(QStringLiteral("X-KDE-ParentApp"));

            if (category == QLatin1String("Miscellaneous")) {
                return (parentApp.isEmpty() || pa == parentApp) && (md.category() == category || md.category().isEmpty());
            } else {
                return (parentApp.isEmpty() || pa == parentApp) && md.category() == category;
            }
        };
    }

    QList<KPluginMetaData> list;
    if (!d->isDefaultLoader && (parentApp.isEmpty() || parentApp == QCoreApplication::instance()->applicationName())) {
        list = KPluginInfo::toMetaData(internalAppletInfo(category)).toList();
    }
    return KPackage::PackageLoader::self()->findPackages(QStringLiteral("Plasma/Applet"), QString(), filter);
}

#if PLASMA_BUILD_DEPRECATED_SINCE(5, 28)
KPluginInfo::List PluginLoader::listAppletInfo(const QString &category, const QString &parentApp)
{
    const auto plugins = listAppletMetaData(category, parentApp);

#if KSERVICE_BUILD_DEPRECATED_SINCE(5, 0)
    KPluginInfo::List list;
    // NOTE: it still produces kplugininfos from KServices because some user code expects
    // info.service() to be valid and would crash otherwise
    for (const auto &md : plugins) {
        QT_WARNING_PUSH
        QT_WARNING_DISABLE_CLANG("-Wdeprecated-declarations")
        QT_WARNING_DISABLE_GCC("-Wdeprecated-declarations")
        auto pi = md.metaDataFileName().endsWith(QLatin1String(".json")) ? KPluginInfo(md) : KPluginInfo(KService::serviceByStorageId(md.metaDataFileName()));
        QT_WARNING_POP
        if (!pi.isValid()) {
            qCWarning(LOG_PLASMA) << "Could not load plugin info for plugin :" << md.pluginId() << "skipping plugin";
            continue;
        }
        list << pi;
    }
    return list;
#else
    return KPluginInfo::fromMetaData(plugins.toVector());
#endif
}
#endif

QList<KPluginMetaData> PluginLoader::listAppletMetaDataForMimeType(const QString &mimeType)
{
    auto filter = [&mimeType](const KPluginMetaData &md) -> bool {
        return KPluginMetaData::readStringList(md.rawData(), QStringLiteral("X-Plasma-DropMimeTypes")).contains(mimeType);
    };
    return KPackage::PackageLoader::self()->findPackages(QStringLiteral("Plasma/Applet"), QString(), filter);
}
#if PLASMA_BUILD_DEPRECATED_SINCE(5, 83)
KPluginInfo::List PluginLoader::listAppletInfoForMimeType(const QString &mimeType)
{
    return KPluginInfo::fromMetaData(listAppletMetaDataForMimeType(mimeType).toVector());
}
#endif

QList<KPluginMetaData> PluginLoader::listAppletMetaDataForUrl(const QUrl &url)
{
    QString parentApp;
    QCoreApplication *app = QCoreApplication::instance();
    if (app) {
        parentApp = app->applicationName();
    }

    auto filter = [&parentApp](const KPluginMetaData &md) -> bool {
        const QString pa = md.value(QStringLiteral("X-KDE-ParentApp"));
        return (parentApp.isEmpty() || pa == parentApp) //
            && !KPluginMetaData::readStringList(md.rawData(), QStringLiteral("X-Plasma-DropUrlPatterns")).isEmpty();
    };
    const QList<KPluginMetaData> allApplets = KPackage::PackageLoader::self()->findPackages(QStringLiteral("Plasma/Applet"), QString(), filter);

    QList<KPluginMetaData> filtered;
    for (const KPluginMetaData &md : allApplets) {
        const QStringList urlPatterns = KPluginMetaData::readStringList(md.rawData(), QStringLiteral("X-Plasma-DropUrlPatterns"));
        for (const QString &glob : urlPatterns) {
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

#if PLASMA_BUILD_DEPRECATED_SINCE(5, 36)
KPluginInfo::List PluginLoader::listAppletInfoForUrl(const QUrl &url)
{
    return KPluginInfo::fromMetaData(listAppletMetaDataForUrl(url).toVector());
}
#endif

QStringList PluginLoader::listAppletCategories(const QString &parentApp, bool visibleOnly)
{
    KConfigGroup group(KSharedConfig::openConfig(), "General");
    const QStringList excluded = group.readEntry("ExcludeCategories", QStringList());
    auto filter = [&parentApp, &excluded, visibleOnly](const KPluginMetaData &md) -> bool {
        const QString pa = md.value(QStringLiteral("X-KDE-ParentApp"));
        return (parentApp.isEmpty() || pa == parentApp) //
            && (excluded.isEmpty() || excluded.contains(md.value(QStringLiteral("X-KDE-PluginInfo-Category")))) //
            && (!visibleOnly || !md.isHidden());
    };
    const QList<KPluginMetaData> allApplets = KPackage::PackageLoader::self()->findPackages(QStringLiteral("Plasma/Applet"), QString(), filter);

    QStringList categories;
    for (auto &plugin : allApplets) {
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
    PluginLoaderPrivate::s_customCategories = QSet<QString>(categories.begin(), categories.end());
}

QStringList PluginLoader::customAppletCategories() const
{
    return PluginLoaderPrivate::s_customCategories.values();
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

#if PLASMA_BUILD_DEPRECATED_SINCE(5, 83)
KPluginInfo::List PluginLoader::listContainments(const QString &category, const QString &parentApp)
{
    return listContainmentsOfType(QString(), category, parentApp);
}
#endif

QList<KPluginMetaData> PluginLoader::listContainmentsMetaData(std::function<bool(const KPluginMetaData &)> filter)
{
    auto ownFilter = [filter](const KPluginMetaData &md) -> bool {
        if (!md.serviceTypes().contains(QLatin1String("Plasma/Containment"))) {
            return false;
        }

        return filter(md);
    };

    return KPackage::PackageLoader::self()->findPackages(QStringLiteral("Plasma/Applet"), QString(), ownFilter);
}

QList<KPluginMetaData> PluginLoader::listContainmentsMetaDataOfType(const QString &type)
{
    auto filter = [type](const KPluginMetaData &md) -> bool {
        return md.value(QStringLiteral("X-Plasma-ContainmentType")) == type;
    };

    return listContainmentsMetaData(filter);
}

#if PLASMA_BUILD_DEPRECATED_SINCE(5, 83)
KPluginInfo::List PluginLoader::listContainmentsOfType(const QString &type, const QString &category, const QString &parentApp)
{
    KConfigGroup group(KSharedConfig::openConfig(), "General");
    auto filter = [&type, &category, &parentApp](const KPluginMetaData &md) -> bool {
        if (!md.serviceTypes().contains(QLatin1String("Plasma/Containment"))) {
            return false;
        }
        if (!parentApp.isEmpty() && md.value(QStringLiteral("X-KDE-ParentApp")) != parentApp) {
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
#endif

#if PLASMA_BUILD_DEPRECATED_SINCE(5, 83)
KPluginInfo::List PluginLoader::listContainmentsForMimeType(const QString &mimeType)
{
    auto filter = [&mimeType](const KPluginMetaData &md) -> bool {
        return md.serviceTypes().contains(QLatin1String("Plasma/Containment"))
            && KPluginMetaData::readStringList(md.rawData(), QStringLiteral("X-Plasma-DropMimeTypes")).contains(mimeType);
    };

    return KPluginInfo::fromMetaData(KPackage::PackageLoader::self()->findPackages(QStringLiteral("Plasma/Applet"), QString(), filter).toVector());
}
#endif

#if PLASMA_BUILD_DEPRECATED_SINCE(5, 83)
QStringList PluginLoader::listContainmentTypes()
{
    const KPluginInfo::List containmentInfos = listContainments();
    QSet<QString> types;

    for (const KPluginInfo &containmentInfo : containmentInfos) {
        const QStringList theseTypes = containmentInfo.property(QStringLiteral("X-Plasma-ContainmentType")).toStringList();
        for (const QString &type : theseTypes) {
            types.insert(type);
        }
    }

    return types.values();
}
#endif

#if PLASMA_BUILD_DEPRECATED_SINCE(5, 77)
KPluginInfo::List PluginLoader::listDataEngineInfo(const QString &parentApp)
{
    return KPluginInfo::fromMetaData(listDataEngineMetaData(parentApp));
}
#endif

QVector<KPluginMetaData> PluginLoader::listDataEngineMetaData(const QString &parentApp)
{
    auto filter = [&parentApp](const KPluginMetaData &md) -> bool {
        return md.value(QStringLiteral("X-KDE-ParentApp")) == parentApp;
    };

    QVector<KPluginMetaData> plugins;
    if (parentApp.isEmpty()) {
        plugins = KPluginLoader::findPlugins(PluginLoaderPrivate::s_dataEnginePluginDir);
    } else {
        plugins = KPluginLoader::findPlugins(PluginLoaderPrivate::s_dataEnginePluginDir, filter);
    }

    return plugins;
}

#if PLASMA_BUILD_DEPRECATED_SINCE(5, 77)
KPluginInfo::List PluginLoader::listContainmentActionsInfo(const QString &parentApp)
{
    return KPluginInfo::fromMetaData(listContainmentActionsMetaData(parentApp));
}
#endif

QVector<KPluginMetaData> PluginLoader::listContainmentActionsMetaData(const QString &parentApp)
{
    auto filter = [&parentApp](const KPluginMetaData &md) -> bool {
        return md.value(QStringLiteral("X-KDE-ParentApp")) == parentApp;
    };

    QVector<KPluginMetaData> plugins;
    if (parentApp.isEmpty()) {
        plugins = KPluginLoader::findPlugins(PluginLoaderPrivate::s_containmentActionsPluginDir);
    } else {
        plugins = KPluginLoader::findPlugins(PluginLoaderPrivate::s_containmentActionsPluginDir, filter);
    }

#if KSERVICE_BUILD_DEPRECATED_SINCE(5, 0)
    // FIXME: this is only for backwards compatibility, but probably will have to stay
    // for the time being
    QSet<QString> knownPlugins;
    for (const KPluginMetaData &p : qAsConst(plugins)) {
        knownPlugins.insert(p.pluginId());
    }
    QString constraint;
    if (!parentApp.isEmpty()) {
        constraint = QLatin1String("[X-KDE-ParentApp] == '") + parentApp + QLatin1Char('\'');
    }
    const KService::List offers = KServiceTypeTrader::self()->query(QStringLiteral("Plasma/ContainmentActions"), constraint);
    for (KService::Ptr s : offers) {
        if (!knownPlugins.contains(s->pluginKeyword())) {
            QT_WARNING_PUSH
            QT_WARNING_DISABLE_CLANG("-Wdeprecated-declarations")
            QT_WARNING_DISABLE_GCC("-Wdeprecated-declarations")
            plugins.append(KPluginInfo(s).toMetaData());
            QT_WARNING_POP
        }
    }
#endif

    return plugins;
}

Applet *PluginLoader::internalLoadApplet(const QString &name, uint appletId, const QVariantList &args)
{
    Q_UNUSED(name)
    Q_UNUSED(appletId)
    Q_UNUSED(args)
    return nullptr;
}

DataEngine *PluginLoader::internalLoadDataEngine(const QString &name)
{
    Q_UNUSED(name)
    return nullptr;
}

ContainmentActions *PluginLoader::internalLoadContainmentActions(Containment *containment, const QString &name, const QVariantList &args)
{
    Q_UNUSED(containment)
    Q_UNUSED(name)
    Q_UNUSED(args)
    return nullptr;
}

Service *PluginLoader::internalLoadService(const QString &name, const QVariantList &args, QObject *parent)
{
    Q_UNUSED(name)
    Q_UNUSED(args)
    Q_UNUSED(parent)
    return nullptr;
}

#if PLASMA_BUILD_DEPRECATED_SINCE(5, 83)
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

    const KPluginInfo::List allInfo = KPluginInfo::fromFiles(files);

    if (category.isEmpty() || allInfo.isEmpty()) {
        return allInfo;
    }

    KPluginInfo::List matchingInfo;
    for (const KPluginInfo &info : allInfo) {
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
        qCWarning(LOG_PLASMA) << loader.fileName() << ": this plugin is compiled against incompatible Plasma version" << version
                              << "This build is compatible with" << PLASMA_VERSION_MAJOR << ".0.0 (" << minVersion << ") to" << PLASMA_VERSION_STRING << "("
                              << maxVersion << ")";
        return false;
    }

    return true;
}

QVector<KPluginMetaData> PluginLoaderPrivate::Cache::findPluginsById(const QString &name, const QStringList &dirs)
{
    const qint64 now = qRound64(QDateTime::currentMSecsSinceEpoch() / 1000.0);
    bool useRuntimeCache = true;

    if (pluginCacheAge == 0) {
        // Find all the plugins now, but only once
        pluginCacheAge = now;

        auto insertIntoCache = [this](const QString &pluginPath) {
            KPluginMetaData metadata(pluginPath);
            if (!metadata.isValid()) {
                qCDebug(LOG_PLASMA) << "invalid metadata" << pluginPath;
                return;
            }

            plugins[metadata.pluginId()].append(metadata);
        };

        for (const QString &dir : dirs)
            KPluginLoader::forEachPlugin(dir, insertIntoCache);
    } else if (now - pluginCacheAge > maxCacheAge) {
        // cache is old and we're not within a few seconds of startup anymore
        useRuntimeCache = false;
        plugins.clear();
    }

    // if name wasn't a path, pluginName == name
    const QString pluginName = name.section(QLatin1Char('/'), -1);

    QVector<KPluginMetaData> ret;

    if (useRuntimeCache) {
        auto it = plugins.constFind(pluginName);
        if (it != plugins.constEnd()) {
            ret = *it;
        }
        qCDebug(LOG_PLASMA) << "loading applet by name" << name << useRuntimeCache << ret.size();
    } else {
        for (const auto &dir : dirs) {
            ret = KPluginLoader::findPluginsById(dir, pluginName);
            if (!ret.isEmpty())
                break;
        }
    }
    return ret;
}

} // Plasma Namespace
