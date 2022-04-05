/*
    SPDX-FileCopyrightText: 2010 Ryan Rix <ry@n.rix.si>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "pluginloader.h"

#include <QPointer>
#include <QStandardPaths>

#include <KLazyLocalizedString>
#include <KService>
#include <KServiceTypeTrader>
#include <QDebug>
#include <QRegExp>
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

namespace Plasma
{
inline bool isContainmentMetaData(const KPluginMetaData &md)
{
    return md.rawData().contains(QStringLiteral("X-Plasma-ContainmentType"))
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        || md.serviceTypes().contains(QLatin1String("Plasma/Containment"))
#endif
        ;
}
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
        QHash<QString, KPluginMetaData> plugins;

    public:
        KPluginMetaData findPluginById(const QString &name, const QString &pluginNamespace);
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
    categories << kli18n("Accessibility").toString().toLower()
               << kli18n("Application Launchers").toString().toLower()
               << kli18n("Astronomy").toString().toLower()
               << kli18n("Date and Time").toString().toLower()
               << kli18n("Development Tools").toString().toLower()
               << kli18n("Education").toString().toLower()
               << kli18n("Environment and Weather").toString().toLower()
               << kli18n("Examples").toString().toLower()
               << kli18n("File System").toString().toLower()
               << kli18n("Fun and Games").toString().toLower()
               << kli18n("Graphics").toString().toLower()
               << kli18n("Language").toString().toLower()
               << kli18n("Mapping").toString().toLower()
               << kli18n("Miscellaneous").toString().toLower()
               << kli18n("Multimedia").toString().toLower()
               << kli18n("Online Services").toString().toLower()
               << kli18n("Productivity").toString().toLower()
               << kli18n("System Information").toString().toLower()
               << kli18n("Utilities").toString().toLower()
               << kli18n("Windows and Tasks").toString().toLower()
               << kli18n("Clipboard").toString().toLower()
               << kli18n("Tasks").toString().toLower();
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
    for (const auto &wp : std::as_const(d->structures)) {
        delete wp;
    }
#endif
    delete d;
}

#if PLASMA_BUILD_DEPRECATED_SINCE(5, 86)
void PluginLoader::setPluginLoader(PluginLoader *loader)
{
    if (!s_pluginLoader) {
        s_pluginLoader = loader;
    }
}
#endif

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

#if PLASMA_BUILD_DEPRECATED_SINCE(5, 86)
    Applet *applet = d->isDefaultLoader ? nullptr : internalLoadApplet(name, appletId, args);
    if (applet) {
        return applet;
    }
#else
    Applet *applet = nullptr;
#endif

    if (appletId == 0) {
        appletId = ++AppletPrivate::s_maxAppletId;
    }

    auto plugin = d->plasmoidCache.findPluginById(name, PluginLoaderPrivate::s_plasmoidsPluginDir);
    const KPackage::Package p = KPackage::PackageLoader::self()->loadPackage(QStringLiteral("Plasma/Applet"), name);

    // If the applet is using another applet package, search for the plugin of the other applet
    if (!plugin.isValid()) {
        const QString parentPlugin = p.metadata().value(QStringLiteral("X-Plasma-RootPath"));
        if (!parentPlugin.isEmpty()) {
            plugin = d->plasmoidCache.findPluginById(parentPlugin, PluginLoaderPrivate::s_plasmoidsPluginDir);
        }
    }

    if (plugin.isValid()) {
        QPluginLoader loader(plugin.fileName());
        QVariantList allArgs = {QVariant::fromValue(p), loader.metaData().toVariantMap(), appletId};
        allArgs << args;
        if (KPluginFactory *factory = KPluginFactory::loadFactory(plugin).plugin) {
            if (factory->metaData().rawData().isEmpty()) {
                factory->setMetaData(p.metadata());
            }
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

        if (isContainmentMetaData(p.metadata())) {
            applet = new Containment(nullptr, p.metadata(), allArgs);
        } else {
            applet = new Applet(nullptr, p.metadata(), allArgs);
        }
    }

    const QString localePath = p.filePath("translations");
    if (!localePath.isEmpty()) {
        KLocalizedString::addDomainLocaleDir(QByteArray("plasma_applet_") + name.toLatin1(), localePath);
    }
    return applet;
}

#if PLASMA_BUILD_DEPRECATED_SINCE(5, 94)
DataEngine *PluginLoader::loadDataEngine(const QString &name)
{
#if PLASMA_BUILD_DEPRECATED_SINCE(5, 86)
    DataEngine *engine = d->isDefaultLoader ? nullptr : internalLoadDataEngine(name);
    if (engine) {
        return engine;
    }
#else
    DataEngine *engine = nullptr;
#endif

    // Look for C++ plugins first
    KPluginMetaData plugin = d->dataengineCache.findPluginById(name, PluginLoaderPrivate::s_dataEnginePluginDir);
    if (plugin.isValid()) {
        const QVariantList args{QVariant::fromValue(plugin)};
        engine = KPluginFactory::instantiatePlugin<Plasma::DataEngine>(plugin, nullptr, args).plugin;
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
#endif

#if PLASMA_BUILD_DEPRECATED_SINCE(5, 94)
QStringList PluginLoader::listAllEngines(const QString &parentApp)
{
    QStringList engines;
    // Look for C++ plugins first
    auto filter = [&parentApp](const KPluginMetaData &md) -> bool {
        return md.value(QStringLiteral("X-KDE-ParentApp")) == parentApp;
    };
    QVector<KPluginMetaData> plugins;
    if (parentApp.isEmpty()) {
        plugins = KPluginMetaData::findPlugins(PluginLoaderPrivate::s_dataEnginePluginDir);
    } else {
        plugins = KPluginMetaData::findPlugins(PluginLoaderPrivate::s_dataEnginePluginDir, filter);
    }

    for (auto &plugin : std::as_const(plugins)) {
        engines << plugin.pluginId();
    }

    const QList<KPluginMetaData> packagePlugins = KPackage::PackageLoader::self()->listPackages(QStringLiteral("Plasma/DataEngine"));
    for (const auto &plugin : packagePlugins) {
        engines << plugin.pluginId();
    }

    return engines;
}
#endif

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
        plugins = KPluginMetaData::findPlugins(PluginLoaderPrivate::s_dataEnginePluginDir, filterNormal);
    } else {
        plugins = KPluginMetaData::findPlugins(PluginLoaderPrivate::s_dataEnginePluginDir, filterParentApp);
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
#if PLASMA_BUILD_DEPRECATED_SINCE(5, 86)
    Service *service = d->isDefaultLoader ? nullptr : internalLoadService(name, args, parent);
    if (service) {
        return service;
    }
#else
    Service *service = nullptr;
#endif

    // TODO: scripting API support
    if (name.isEmpty()) {
        return new NullService(QString(), parent);
    } else if (name == QLatin1String("org.kde.servicestorage")) {
        return new Storage(parent);
    }

    // Look for C++ plugins first
    KPluginMetaData plugin = KPluginMetaData::findPluginById(PluginLoaderPrivate::s_servicesPluginDir, name);
    if (plugin.isValid()) {
        service = KPluginFactory::instantiatePlugin<Plasma::Service>(plugin, parent, args).plugin;
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
#if PLASMA_BUILD_DEPRECATED_SINCE(5, 86)
    ContainmentActions *actions = d->isDefaultLoader ? nullptr : internalLoadContainmentActions(parent, name, args);
    if (actions) {
        return actions;
    }
#endif

    KPluginMetaData plugin = d->containmentactionCache.findPluginById(name, PluginLoaderPrivate::s_containmentActionsPluginDir);

    if (plugin.isValid()) {
        if (auto res = KPluginFactory::instantiatePlugin<Plasma::ContainmentActions>(plugin, nullptr, {QVariant::fromValue(plugin)})) {
            return res.plugin;
        }
    }

#if PLASMA_BUILD_DEPRECATED_SINCE(5, 88)
    QString constraint = QStringLiteral("[X-KDE-PluginInfo-Name] == '%1'").arg(name);
    KService::List offers = KServiceTypeTrader::self()->query(QStringLiteral("Plasma/ContainmentActions"), constraint);

    if (offers.isEmpty()) {
#ifndef NDEBUG
        qCDebug(LOG_PLASMA) << "offers is empty for " << name;
#endif
        return nullptr;
    }

    KService::Ptr offer = offers.first();
    qCWarning(LOG_PLASMA) << "Plugin" << name << "was loaded using deprecated KServiceTypeTrader."
                          << "Use embedded json metadata and install the plugin in plasma/containmentactions namespace instead";

    KPluginMetaData data(offer->library());
    QVariantList allArgs;
    allArgs << offer->storageId() << args;

    return KPluginFactory::instantiatePlugin<Plasma::ContainmentActions>(data, nullptr, allArgs).plugin;
#else
    return nullptr;
#endif
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

        const QVector<KPluginMetaData> plugins = KPluginMetaData::findPlugins(PluginLoaderPrivate::s_packageStructurePluginDir, filter);

        if (!plugins.isEmpty()) {
            if (auto res = KPluginFactory::instantiatePlugin<Plasma::PackageStructure>(plugins.first())) {
                structure = res.plugin;
            } else {
                qWarning() << "Error loading plugin:" << res.errorString;
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

    return Package();
}
#endif
QList<KPluginMetaData> listAppletMetaDataInternal(const QString &category, const QString &parentApp)
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

    return KPackage::PackageLoader::self()->findPackages(QStringLiteral("Plasma/Applet"), QString(), filter);
}

QList<KPluginMetaData> PluginLoader::listAppletMetaData(const QString &category)
{
    return listAppletMetaDataInternal(category, QString());
}

#if PLASMA_BUILD_DEPRECATED_SINCE(5, 88)
QList<KPluginMetaData> PluginLoader::listAppletMetaData(const QString &category, const QString &parentApp)
{
    return listAppletMetaDataInternal(category, parentApp);
}
#endif

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
        return md.value(QStringLiteral("X-Plasma-DropMimeTypes"), QStringList()).contains(mimeType);
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
            && !md.value(QStringLiteral("X-Plasma-DropUrlPatterns"), QStringList()).isEmpty();
    };
    const QList<KPluginMetaData> allApplets = KPackage::PackageLoader::self()->findPackages(QStringLiteral("Plasma/Applet"), QString(), filter);

    QList<KPluginMetaData> filtered;
    for (const KPluginMetaData &md : allApplets) {
        const QStringList urlPatterns = md.value(QStringLiteral("X-Plasma-DropUrlPatterns"), QStringList());
        for (const QString &glob : urlPatterns) {
            QRegExp rx(glob);
            rx.setPatternSyntax(QRegExp::Wildcard);
            if (rx.exactMatch(url.toString())) {
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
        return isContainmentMetaData(md) && filter(md);
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
    auto filter = [&type, &category, &parentApp](const KPluginMetaData &md) -> bool {
        if (isContainmentMetaData(md)) {
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
        return isContainmentMetaData(md) && md.value(QStringLiteral("X-Plasma-DropMimeTypes"), QStringList()).contains(mimeType);
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
        plugins = KPluginMetaData::findPlugins(PluginLoaderPrivate::s_dataEnginePluginDir);
    } else {
        plugins = KPluginMetaData::findPlugins(PluginLoaderPrivate::s_dataEnginePluginDir, filter);
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
        plugins = KPluginMetaData::findPlugins(PluginLoaderPrivate::s_containmentActionsPluginDir);
    } else {
        plugins = KPluginMetaData::findPlugins(PluginLoaderPrivate::s_containmentActionsPluginDir, filter);
    }

#if KSERVICE_BUILD_DEPRECATED_SINCE(5, 0)
    QSet<QString> knownPlugins;
    for (const KPluginMetaData &p : std::as_const(plugins)) {
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

#if PLASMA_BUILD_DEPRECATED_SINCE(5, 86)
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
#endif

#if PLASMA_BUILD_DEPRECATED_SINCE(5, 88)
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
#endif

KPluginMetaData PluginLoaderPrivate::Cache::findPluginById(const QString &name, const QString &pluginNamespace)
{
    const qint64 now = qRound64(QDateTime::currentMSecsSinceEpoch() / 1000.0);
    bool useRuntimeCache = true;

    if (pluginCacheAge == 0) {
        // Find all the plugins now, but only once
        pluginCacheAge = now;

        const auto metaDataList = KPluginMetaData::findPlugins(pluginNamespace, {}, KPluginMetaData::AllowEmptyMetaData);
        for (const KPluginMetaData &metadata : metaDataList) {
            plugins.insert(metadata.pluginId(), metadata);
        }
    } else if (now - pluginCacheAge > maxCacheAge) {
        // cache is old and we're not within a few seconds of startup anymore
        useRuntimeCache = false;
        plugins.clear();
    }

    // if name wasn't a path, pluginName == name
    const QString pluginName = name.section(QLatin1Char('/'), -1);

    if (useRuntimeCache) {
        KPluginMetaData data = plugins.value(name);
        qCDebug(LOG_PLASMA) << "loading applet by name" << name << useRuntimeCache << data.isValid();
        return data;
    } else {
        const QVector<KPluginMetaData> offers = KPluginMetaData::findPlugins(
            pluginNamespace,
            [&pluginName](const KPluginMetaData &data) {
                return data.pluginId() == pluginName;
            },
            KPluginMetaData::AllowEmptyMetaData);
        return offers.isEmpty() ? KPluginMetaData() : offers.first();
    }
}

} // Plasma Namespace
