/*
    SPDX-FileCopyrightText: 2010 Ryan Rix <ry@n.rix.si>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "pluginloader.h"

#include <QGuiApplication>
#include <QPluginLoader>
#include <QPointer>
#include <QStandardPaths>

#include <KLazyLocalizedString>
#include <KRuntimePlatform>
#include <QDebug>
#include <QRegularExpression>
#include <kcoreaddons_export.h>
#include <kpackage/packageloader.h>

#include "config-plasma.h"

#include "applet.h"
#include "containment.h"
#include "containmentactions.h"
#include "debug_p.h"
#include "private/applet_p.h"

namespace Plasma
{
inline bool isContainmentMetaData(const KPluginMetaData &md)
{
    return md.rawData().contains(QStringLiteral("X-Plasma-ContainmentType"));
}

class PluginLoaderPrivate
{
public:
    PluginLoaderPrivate()
    {
    }

    static QString s_plasmoidsPluginDir;
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
    Cache containmentactionCache;
};

QString PluginLoaderPrivate::s_plasmoidsPluginDir = QStringLiteral("plasma/applets");
QString PluginLoaderPrivate::s_containmentActionsPluginDir = QStringLiteral("plasma/containmentactions");

PluginLoader::PluginLoader()
    : d(new PluginLoaderPrivate)
{
}

PluginLoader::~PluginLoader()
{
    delete d;
}

PluginLoader *PluginLoader::self()
{
    static PluginLoader self;
    return &self;
}

Applet *PluginLoader::loadApplet(const QString &name, uint appletId, const QVariantList &args)
{
    if (name.isEmpty()) {
        return nullptr;
    }

    Applet *applet = nullptr;

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

    QVariantList allArgs;
    allArgs << QVariant::fromValue(p) << p.metadata().fileName() << appletId << args;

    if (isContainmentMetaData(p.metadata())) {
        applet = new Containment(nullptr, p.metadata(), allArgs);
    } else {
        applet = new Applet(nullptr, p.metadata(), allArgs);
    }

    const QString localePath = p.filePath("translations");
    if (!localePath.isEmpty()) {
        KLocalizedString::addDomainLocaleDir(QByteArray("plasma_applet_") + name.toLatin1(), localePath);
    }
    return applet;
}

ContainmentActions *PluginLoader::loadContainmentActions(Containment *parent, const QString &name, const QVariantList &args)
{
    if (name.isEmpty()) {
        return nullptr;
    }

    KPluginMetaData plugin = d->containmentactionCache.findPluginById(name, PluginLoaderPrivate::s_containmentActionsPluginDir);

    if (plugin.isValid()) {
        if (auto res = KPluginFactory::instantiatePlugin<Plasma::ContainmentActions>(plugin, nullptr, {QVariant::fromValue(plugin)})) {
            return res.plugin;
        }
    }

    return nullptr;
}

QList<KPluginMetaData> listAppletMetaDataInternal(const QString &category, const QString &parentApp)
{
    auto platforms = KRuntimePlatform::runtimePlatform();
    // For now desktop always lists everything
    if (platforms.contains(QStringLiteral("desktop"))) {
        platforms.clear();
    }

    // FIXME: this assumes we are always use packages.. no pure c++
    std::function<bool(const KPluginMetaData &)> filter;
    if (category.isEmpty()) { // use all but the excluded categories
        KConfigGroup group(KSharedConfig::openConfig(), "General");
        QStringList excluded = group.readEntry("ExcludeCategories", QStringList());

        filter = [excluded, platforms](const KPluginMetaData &md) -> bool {
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

            return !excluded.contains(md.category());
        };
    } else { // specific category (this could be an excluded one - is that bad?)

        filter = [category, platforms](const KPluginMetaData &md) -> bool {
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

            if (category == QLatin1String("Miscellaneous")) {
                return md.category() == category || md.category().isEmpty();
            } else {
                return md.category() == category;
            }
        };
    }

    return KPackage::PackageLoader::self()->findPackages(QStringLiteral("Plasma/Applet"), QString(), filter);
}

QList<KPluginMetaData> PluginLoader::listAppletMetaData(const QString &category)
{
    return listAppletMetaDataInternal(category, QString());
}

QList<KPluginMetaData> PluginLoader::listAppletMetaDataForMimeType(const QString &mimeType)
{
    auto filter = [&mimeType](const KPluginMetaData &md) -> bool {
        return md.value(QStringLiteral("X-Plasma-DropMimeTypes"), QStringList()).contains(mimeType);
    };
    return KPackage::PackageLoader::self()->findPackages(QStringLiteral("Plasma/Applet"), QString(), filter);
}

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
            QRegularExpression rx(QRegularExpression::anchoredPattern(QRegularExpression::wildcardToRegularExpression(glob)));
            if (rx.match(url.toString()).hasMatch()) {
                filtered << md;
            }
        }
    }

    return filtered;
}

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

    return plugins;
}

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
