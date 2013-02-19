/*
 *   Copyright 2007 by Aaron Seigo <aseigo@kde.org>
 *   Copyright 2008 by Ménard Alexis <darktears31@gmail.com>
 *   Copyright 2009 Chani Armitage <chani@kde.org>
 *   Copyright 2012 Marco Martin <notmart@kde.org>
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

#include "containment.h"
#include "private/containment_p.h"

#include "config-plasma.h"

#include <QApplication>
#include <QClipboard>
#include <QFile>
#include <QContextMenuEvent>
#include <QMimeData>
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <qtemporaryfile.h>
#include <qmimedatabase.h>

#include <kaction.h>
#include <kcoreauthorized.h>
#include <klocalizedstring.h>
#include <kservicetypetrader.h>

#if !PLASMA_NO_KIO
#include "kio/jobclasses.h" // for KIO::JobFlags
#include "kio/job.h"
#include "kio/scheduler.h"
#endif

#include "containmentactions.h"
#include "containmentactionspluginsconfig.h"
#include "corona.h"
#include "pluginloader.h"

#include "private/applet_p.h"
#include "private/containmentactionspluginsconfig_p.h"

#include "plasma/plasma.h"

namespace Plasma
{

Containment::Containment(QObject *parent,
                         const QString &serviceId,
                         uint containmentId)
    : Applet(parent, serviceId, containmentId),
      d(new ContainmentPrivate(this))
{
    // WARNING: do not access config() OR globalConfig() in this method!
    //          that requires a scene, which is not available at this point
    setContainmentType(CustomContainment);
    setHasConfigurationInterface(false);
}

Containment::Containment(QObject *parent, const QVariantList &args)
    : Applet(parent, args),
      d(new ContainmentPrivate(this))
{
    // WARNING: do not access config() OR globalConfig() in this method!
    //          that requires a scene, which is not available at this point
    setHasConfigurationInterface(false);
}

Containment::Containment(const QString &packagePath, uint appletId, const QVariantList &args)
    : Applet(packagePath, appletId, args),
      d(new ContainmentPrivate(this))
{
    // WARNING: do not access config() OR globalConfig() in this method!
    //          that requires a scene, which is not available at this point
    setHasConfigurationInterface(false);
}

Containment::~Containment()
{
    // Applet touches our dptr if we are a containment and is the superclass (think of dtors)
    // so we reset this as we exit the building
    Applet::d->isContainment = false;
    delete d;
}

void Containment::init()
{
    Applet::init();
    if (!isContainment()) {
        return;
    }

    if (d->type == NoContainmentType) {
        setContainmentType(DesktopContainment);
    }

    //connect actions
    ContainmentPrivate::addDefaultActions(d->actions(), this);
    bool unlocked = immutability() == Mutable;

    //fix the text of the actions that need title()
    //btw, do we really want to use title() when it's a desktopcontainment?
    QAction *closeApplet = action("remove");
    if (closeApplet) {
        closeApplet->setText(i18nc("%1 is the name of the applet", "Remove this %1", title()));
    }

    QAction *configAction = action("configure");
    if (configAction) {
        configAction->setText(i18nc("%1 is the name of the applet", "%1 Settings", title()));
    }

    QAction *appletBrowserAction = action("add widgets");
    if (appletBrowserAction) {
        appletBrowserAction->setVisible(unlocked);
        appletBrowserAction->setEnabled(unlocked);
        connect(appletBrowserAction, SIGNAL(triggered()), this, SLOT(triggerShowAddWidgets()));
    }

    QAction *act = action("next applet");
    if (act) {
        connect(act, SIGNAL(triggered()), this, SLOT(focusNextApplet()));
    }

    act = action("previous applet");
    if (act) {
        connect(act, SIGNAL(triggered()), this, SLOT(focusPreviousApplet()));
    }

    if (immutability() != SystemImmutable && corona()) {
        QAction *lockDesktopAction = corona()->action("lock widgets");
        //keep a pointer so nobody notices it moved to corona
        if (lockDesktopAction) {
            d->actions()->addAction("lock widgets", lockDesktopAction);
        }
    }
    if (d->type != PanelContainment && d->type != CustomPanelContainment) {
        if (corona()) {
            //FIXME this is just here because of the darn keyboard shortcut :/
            act = corona()->action("manage activities");
            if (act) {
                d->actions()->addAction("manage activities", act);
            }
            //a stupid hack to make this one's keyboard shortcut work
            act = corona()->action("configure shortcuts");
            if (act) {
                d->actions()->addAction("configure shortcuts", act);
            }
        }
    }
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
    kDebug() << "!!!!!!!!!!!!initConstraints" << group.name() << d->type;
    kDebug() << "    location:" << group.readEntry("location", (int)d->location);
    kDebug() << "    geom:" << group.readEntry("geometry", geometry());
    kDebug() << "    formfactor:" << group.readEntry("formfactor", (int)d->formFactor);
    kDebug() << "    screen:" << group.readEntry("screen", d->screen);
#endif
*/
    if (!isContainment()) {
        Applet::restore(group);
        return;
    }

    setLocation((Plasma::Location)group.readEntry("location", (int)d->location));
    setFormFactor((Plasma::FormFactor)group.readEntry("formfactor", (int)d->formFactor));
    //kDebug() << "setScreen from restore";
    d->setScreen(group.readEntry("screen", d->screen));
    d->activityId = group.readEntry("activityId", QString());

    flushPendingConstraintsEvents();
    restoreContents(group);
    setImmutability((ImmutabilityType)group.readEntry("immutability", (int)Mutable));

    setWallpaper(group.readEntry("wallpaperplugin", ContainmentPrivate::defaultWallpaper));

    KConfigGroup cfg;
    if (containmentType() == PanelContainment || containmentType() == CustomPanelContainment) {
        //don't let global desktop actions conflict with panels
        //this also prevents panels from sharing config with each other
        //but the panels aren't configurable anyways, and I doubt that'll change.
        d->containmentActionsSource = ContainmentActions::Local;
        cfg = KConfigGroup(&group, "ActionPlugins");
    } else {
        const QString source = group.readEntry("ActionPluginsSource", QString());
        if (source == "Global") {
            cfg = KConfigGroup(corona()->config(), "ActionPlugins");
            d->containmentActionsSource = ContainmentActions::Global;
        } else if (source == "Activity") {
            cfg = KConfigGroup(corona()->config(), "Activities");
            cfg = KConfigGroup(&cfg, d->activityId);
            cfg = KConfigGroup(&cfg, "ActionPlugins");
            d->containmentActionsSource = ContainmentActions::Activity;
        } else if (source == "Local") {
            cfg = group;
            d->containmentActionsSource = ContainmentActions::Local;
        } else {
            //default to global
            //but, if there is no global config, try copying it from local.
            cfg = KConfigGroup(corona()->config(), "ActionPlugins");
            if (!cfg.exists()) {
                cfg = KConfigGroup(&group, "ActionPlugins");
            }
            d->containmentActionsSource = ContainmentActions::Global;
            group.writeEntry("ActionPluginsSource", "Global");
        }
    }

    //kDebug() << cfg.keyList();
    if (cfg.exists()) {
        foreach (const QString &key, cfg.keyList()) {
            //kDebug() << "loading" << key;
            addContainmentActions(key, cfg.readEntry(key, QString()));
        }
    } else { //shell defaults
        ContainmentActionsPluginsConfig conf = corona()->containmentActionsDefaults(d->type);
        //steal the data directly, for efficiency
        QHash<QString,QString> defaults = conf.d->plugins;
        for (QHash<QString,QString>::const_iterator it = defaults.constBegin(),
                end = defaults.constEnd(); it != end; ++it) {
            addContainmentActions(it.key(), it.value());
        }
    }

    /*
#ifndef NDEBUG
    kDebug() << "Containment" << id() <<
#endif
                "screen" << screen() <<
                "geometry is" << geometry() <<
                "config entries" << group.entryMap();
    */
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

    if (!isContainment()) {
        return;
    }

    group.writeEntry("screen", d->screen);
    group.writeEntry("formfactor", (int)d->formFactor);
    group.writeEntry("location", (int)d->location);
    group.writeEntry("activityId", d->activityId);

    group.writeEntry("wallpaperplugin", d->wallpaper);

    saveContents(group);
}

void Containment::saveContents(KConfigGroup &group) const
{
    KConfigGroup applets(&group, "Applets");
    foreach (const Applet *applet, d->applets) {
        KConfigGroup appletConfig(&applets, QString::number(applet->id()));
        applet->save(appletConfig);
    }
}

void Containment::restoreContents(KConfigGroup &group)
{
    KConfigGroup applets(&group, "Applets");

    // Sort the applet configs in order of geometry to ensure that applets
    // are added from left to right or top to bottom for a panel containment
    QList<KConfigGroup> appletConfigs;
    foreach (const QString &appletGroup, applets.groupList()) {
        //kDebug() << "reading from applet group" << appletGroup;
        KConfigGroup appletConfig(&applets, appletGroup);
        appletConfigs.append(appletConfig);
    }
    qStableSort(appletConfigs.begin(), appletConfigs.end(), appletConfigLessThan);

    QMutableListIterator<KConfigGroup> it(appletConfigs);
    while (it.hasNext()) {
        KConfigGroup &appletConfig = it.next();
        int appId = appletConfig.name().toUInt();
        QString plugin = appletConfig.readEntry("plugin", QString());

        if (plugin.isEmpty()) {
            continue;
        }

        d->addApplet(plugin, QVariantList(), appId);
    }
}

Plasma::ContainmentType Containment::containmentType() const
{
    return d->type;
}

void Containment::setContainmentType(Plasma::ContainmentType type)
{
    if (d->type == type) {
        return;
    }

    d->type = type;
}

Corona *Containment::corona() const
{
    return qobject_cast<Corona*>(parent());
}

void Containment::setFormFactor(FormFactor formFactor)
{
    if (d->formFactor == formFactor) {
        return;
    }

    //kDebug() << "switching FF to " << formFactor;
    d->formFactor = formFactor;

    updateConstraints(Plasma::FormFactorConstraint);

    KConfigGroup c = config();
    c.writeEntry("formfactor", (int)formFactor);
    emit configNeedsSaving();
}

void Containment::setLocation(Location location)
{
    if (d->location == location) {
        return;
    }

    d->location = location;

    foreach (Applet *applet, d->applets) {
        applet->updateConstraints(Plasma::LocationConstraint);
    }

    updateConstraints(Plasma::LocationConstraint);

    KConfigGroup c = config();
    c.writeEntry("location", (int)location);
    emit configNeedsSaving();
}

Applet *Containment::addApplet(const QString &name, const QVariantList &args)
{
    return d->addApplet(name, args);
}

void Containment::addApplet(Applet *applet)
{
    if (!isContainment() || immutability() != Mutable) {
        return;
    }

    if (!applet) {
#ifndef NDEBUG
        kDebug() << "adding null applet!?!";
#endif
        return;
    }

    if (d->applets.contains(applet)) {
#ifndef NDEBUG
        kDebug() << "already have this applet!";
#endif
    }

    Containment *currentContainment = applet->containment();

    if (currentContainment && currentContainment != this) {
        emit currentContainment->appletRemoved(applet);

        disconnect(applet, 0, currentContainment, 0);
        KConfigGroup oldConfig = applet->config();
        currentContainment->d->applets.removeAll(applet);
        applet->setParent(this);

        // now move the old config to the new location
        //FIXME: this doesn't seem to get the actual main config group containing plugin=, etc
        KConfigGroup c = config().group("Applets").group(QString::number(applet->id()));
        oldConfig.reparent(&c);
        applet->d->resetConfigurationObject();

        disconnect(applet, SIGNAL(activate()), currentContainment, SIGNAL(activate()));
    } else {
        applet->setParent(this);
    }

    d->applets << applet;

    connect(applet, SIGNAL(configNeedsSaving()), this, SIGNAL(configNeedsSaving()));
    connect(applet, SIGNAL(releaseVisualFocus()), this, SIGNAL(releaseVisualFocus()));
    connect(applet, SIGNAL(appletDeleted(Plasma::Applet*)), this, SLOT(appletDeleted(Plasma::Applet*)));
    connect(applet, SIGNAL(statusChanged(Plasma::ItemStatus)), this, SLOT(checkStatus(Plasma::ItemStatus)));
    connect(applet, SIGNAL(activate()), this, SIGNAL(activate()));

    if (!currentContainment) {
        const bool isNew = applet->d->mainConfigGroup()->entryMap().isEmpty();

        if (!isNew) {
            applet->restore(*applet->d->mainConfigGroup());
        }

        applet->init();

        if (isNew) {
            applet->save(*applet->d->mainConfigGroup());
            emit configNeedsSaving();
        }
        //FIXME: an on-appear animation would be nice to have again
    }

    applet->updateConstraints(Plasma::AllConstraints);
    applet->flushPendingConstraintsEvents();

    emit appletAdded(applet);

    if (!currentContainment) {
        applet->updateConstraints(Plasma::StartupCompletedConstraint);
        applet->flushPendingConstraintsEvents();
    }

    applet->d->scheduleModificationNotification();
}

QList<Applet *> Containment::applets() const
{
    return d->applets;
}

void Containment::setScreen(int newScreen)
{
    d->setScreen(newScreen);
}

int Containment::screen() const
{
    return d->screen;
}

void Containment::enableAction(const QString &name, bool enable)
{
    QAction *action = this->action(name);
    if (action) {
        action->setEnabled(enable);
        action->setVisible(enable);
    }
}

void Containment::addAssociatedWidget(QWidget *widget)
{
    //TODO: move this whole method in the c++ part of the QML implementation
    Applet::addAssociatedWidget(widget);

    /*if (d->focusedApplet) {
        d->focusedApplet->addAssociatedWidget(widget);
    }*/

    foreach (const Applet *applet, d->applets) {
        if (applet->d->activationAction) {
            widget->addAction(applet->d->activationAction);
        }
    }
}

void Containment::removeAssociatedWidget(QWidget *widget)
{
    //TODO: move this whole method in the c++ part of the QML implementation
    Applet::removeAssociatedWidget(widget);
    /*if (d->focusedApplet) {
        d->focusedApplet->removeAssociatedWidget(widget);
    }*/

    foreach (const Applet *applet, d->applets) {
        if (applet->d->activationAction) {
            widget->removeAction(applet->d->activationAction);
        }
    }
}

void Containment::setDrawWallpaper(bool drawWallpaper)
{
    d->drawWallpaper = drawWallpaper;
}

bool Containment::drawWallpaper()
{
    return d->drawWallpaper;
}

void Containment::setWallpaper(const QString &pluginName)
{
    if (pluginName != d->wallpaper) {
        d->wallpaper = pluginName;

        KConfigGroup cfg = config();
        cfg.writeEntry("wallpaperplugin", d->wallpaper);
        emit configNeedsSaving();
        emit wallpaperChanged();
    }
}

QString Containment::wallpaper() const
{
    return d->wallpaper;
}

void Containment::addContainmentActions(const QString &trigger, const QString &pluginName)
{
    KConfigGroup cfg = d->containmentActionsConfig();
    ContainmentActions *plugin = 0;

    if (containmentActions().contains(trigger)) {
        plugin = containmentActions().value(trigger);
        if (plugin->pluginName() != pluginName) {
            containmentActions().remove(trigger);
            delete plugin;
            plugin = 0;
        }
    }

    if (pluginName.isEmpty()) {
        cfg.deleteEntry(trigger);
    } else if (plugin) {
        // it already existed, reset the containment so it wil reload config on next show
        plugin->setContainment(0);
    } else {
        switch (d->containmentActionsSource) {
        case ContainmentActions::Activity:
            //FIXME
        case ContainmentActions::Local:
            plugin = PluginLoader::self()->loadContainmentActions(this, pluginName);
            break;
        default:
            plugin = PluginLoader::self()->loadContainmentActions(0, pluginName);
        }

        if (plugin) {
            cfg.writeEntry(trigger, pluginName);
            plugin->setSource(d->containmentActionsSource);
            containmentActions().insert(trigger, plugin);
        } else {
            //bad plugin... gets removed. is this a feature or a bug?
            cfg.deleteEntry(trigger);
        }
    }

    emit configNeedsSaving();
}

QHash<QString, ContainmentActions*> &Containment::containmentActions()
{
    switch (d->containmentActionsSource) {
        case ContainmentActions::Activity:
            //FIXME
        case ContainmentActions::Local:
            return d->localActionPlugins;
        default:
            return d->globalActionPlugins;
    }
}

void Containment::setActivity(const QString &activityId)
{
    if (activityId.isEmpty() || d->activityId == activityId) {
        return;
    }

    d->activityId = activityId;
    KConfigGroup c = config();
    c.writeEntry("activityId", activityId);

    emit configNeedsSaving();
    emit activityChanged(activityId);
}

QString Containment::activity() const
{
    return d->activityId;
}

void Containment::showConfigurationInterface()
{
    Applet::showConfigurationInterface();
}

} // Plasma namespace



#include "moc_containment.cpp"
