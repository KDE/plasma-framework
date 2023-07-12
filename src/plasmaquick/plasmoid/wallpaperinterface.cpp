/*
    SPDX-FileCopyrightText: 2013 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "wallpaperinterface.h"
#include "appletcontext_p.h"

#include "containmentinterface.h"
#include "sharedqmlengine.h"

#include <KConfigPropertyMap>
#include <KActionCollection>
#include <KConfigLoader>
#include <KDesktopFile>

#include <QDebug>
#include <QFile>
#include <QQmlContext>
#include <QQmlExpression>
#include <QQmlProperty>

#include <Plasma/Corona>
#include <Plasma/PluginLoader>
#include <kpackage/packageloader.h>
#include <qabstractitemmodel.h>
#include <qtmetamacros.h>

WallpaperInterface::WallpaperInterface(QQuickItem *parent)
    : QQuickItem(parent)
{
    // resize at the beginning to avoid as much resize events as possible
    if (parent) {
        setSize(QSizeF(parent->width(), parent->height()));
    }
}

WallpaperInterface::~WallpaperInterface()
{
}

void WallpaperInterface::classBegin()
{
    QQuickItem::classBegin();
    PlasmaQuick::AppletContext *ac = qobject_cast<PlasmaQuick::AppletContext *>(QQmlEngine::contextForObject(this)->parentContext());
    Q_ASSERT(ac);
    m_containment = ac->applet()->containment();
    m_wallpaperPlugin = m_containment->wallpaper();
    m_qmlObject = ac->sharedQmlEngine();
    m_qmlObject->setParent(this);

    m_pkg = KPackage::PackageLoader::self()->loadPackage(QStringLiteral("Plasma/Wallpaper"));
    m_pkg.setPath(m_wallpaperPlugin);

    if (configScheme()) {
        m_configuration = new KConfigPropertyMap(configScheme(), this);
    }

    connect(m_containment->corona(), &Plasma::Corona::startupCompleted, this, std::bind(&WallpaperInterface::repaintNeeded, this, Qt::transparent));
}

void WallpaperInterface::componentComplete()
{
    QQuickItem::componentComplete();

    m_loading = false;
    Q_EMIT isLoadingChanged();
}

QList<KPluginMetaData> WallpaperInterface::listWallpaperMetadataForMimetype(const QString &mimetype, const QString &formFactor)
{
    auto filter = [&mimetype, &formFactor](const KPluginMetaData &md) -> bool {
        if (!formFactor.isEmpty() && !md.value(QStringLiteral("X-Plasma-FormFactors")).contains(formFactor)) {
            return false;
        }
        return md.value(QStringLiteral("X-Plasma-DropMimeTypes"), QStringList()).contains(mimetype);
    };
    return KPackage::PackageLoader::self()->findPackages(QStringLiteral("Plasma/Wallpaper"), QString(), filter);
}

KPackage::Package WallpaperInterface::kPackage() const
{
    return m_pkg;
}

QString WallpaperInterface::pluginName() const
{
    return m_wallpaperPlugin;
}

KConfigPropertyMap *WallpaperInterface::configuration() const
{
    return m_configuration;
}

KConfigLoader *WallpaperInterface::configScheme()
{
    if (!m_configLoader) {
        // FIXME: do we need "mainconfigxml" in wallpaper packagestructures?
        const QString xmlPath = m_pkg.filePath("config", QStringLiteral("main.xml"));

        KConfigGroup cfg = m_containment->config();
        cfg = KConfigGroup(&cfg, "Wallpaper");
        cfg = KConfigGroup(&cfg, m_wallpaperPlugin);

        if (xmlPath.isEmpty()) {
            m_configLoader = new KConfigLoader(cfg, nullptr, this);
        } else {
            QFile file(xmlPath);
            m_configLoader = new KConfigLoader(cfg, &file, this);
        }
    }

    return m_configLoader;
}

void WallpaperInterface::requestOpenUrl(const QUrl &url)
{
    Q_EMIT openUrlRequested(url);
}

WallpaperInterface *WallpaperInterface::loadWallpaper(ContainmentInterface *containmentInterface)
{
    if (containmentInterface->containment()->wallpaper().isEmpty()) {
        return nullptr;
    }
    KPackage::Package pkg = KPackage::PackageLoader::self()->loadPackage(QStringLiteral("Plasma/Wallpaper"));
    pkg.setPath(containmentInterface->containment()->wallpaper());
    if (!pkg.isValid()) {
        qWarning() << "Error loading the wallpaper, no valid package loaded";
        return nullptr;
    }

    PlasmaQuick::SharedQmlEngine *qmlObject = new PlasmaQuick::SharedQmlEngine(containmentInterface->containment());
    qmlObject->setInitializationDelayed(true);

    const QString rootPath = pkg.metadata().value(QStringLiteral("X-Plasma-RootPath"));
    if (!rootPath.isEmpty()) {
        qmlObject->setTranslationDomain(QLatin1String("plasma_wallpaper_") + rootPath);
    } else {
        qmlObject->setTranslationDomain(QLatin1String("plasma_wallpaper_") + pkg.metadata().pluginId());
    }

    /*
     * The initialization is delayed, so it's fine to setSource first.
     * This also prevents many undefined wallpaper warnings caused by "wallpaper" being set
     * when the old wallpaper plugin still exists.
     */
    qmlObject->setSource(pkg.fileUrl("mainscript"));
    WallpaperInterface *wallpaper = qobject_cast<WallpaperInterface *>(qmlObject->rootObject());
    if (!wallpaper) {
        if (qmlObject->mainComponent() && qmlObject->mainComponent()->isError()) {
            qWarning() << "Error loading the wallpaper" << qmlObject->mainComponent()->errors();
        } else if (qmlObject->rootObject()) {
            qWarning() << "Root item of wallpaper" << containmentInterface->containment()->wallpaper() << "not a WallpaperInterface instance, instead is"
                       << qmlObject->rootObject();
        }
        delete qmlObject->rootObject();
        return nullptr;
    }

    if (!qEnvironmentVariableIntValue("PLASMA_NO_CONTEXTPROPERTIES")) {
        qmlObject->rootContext()->setContextProperty(QStringLiteral("wallpaper"), wallpaper);
    }

    // initialize with our size to avoid as much resize events as possible
    QVariantHash props;
    props[QStringLiteral("parent")] = QVariant::fromValue(containmentInterface);
    props[QStringLiteral("width")] = containmentInterface->width();
    props[QStringLiteral("height")] = containmentInterface->height();
    qmlObject->completeInitialization(props);
    return wallpaper;
}

QList<QAction *> WallpaperInterface::contextualActions() const
{
    return m_contextualActions;
}

QQmlListProperty<QAction> WallpaperInterface::qmlContextualActions()
{
    return QQmlListProperty<QAction>(this,
                                     nullptr,
                                     WallpaperInterface::contextualActions_append,
                                     WallpaperInterface::contextualActions_count,
                                     WallpaperInterface::contextualActions_at,
                                     WallpaperInterface::contextualActions_clear,
                                     WallpaperInterface::contextualActions_replace,
                                     WallpaperInterface::contextualActions_removeLast);
}

bool WallpaperInterface::supportsMimetype(const QString &mimetype) const
{
    return m_pkg.metadata().value(QStringLiteral("X-Plasma-DropMimeTypes"), QStringList()).contains(mimetype);
}

bool WallpaperInterface::isLoading() const
{
    return m_loading;
}

void WallpaperInterface::contextualActions_append(QQmlListProperty<QAction> *prop, QAction *action)
{
    WallpaperInterface *w = static_cast<WallpaperInterface *>(prop->object);
    w->m_contextualActions.append(action);
    Q_EMIT w->contextualActionsChanged(w->m_contextualActions);
};

qsizetype WallpaperInterface::contextualActions_count(QQmlListProperty<QAction> *prop)
{
    WallpaperInterface *w = static_cast<WallpaperInterface *>(prop->object);
    return w->m_contextualActions.count();
}

QAction *WallpaperInterface::contextualActions_at(QQmlListProperty<QAction> *prop, qsizetype idx)
{
    WallpaperInterface *w = static_cast<WallpaperInterface *>(prop->object);
    return w->m_contextualActions.value(idx);
}

void WallpaperInterface::contextualActions_clear(QQmlListProperty<QAction> *prop)
{
    WallpaperInterface *w = static_cast<WallpaperInterface *>(prop->object);
    w->m_contextualActions.clear();
    Q_EMIT w->contextualActionsChanged(w->m_contextualActions);
}

void WallpaperInterface::contextualActions_replace(QQmlListProperty<QAction> *prop, qsizetype idx, QAction *action)
{
    WallpaperInterface *w = static_cast<WallpaperInterface *>(prop->object);
    w->m_contextualActions.replace(idx, action);
    Q_EMIT w->contextualActionsChanged(w->m_contextualActions);
}

void WallpaperInterface::contextualActions_removeLast(QQmlListProperty<QAction> *prop)
{
    WallpaperInterface *w = static_cast<WallpaperInterface *>(prop->object);
    w->m_contextualActions.pop_back();
    Q_EMIT w->contextualActionsChanged(w->m_contextualActions);
}

#include "moc_wallpaperinterface.cpp"
