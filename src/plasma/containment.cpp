/*
    SPDX-FileCopyrightText: 2007 Aaron Seigo <aseigo@kde.org>
    SPDX-FileCopyrightText: 2008 Ménard Alexis <darktears31@gmail.com>
    SPDX-FileCopyrightText: 2009 Chani Armitage <chani@kde.org>
    SPDX-FileCopyrightText: 2012 Marco Martin <notmart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "containment.h"
#include "private/containment_p.h"

#include "config-plasma.h"

#include <QClipboard>
#include <QContextMenuEvent>
#include <QDebug>
#include <QFile>
#include <QMimeData>
#include <QMimeDatabase>
#include <QPainter>
#include <QTemporaryFile>

#include <KAuthorized>
#include <KConfigLoader>
#include <KConfigSkeleton>
#include <KLocalizedString>

#include "containmentactions.h"
#include "corona.h"
#include "debug_p.h"
#include "pluginloader.h"

#include "private/applet_p.h"

#include "plasma/plasma.h"

namespace Plasma
{
Containment::Containment(QObject *parentObject, const KPluginMetaData &data, const QVariantList &args)
    : Applet(parentObject, data, args)
    , d(new ContainmentPrivate(this))
{
    // WARNING: do not access config() OR globalConfig() in this method!
    //          that requires a scene, which is not available at this point
    setHasConfigurationInterface(true);
}

#if PLASMA_BUILD_DEPRECATED_SINCE(5, 86)
Containment::Containment(QObject *parent, const QString &serviceId, uint containmentId)
    : Applet(parent, serviceId, containmentId)
    , d(new ContainmentPrivate(this))
{
    // WARNING: do not access config() OR globalConfig() in this method!
    //          that requires a scene, which is not available at this point
    setContainmentType(Types::CustomContainment);
    setHasConfigurationInterface(true);
}

Containment::Containment(QObject *parent, const QVariantList &args)
    : Applet(parent, args)
    , d(new ContainmentPrivate(this))
{
    // WARNING: do not access config() OR globalConfig() in this method!
    //          that requires a scene, which is not available at this point
    setHasConfigurationInterface(true);
}

Containment::Containment(const KPluginMetaData &md, uint appletId)
    : Applet(md, nullptr, appletId)
    , d(new ContainmentPrivate(this))
{
    // WARNING: do not access config() OR globalConfig() in this method!
    //          that requires a scene, which is not available at this point
    setHasConfigurationInterface(true);
}
#endif

Containment::~Containment()
{
    qDeleteAll(d->localActionPlugins);
    delete d;
}

void Containment::init()
{
    Applet::init();
    static_cast<Applet *>(this)->d->setupScripting();

    if (d->type == Types::NoContainmentType) {
        // setContainmentType(Plasma::Types::DesktopContainment);
        // Try to determine the containment type. It must be done as soon as possible
        QString type = pluginMetaData().value(QStringLiteral("X-Plasma-ContainmentType"));

        if (type == QLatin1String("Panel")) {
            setContainmentType(Plasma::Types::PanelContainment);
        } else if (type == QLatin1String("Custom")) {
            setContainmentType(Plasma::Types::CustomContainment);
        } else if (type == QLatin1String("CustomPanel")) {
            setContainmentType(Plasma::Types::CustomPanelContainment);
            // default to desktop
        } else {
            setContainmentType(Plasma::Types::DesktopContainment);
        }
    }

    // connect actions
    ContainmentPrivate::addDefaultActions(actions(), this);
    bool unlocked = immutability() == Types::Mutable;

    // fix the text of the actions that need title()
    // btw, do we really want to use title() when it's a desktopcontainment?
    QAction *closeApplet = actions()->action(QStringLiteral("remove"));
    if (closeApplet) {
        closeApplet->setText(i18nc("%1 is the name of the applet", "Remove %1", title()));
    }

    QAction *configAction = actions()->action(QStringLiteral("configure"));
    if (configAction) {
        if (d->type == Types::PanelContainment || d->type == Types::CustomPanelContainment) {
            configAction->setText(i18n("Enter Edit Mode"));
            configAction->setIcon(QIcon::fromTheme(QStringLiteral("document-edit")));
        } else {
            configAction->setText(i18nc("%1 is the name of the applet", "Configure %1...", title()));
        }
    }

    QAction *appletBrowserAction = actions()->action(QStringLiteral("add widgets"));
    if (appletBrowserAction) {
        appletBrowserAction->setVisible(unlocked);
        appletBrowserAction->setEnabled(unlocked);
        connect(appletBrowserAction, SIGNAL(triggered()), this, SLOT(triggerShowAddWidgets()));
    }

    if (immutability() != Types::SystemImmutable && corona()) {
        QAction *lockDesktopAction = corona()->actions()->action(QStringLiteral("lock widgets"));
        // keep a pointer so nobody notices it moved to corona
        if (lockDesktopAction) {
            actions()->addAction(QStringLiteral("lock widgets"), lockDesktopAction);
        }
    }

    // HACK: this is valid only in the systray case
    connect(this, &Containment::configureRequested, this, [=](Plasma::Applet *a) {
        if (Plasma::Applet *p = qobject_cast<Plasma::Applet *>(parent())) {
            Q_EMIT p->containment()->configureRequested(a);
        }
    });
}

// helper function for sorting the list of applets
bool appletConfigLessThan(const KConfigGroup &c1, const KConfigGroup &c2)
{
    int i1 = c1.readEntry("id", 0);
    int i2 = c2.readEntry("id", 0);

    return (i1 < i2);
}

void Containment::restore(KConfigGroup &group)
{
    /*
    #ifndef NDEBUG
    // qCDebug(LOG_PLASMA) << "!!!!!!!!!!!!initConstraints" << group.name() << d->type;
    // qCDebug(LOG_PLASMA) << "    location:" << group.readEntry("location", (int)d->location);
    // qCDebug(LOG_PLASMA) << "    geom:" << group.readEntry("geometry", geometry());
    // qCDebug(LOG_PLASMA) << "    formfactor:" << group.readEntry("formfactor", (int)d->formFactor);
    // qCDebug(LOG_PLASMA) << "    screen:" << group.readEntry("screen", d->screen);
    #endif
    */
    setLocation((Plasma::Types::Location)group.readEntry("location", (int)d->location));
    setFormFactor((Plasma::Types::FormFactor)group.readEntry("formfactor", (int)d->formFactor));
    d->lastScreen = group.readEntry("lastScreen", d->lastScreen);

    setWallpaper(group.readEntry("wallpaperplugin", ContainmentPrivate::defaultWallpaper));

    d->activityId = group.readEntry("activityId", QString());

    flushPendingConstraintsEvents();
    restoreContents(group);
    setImmutability((Types::ImmutabilityType)group.readEntry("immutability", (int)Types::Mutable));

    if (isContainment() && KAuthorized::authorize(QStringLiteral("plasma/containment_actions"))) {
        KConfigGroup cfg = KConfigGroup(corona()->config(), "ActionPlugins");
        cfg = KConfigGroup(&cfg, QString::number(containmentType()));

        // qCDebug(LOG_PLASMA) << cfg.keyList();
        if (cfg.exists()) {
            const auto keyList = cfg.keyList();
            for (const QString &key : keyList) {
                // qCDebug(LOG_PLASMA) << "loading" << key;
                setContainmentActions(key, cfg.readEntry(key, QString()));
            }
        } else { // shell defaults
            KConfigGroup defaultActionsCfg;

            switch (d->type) {
            case Plasma::Types::PanelContainment:
                /* fall through*/
            case Plasma::Types::CustomPanelContainment:
                defaultActionsCfg = KConfigGroup(KSharedConfig::openConfig(corona()->kPackage().filePath("defaults")), "Panel");
                break;
            case Plasma::Types::DesktopContainment:
                defaultActionsCfg = KConfigGroup(KSharedConfig::openConfig(corona()->kPackage().filePath("defaults")), "Desktop");
                break;
            default:
                // for any other type of containment, there are no defaults
                break;
            }
            if (defaultActionsCfg.isValid()) {
                defaultActionsCfg = KConfigGroup(&defaultActionsCfg, "ContainmentActions");
                const auto keyList = defaultActionsCfg.keyList();
                for (const QString &key : keyList) {
                    setContainmentActions(key, defaultActionsCfg.readEntry(key, QString()));
                }
            }
        }
    }
    Applet::restore(group);
}

void Containment::save(KConfigGroup &g) const
{
    if (Applet::d->transient) {
        return;
    }

    KConfigGroup group = g;
    if (!group.isValid()) {
        group = config();
    }

    // locking is saved in Applet::save
    Applet::save(group);

    //     group.writeEntry("screen", d->screen);
    group.writeEntry("lastScreen", d->lastScreen);
    group.writeEntry("formfactor", (int)d->formFactor);
    group.writeEntry("location", (int)d->location);
    group.writeEntry("activityId", d->activityId);

    group.writeEntry("wallpaperplugin", d->wallpaper);

    saveContents(group);
}

void Containment::saveContents(KConfigGroup &group) const
{
    KConfigGroup applets(&group, "Applets");
    for (const Applet *applet : std::as_const(d->applets)) {
        KConfigGroup appletConfig(&applets, QString::number(applet->id()));
        applet->save(appletConfig);
    }
}

void Containment::restoreContents(KConfigGroup &group)
{
    KConfigGroup applets(&group, "Applets");

    // restore the applets ordered by id
    QStringList groups = applets.groupList();
    std::sort(groups.begin(), groups.end());

    // Sort the applet configs in order of geometry to ensure that applets
    // are added from left to right or top to bottom for a panel containment
    QList<KConfigGroup> appletConfigs;
    for (const QString &appletGroup : std::as_const(groups)) {
        // qCDebug(LOG_PLASMA) << "reading from applet group" << appletGroup;
        KConfigGroup appletConfig(&applets, appletGroup);
        appletConfigs.append(appletConfig);
    }
    std::stable_sort(appletConfigs.begin(), appletConfigs.end(), appletConfigLessThan);

    QMutableListIterator<KConfigGroup> it(appletConfigs);
    while (it.hasNext()) {
        KConfigGroup &appletConfig = it.next();
        if (appletConfig.readEntry(QStringLiteral("transient"), false)) {
            appletConfig.deleteGroup();
            continue;
        }
        int appId = appletConfig.name().toUInt();
        QString plugin = appletConfig.readEntry("plugin", QString());

        if (plugin.isEmpty()) {
            continue;
        }

        d->createApplet(plugin, QVariantList(), appId);
    }

    // if there are no applets, none of them is "loading"
    if (Containment::applets().isEmpty()) {
        d->appletsUiReady = true;
    }
    const auto lstApplets = Containment::applets();
    for (Applet *applet : lstApplets) {
        if (!applet->pluginMetaData().isValid()) {
            applet->updateConstraints(Plasma::Types::UiReadyConstraint);
        }
    }
}

Plasma::Types::ContainmentType Containment::containmentType() const
{
    return d->type;
}

void Containment::setContainmentType(Plasma::Types::ContainmentType type)
{
    if (d->type == type) {
        return;
    }

    d->type = type;
    Q_EMIT containmentTypeChanged();
}

Corona *Containment::corona() const
{
    if (Plasma::Corona *corona = qobject_cast<Corona *>(parent())) {
        return corona;
        // case in which this containment is child of an applet, hello systray :)
    } else {
        Plasma::Applet *parentApplet = qobject_cast<Plasma::Applet *>(parent());
        if (parentApplet && parentApplet->containment()) {
            return parentApplet->containment()->corona();
        }
    }

    return nullptr;
}

void Containment::setFormFactor(Types::FormFactor formFactor)
{
    if (d->formFactor == formFactor) {
        return;
    }

    // qCDebug(LOG_PLASMA) << "switching FF to " << formFactor;
    d->formFactor = formFactor;

    updateConstraints(Plasma::Types::FormFactorConstraint);

    KConfigGroup c = config();
    c.writeEntry("formfactor", (int)formFactor);
    Q_EMIT configNeedsSaving();
    Q_EMIT formFactorChanged(formFactor);
}

void Containment::setContainmentDisplayHints(Types::ContainmentDisplayHints hints)
{
    if (d->containmentDisplayHints == hints) {
        return;
    }

    d->containmentDisplayHints = hints;
    Q_EMIT containmentDisplayHintsChanged(hints);
}

void Containment::setLocation(Types::Location location)
{
    if (d->location == location) {
        return;
    }

    d->location = location;

    for (Applet *applet : std::as_const(d->applets)) {
        applet->updateConstraints(Plasma::Types::LocationConstraint);
    }

    updateConstraints(Plasma::Types::LocationConstraint);

    KConfigGroup c = config();
    c.writeEntry("location", (int)location);
    Q_EMIT configNeedsSaving();
    Q_EMIT locationChanged(location);
}

Applet *Containment::createApplet(const QString &name, const QVariantList &args)
{
    Plasma::Applet *applet = d->createApplet(name, args);
    if (applet) {
        Q_EMIT appletCreated(applet);
    }
    return applet;
}

void Containment::addApplet(Applet *applet)
{
    if (!applet) {
#ifndef NDEBUG
        // qCDebug(LOG_PLASMA) << "adding null applet!?!";
#endif
        return;
    }

    if (immutability() != Types::Mutable && !applet->property("org.kde.plasma:force-create").toBool()) {
        return;
    }

#ifndef NDEBUG
    if (d->applets.contains(applet)) {
        // qCDebug(LOG_PLASMA) << "already have this applet!";
    }
#endif

    Containment *currentContainment = applet->containment();

    if (currentContainment && currentContainment != this) {
        Q_EMIT currentContainment->appletRemoved(applet);

        disconnect(applet, nullptr, currentContainment, nullptr);
        connect(currentContainment, nullptr, applet, nullptr);
        KConfigGroup oldConfig = applet->config();
        currentContainment->d->applets.removeAll(applet);
        applet->setParent(this);

        // now move the old config to the new location
        // FIXME: this doesn't seem to get the actual main config group containing plugin=, etc
        KConfigGroup c = config().group("Applets").group(QString::number(applet->id()));
        oldConfig.reparent(&c);
        applet->d->resetConfigurationObject();

        disconnect(applet, &Applet::activated, currentContainment, &Applet::activated);
        // change the group to its configloader, if any
        // FIXME: this is very, very brutal
        if (applet->configScheme()) {
            const QString oldGroupPrefix = QStringLiteral("Containments") + QString::number(currentContainment->id()) + QStringLiteral("Applets");
            const QString newGroupPrefix = QStringLiteral("Containments") + QString::number(id()) + QStringLiteral("Applets");

            applet->configScheme()->setCurrentGroup(applet->configScheme()->currentGroup().replace(0, oldGroupPrefix.length(), newGroupPrefix));

            const auto items = applet->configScheme()->items();
            for (KConfigSkeletonItem *item : items) {
                item->setGroup(item->group().replace(0, oldGroupPrefix.length(), newGroupPrefix));
            }
        }
    } else {
        applet->setParent(this);
    }

    // make sure the applets are sorted by id
    auto position = std::lower_bound(d->applets.begin(), d->applets.end(), applet, [](Plasma::Applet *a1, Plasma::Applet *a2) {
        return a1->id() < a2->id();
    });
    d->applets.insert(position, applet);

    if (!d->uiReady) {
        d->loadingApplets << applet;
    }

    connect(applet, &Applet::configNeedsSaving, this, &Applet::configNeedsSaving);
    connect(applet, SIGNAL(appletDeleted(Plasma::Applet *)), this, SLOT(appletDeleted(Plasma::Applet *)));
    connect(applet, SIGNAL(statusChanged(Plasma::Types::ItemStatus)), this, SLOT(checkStatus(Plasma::Types::ItemStatus)));
    connect(applet, &Applet::activated, this, &Applet::activated);
    connect(this, &Containment::containmentDisplayHintsChanged, applet, &Applet::containmentDisplayHintsChanged);

    if (!currentContainment) {
        const bool isNew = applet->d->mainConfigGroup()->entryMap().isEmpty();

        if (!isNew) {
            applet->restore(*applet->d->mainConfigGroup());
        }

        applet->init();
        applet->d->setupScripting();

        if (isNew) {
            applet->save(*applet->d->mainConfigGroup());
            Q_EMIT configNeedsSaving();
        }
        // FIXME: an on-appear animation would be nice to have again
    }

    applet->updateConstraints(Plasma::Types::AllConstraints);
    applet->flushPendingConstraintsEvents();

    Q_EMIT appletAdded(applet);

    if (!currentContainment) {
        applet->updateConstraints(Plasma::Types::StartupCompletedConstraint);
        applet->flushPendingConstraintsEvents();
    }

    applet->d->scheduleModificationNotification();
}

QList<Applet *> Containment::applets() const
{
    return d->applets;
}

int Containment::screen() const
{
    Q_ASSERT(corona());
    if (Corona *c = corona()) {
        return c->screenForContainment(this);
    } else {
        return -1;
    }
}

int Containment::lastScreen() const
{
    return d->lastScreen;
}

void Containment::setWallpaper(const QString &pluginName)
{
    if (pluginName != d->wallpaper) {
        d->wallpaper = pluginName;

        KConfigGroup cfg = config();
        cfg.writeEntry("wallpaperplugin", d->wallpaper);
        Q_EMIT configNeedsSaving();
        Q_EMIT wallpaperChanged();
    }
}

QString Containment::wallpaper() const
{
    return d->wallpaper;
}

void Containment::setContainmentActions(const QString &trigger, const QString &pluginName)
{
    KConfigGroup cfg = d->containmentActionsConfig();
    ContainmentActions *plugin = nullptr;

    plugin = containmentActions().value(trigger);
    if (plugin && plugin->metadata().pluginId() != pluginName) {
        containmentActions().remove(trigger);
        delete plugin;
        plugin = nullptr;
    }

    if (pluginName.isEmpty()) {
        cfg.deleteEntry(trigger);
    } else if (plugin) {
        // it already existed, just reload config
        plugin->setContainment(this); // to be safe
        // FIXME make a truly unique config group
        KConfigGroup pluginConfig = KConfigGroup(&cfg, trigger);
        plugin->restore(pluginConfig);

    } else {
        plugin = PluginLoader::self()->loadContainmentActions(this, pluginName);

        if (plugin) {
            cfg.writeEntry(trigger, pluginName);
            containmentActions().insert(trigger, plugin);
            plugin->setContainment(this);
            KConfigGroup pluginConfig = KConfigGroup(&cfg, trigger);
            plugin->restore(pluginConfig);
        } else {
            // bad plugin... gets removed. is this a feature or a bug?
            cfg.deleteEntry(trigger);
        }
    }

    Q_EMIT configNeedsSaving();
}

QHash<QString, ContainmentActions *> &Containment::containmentActions()
{
    return d->localActionPlugins;
}

bool Containment::isUiReady() const
{
    return d->uiReady && d->appletsUiReady && Applet::d->started;
}

void Containment::setActivity(const QString &activityId)
{
    if (activityId.isEmpty() || d->activityId == activityId) {
        return;
    }

    d->activityId = activityId;
    KConfigGroup c = config();
    c.writeEntry("activityId", activityId);

    Q_EMIT configNeedsSaving();
    Q_EMIT activityChanged(activityId);
}

QString Containment::activity() const
{
    return d->activityId;
}

void Containment::reactToScreenChange()
{
    int newScreen = screen();

    if (newScreen >= 0) {
        d->lastScreen = newScreen;
        KConfigGroup c = config();
        c.writeEntry("lastScreen", d->lastScreen);
        Q_EMIT configNeedsSaving();
    }

    Q_EMIT screenChanged(newScreen);
}

} // Plasma namespace

#include "moc_containment.cpp"
