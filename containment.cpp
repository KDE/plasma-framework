/*
 *   Copyright 2007 by Aaron Seigo <aseigo@kde.org>
 *   Copyright 2008 by Ménard Alexis <darktears31@gmail.com>
 *   Copyright 2009 Chani Armitage <chani@kde.org>
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
#include <QGraphicsSceneContextMenuEvent>
#include <QGraphicsView>
#include <QMimeData>
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QGraphicsLayout>
#include <QGraphicsLinearLayout>

#include <kaction.h>
#include <kauthorized.h>
#include <kicon.h>
#include <kmenu.h>
#include <kmessagebox.h>
#include <kmimetype.h>
#include <kservicetypetrader.h>
#include <kstandarddirs.h>
#include <ktemporaryfile.h>
#include <kwindowsystem.h>

#ifndef PLASMA_NO_KIO
#include "kio/jobclasses.h" // for KIO::JobFlags
#include "kio/job.h"
#include "kio/scheduler.h"
#endif

#include "abstracttoolbox.h"
#include "animator.h"
#include "context.h"
#include "containmentactions.h"
#include "containmentactionspluginsconfig.h"
#include "corona.h"
#include "extender.h"
#include "extenderitem.h"
#include "svg.h"
#include "wallpaper.h"

#include "remote/accessappletjob.h"
#include "remote/accessmanager.h"

#include "private/applet_p.h"
#include "private/containmentactionspluginsconfig_p.h"
#include "private/extenderitemmimedata_p.h"
#include "private/extenderapplet_p.h"
#include "private/wallpaper_p.h"

#include "plasma/plasma.h"
#include "animations/animation.h"

namespace Plasma
{

bool ContainmentPrivate::s_positioningPanels = false;
QHash<QString, ContainmentActions*> ContainmentPrivate::globalActionPlugins;
static const char defaultWallpaper[] = "image";
static const char defaultWallpaperMode[] = "SingleImage";

Containment::StyleOption::StyleOption()
    : QStyleOptionGraphicsItem(),
      view(0)
{
    version = Version;
    type = Type;
}

Containment::StyleOption::StyleOption(const Containment::StyleOption & other)
    : QStyleOptionGraphicsItem(other),
      view(other.view)
{
    version = Version;
    type = Type;
}

Containment::StyleOption::StyleOption(const QStyleOptionGraphicsItem &other)
    : QStyleOptionGraphicsItem(other),
      view(0)
{
    version = Version;
    type = Type;
}

Containment::Containment(QGraphicsItem *parent,
                         const QString &serviceId,
                         uint containmentId)
    : Applet(parent, serviceId, containmentId),
      d(new ContainmentPrivate(this))
{
    // WARNING: do not access config() OR globalConfig() in this method!
    //          that requires a scene, which is not available at this point
    setPos(0, 0);
    setBackgroundHints(NoBackground);
    setContainmentType(CustomContainment);
    setHasConfigurationInterface(false);
}

Containment::Containment(QObject *parent, const QVariantList &args)
    : Applet(parent, args),
      d(new ContainmentPrivate(this))
{
    // WARNING: do not access config() OR globalConfig() in this method!
    //          that requires a scene, which is not available at this point
    setPos(0, 0);
    setBackgroundHints(NoBackground);
    setHasConfigurationInterface(false);
}

Containment::Containment(const QString &packagePath, uint appletId, const QVariantList &args)
    : Plasma::Applet(packagePath, appletId, args),
      d(new ContainmentPrivate(this))
{
    // WARNING: do not access config() OR globalConfig() in this method!
    //          that requires a scene, which is not available at this point
    setPos(0, 0);
    setBackgroundHints(NoBackground);
    setHasConfigurationInterface(false);
}

Containment::~Containment()
{
    delete d;
    // Applet touches our dptr if we are a containment and is the superclass (think of dtors)
    // so we reset this as we exit the building
    Applet::d->isContainment = false;
}

void Containment::init()
{
    Applet::init();
    if (!isContainment()) {
        return;
    }

    setCacheMode(NoCache);
    setFlag(QGraphicsItem::ItemIsMovable, false);
    setFlag(QGraphicsItem::ItemClipsChildrenToShape, true);
    setAcceptDrops(true);
    setAcceptsHoverEvents(true);

    if (d->type == NoContainmentType) {
        setContainmentType(DesktopContainment);
    }

    //connect actions
    ContainmentPrivate::addDefaultActions(d->actions(), this);
    bool unlocked = immutability() == Mutable;

    //fix the text of the actions that need name()
    //btw, do we really want to use name() when it's a desktopcontainment?
    QAction *closeApplet = action("remove");
    if (closeApplet) {
        closeApplet->setText(i18nc("%1 is the name of the applet", "Remove this %1", name()));
    }

    QAction *configAction = action("configure");
    if (configAction) {
        configAction->setText(i18nc("%1 is the name of the applet", "%1 Settings", name()));
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

        if (d->type == DesktopContainment) {
            addToolBoxAction(action("add widgets"));

            //TODO: do we need some way to allow this be overridden?
            //      it's always available because shells rely on this
            //      to offer their own custom configuration as well
            QAction *configureContainment = action("configure");
            if (configureContainment) {
                addToolBoxAction(configureContainment);
            }
        }
    }
}

void ContainmentPrivate::addDefaultActions(KActionCollection *actions, Containment *c)
{
    actions->setConfigGroup("Shortcuts-Containment");

    //adjust applet actions
    KAction *appAction = qobject_cast<KAction*>(actions->action("remove"));
    appAction->setShortcut(KShortcut("alt+d, alt+r"));
    if (c && c->d->isPanelContainment()) {
        appAction->setText(i18n("Remove this Panel"));
    } else {
        appAction->setText(i18n("Remove this Activity"));
    }

    appAction = qobject_cast<KAction*>(actions->action("configure"));
    if (appAction) {
        appAction->setShortcut(KShortcut("alt+d, alt+s"));
        appAction->setText(i18n("Activity Settings"));
    }

    //add our own actions
    KAction *appletBrowserAction = actions->addAction("add widgets");
    appletBrowserAction->setAutoRepeat(false);
    appletBrowserAction->setText(i18n("Add Widgets..."));
    appletBrowserAction->setIcon(KIcon("list-add"));
    appletBrowserAction->setShortcut(KShortcut("alt+d, a"));
    appletBrowserAction->setData(AbstractToolBox::AddTool);

    KAction *action = actions->addAction("next applet");
    action->setText(i18n("Next Widget"));
    //no icon
    action->setShortcut(KShortcut("alt+d, n"));
    action->setData(AbstractToolBox::ControlTool);

    action = actions->addAction("previous applet");
    action->setText(i18n("Previous Widget"));
    //no icon
    action->setShortcut(KShortcut("alt+d, p"));
    action->setData(AbstractToolBox::ControlTool);
}

// helper function for sorting the list of applets
bool appletConfigLessThan(const KConfigGroup &c1, const KConfigGroup &c2)
{
    QPointF p1 = c1.readEntry("geometry", QRectF()).topLeft();
    QPointF p2 = c2.readEntry("geometry", QRectF()).topLeft();

    if (!qFuzzyCompare(p1.x(), p2.x())) {
        if (QApplication::layoutDirection() == Qt::RightToLeft) {
            return p1.x() > p2.x();
        }

        return p1.x() < p2.x();
    }

    return qFuzzyCompare(p1.y(), p2.y()) || p1.y() < p2.y();
}

void Containment::restore(KConfigGroup &group)
{
    /*kDebug() << "!!!!!!!!!!!!initConstraints" << group.name() << d->type;
    kDebug() << "    location:" << group.readEntry("location", (int)d->location);
    kDebug() << "    geom:" << group.readEntry("geometry", geometry());
    kDebug() << "    formfactor:" << group.readEntry("formfactor", (int)d->formFactor);
    kDebug() << "    screen:" << group.readEntry("screen", d->screen);*/
    if (!isContainment()) {
        Applet::restore(group);
        return;
    }

    QRectF geo = group.readEntry("geometry", geometry());
    //override max/min
    //this ensures panels are set to their saved size even when they have max & min set to prevent
    //resizing
    if (geo.size() != geo.size().boundedTo(maximumSize())) {
        setMaximumSize(maximumSize().expandedTo(geo.size()));
    }

    if (geo.size() != geo.size().expandedTo(minimumSize())) {
        setMinimumSize(minimumSize().boundedTo(geo.size()));
    }


    resize(geo.size());
    //are we an offscreen containment?
    if (containmentType() != PanelContainment && containmentType() != CustomPanelContainment && geo.right() < 0) {
        corona()->addOffscreenWidget(this);
    }

    setLocation((Plasma::Location)group.readEntry("location", (int)d->location));
    setFormFactor((Plasma::FormFactor)group.readEntry("formfactor", (int)d->formFactor));
    //kDebug() << "setScreen from restore";
    d->lastScreen = group.readEntry("lastScreen", d->lastScreen);
    d->lastDesktop = group.readEntry("lastDesktop", d->lastDesktop);
    d->setScreen(group.readEntry("screen", d->screen), group.readEntry("desktop", d->desktop), false);
    QString activityId = group.readEntry("activityId", QString());
    if (!activityId.isEmpty()) {
        d->context()->setCurrentActivityId(activityId);
    }
    setActivity(group.readEntry("activity", QString()));

    flushPendingConstraintsEvents();
    restoreContents(group);
    setImmutability((ImmutabilityType)group.readEntry("immutability", (int)Mutable));

    setWallpaper(group.readEntry("wallpaperplugin", defaultWallpaper),
                 group.readEntry("wallpaperpluginmode", defaultWallpaperMode));

    QMetaObject::invokeMethod(d->toolBox.data(), "restore", Q_ARG(KConfigGroup, group));

    KConfigGroup cfg;
    if (containmentType() == PanelContainment || containmentType() == CustomPanelContainment) {
        //don't let global desktop actions conflict with panels
        //this also prevents panels from sharing config with each other
        //but the panels aren't configurable anyways, and I doubt that'll change.
        d->containmentActionsSource = ContainmentPrivate::Local;
        cfg = KConfigGroup(&group, "ActionPlugins");
    } else {
        QString source = group.readEntry("ActionPluginsSource", QString());
        if (source == "Global") {
            cfg = KConfigGroup(corona()->config(), "ActionPlugins");
            d->containmentActionsSource = ContainmentPrivate::Global;
        } else if (source == "Activity") {
            cfg = KConfigGroup(corona()->config(), "Activities");
            cfg = KConfigGroup(&cfg, activityId);
            cfg = KConfigGroup(&cfg, "ActionPlugins");
            d->containmentActionsSource = ContainmentPrivate::Activity;
        } else if (source == "Local") {
            cfg = group;
            d->containmentActionsSource = ContainmentPrivate::Local;
        } else {
            //default to global
            //but, if there is no global config, try copying it from local.
            cfg = KConfigGroup(corona()->config(), "ActionPlugins");
            if (!cfg.exists()) {
                cfg = KConfigGroup(&group, "ActionPlugins");
            }
            d->containmentActionsSource = ContainmentPrivate::Global;
            group.writeEntry("ActionPluginsSource", "Global");
        }
    }
    //kDebug() << cfg.keyList();
    if (cfg.exists()) {
        foreach (const QString &key, cfg.keyList()) {
            //kDebug() << "loading" << key;
            setContainmentActions(key, cfg.readEntry(key, QString()));
        }
    } else { //shell defaults
        ContainmentActionsPluginsConfig conf = corona()->containmentActionsDefaults(d->type);
        //steal the data directly, for efficiency
        QHash<QString,QString> defaults = conf.d->plugins;
        for (QHash<QString,QString>::const_iterator it = defaults.constBegin(),
                end = defaults.constEnd(); it != end; ++it) {
            setContainmentActions(it.key(), it.value());
        }
    }

    /*
    kDebug() << "Containment" << id() <<
                "screen" << screen() <<
                "geometry is" << geometry() <<
                "wallpaper" << ((d->wallpaper) ? d->wallpaper->pluginName() : QString()) <<
                "wallpaper mode" << wallpaperMode() <<
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
    group.writeEntry("lastScreen", d->lastScreen);
    group.writeEntry("desktop", d->desktop);
    group.writeEntry("lastDesktop", d->lastDesktop);
    group.writeEntry("formfactor", (int)d->formFactor);
    group.writeEntry("location", (int)d->location);
    group.writeEntry("activity", d->context()->currentActivity());
    group.writeEntry("activityId", d->context()->currentActivityId());


    QMetaObject::invokeMethod(d->toolBox.data(), "save", Q_ARG(KConfigGroup, group));


    if (d->wallpaper) {
        group.writeEntry("wallpaperplugin", d->wallpaper->pluginName());
        group.writeEntry("wallpaperpluginmode", d->wallpaper->renderingMode().name());

        if (d->wallpaper->isInitialized()) {
            KConfigGroup wallpaperConfig(&group, "Wallpaper");
            wallpaperConfig = KConfigGroup(&wallpaperConfig, d->wallpaper->pluginName());
            d->wallpaper->save(wallpaperConfig);
        }
    }

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

void ContainmentPrivate::initApplets()
{
    foreach (Applet *applet, applets) {
        applet->restore(*applet->d->mainConfigGroup());
        applet->init();
        kDebug() << "!!{} STARTUP TIME" << QTime().msecsTo(QTime::currentTime()) << "Applet" << applet->name();
    }

    q->flushPendingConstraintsEvents();

    foreach (Applet *applet, applets) {
        applet->flushPendingConstraintsEvents();
    }

    kDebug() << "!!{} STARTUP TIME" << QTime().msecsTo(QTime::currentTime()) << "Containment's applets initialized" << q->name();
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

        d->addApplet(plugin, QVariantList(), appletConfig.readEntry("geometry", QRectF()), appId, true);
    }
}

Containment::Type Containment::containmentType() const
{
    return d->type;
}

void Containment::setContainmentType(Containment::Type type)
{
    if (d->type == type) {
        return;
    }

    delete d->toolBox.data();
    d->type = type;
    d->checkContainmentFurniture();
}

void ContainmentPrivate::checkContainmentFurniture()
{
    if (q->isContainment() &&
        (type == Containment::DesktopContainment || type == Containment::PanelContainment)) {
        createToolBox();
    }
}

Corona *Containment::corona() const
{
    return qobject_cast<Corona*>(scene());
}

void Containment::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    event->ignore();
    if (d->wallpaper && d->wallpaper->isInitialized()) {
        QGraphicsItem *item = scene()->itemAt(event->scenePos());
        if (item == this) {
            d->wallpaper->mouseMoveEvent(event);
        }
    }

    if (!event->isAccepted()) {
        event->accept();
        Applet::mouseMoveEvent(event);
    }
}

void Containment::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    //close a toolbox if exists, to emulate qmenu behavior
    if (d->toolBox) {
        d->toolBox.data()->setShowing(false);
    }
    event->ignore();
    if (d->appletAt(event->scenePos())) {
        return; //no unexpected click-throughs
    }

    if (d->wallpaper && d->wallpaper->isInitialized() && !event->isAccepted()) {
        d->wallpaper->mousePressEvent(event);
    }

    if (event->isAccepted()) {
        setFocus(Qt::MouseFocusReason);
    } else if (event->button() == Qt::RightButton && event->modifiers() == Qt::NoModifier) {
        // we'll catch this in the context menu even
        Applet::mousePressEvent(event);
    } else {
        QString trigger = ContainmentActions::eventToString(event);
        if (d->prepareContainmentActions(trigger, event->screenPos())) {
            d->actionPlugins()->value(trigger)->contextEvent(event);
        }

        if (!event->isAccepted()) {
            Applet::mousePressEvent(event);
        }
    }
}

void Containment::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    event->ignore();

    if (d->appletAt(event->scenePos())) {
        return; //no unexpected click-throughs
    }

    QString trigger = ContainmentActions::eventToString(event);

    if (d->wallpaper && d->wallpaper->isInitialized()) {
        d->wallpaper->mouseReleaseEvent(event);
    }

    if (!event->isAccepted() && isContainment()) {
        if (d->prepareContainmentActions(trigger, event->screenPos())) {
            d->actionPlugins()->value(trigger)->contextEvent(event);
        }

        event->accept();
        Applet::mouseReleaseEvent(event);
    }
}

void Containment::showDropZone(const QPoint pos)
{
    Q_UNUSED(pos)
    //Base implementation does nothing, don't put code here
}

void Containment::showContextMenu(const QPointF &containmentPos, const QPoint &screenPos)
{
    //kDebug() << containmentPos << screenPos;
    QGraphicsSceneContextMenuEvent gvevent;
    gvevent.setScreenPos(screenPos);
    gvevent.setScenePos(mapToScene(containmentPos));
    gvevent.setPos(containmentPos);
    gvevent.setReason(QGraphicsSceneContextMenuEvent::Mouse);
    gvevent.setWidget(view());
    contextMenuEvent(&gvevent);
}

void Containment::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    if (!isContainment() || !KAuthorized::authorizeKAction("plasma/containment_context_menu")) {
        Applet::contextMenuEvent(event);
        return;
    }

    KMenu desktopMenu;
    Applet *applet = d->appletAt(event->scenePos());
    //kDebug() << "context menu event " << (QObject*)applet;

    if (applet) {
        d->addAppletActions(desktopMenu, applet, event);
    } else {
        d->addContainmentActions(desktopMenu, event);
    }

    //kDebug() << "executing at" << screenPos;
    QMenu *menu = &desktopMenu;
    //kDebug() << "showing menu, actions" << desktopMenu.actions().size() << desktopMenu.actions().first()->menu();
    if (desktopMenu.actions().size() == 1 && desktopMenu.actions().first()->menu()) {
        // we have a menu with a single top level menu; just show that top level menu instad.
        menu = desktopMenu.actions().first()->menu();
    }

    if (!menu->isEmpty()) {
        QPoint pos = event->screenPos();
        if (applet && d->isPanelContainment()) {
            menu->adjustSize();
            pos = applet->popupPosition(menu->size());
            if (event->reason() == QGraphicsSceneContextMenuEvent::Mouse) {
                // if the menu pops up way away from the mouse press, then move it
                // to the mouse press
                if (d->formFactor == Vertical) {
                    if (pos.y() + menu->height() < event->screenPos().y()) {
                        pos.setY(event->screenPos().y());
                    }
                } else if (d->formFactor == Horizontal) {
                    if (pos.x() + menu->width() < event->screenPos().x()) {
                        pos.setX(event->screenPos().x());
                    }
                }
            }
        }

        menu->exec(pos);
        event->accept();
    } else {
        Applet::contextMenuEvent(event);
    }
}

void ContainmentPrivate::addContainmentActions(KMenu &desktopMenu, QEvent *event)
{
    if (static_cast<Corona*>(q->scene())->immutability() != Mutable &&
        !KAuthorized::authorizeKAction("plasma/containment_actions")) {
        //kDebug() << "immutability";
        return;
    }

    const QString trigger = ContainmentActions::eventToString(event);
    prepareContainmentActions(trigger, QPoint(), &desktopMenu);
}

void ContainmentPrivate::addAppletActions(KMenu &desktopMenu, Applet *applet, QEvent *event)
{
    foreach (QAction *action, applet->contextualActions()) {
        if (action) {
            desktopMenu.addAction(action);
        }
    }

    if (!applet->d->failed) {
        QAction *configureApplet = applet->d->actions->action("configure");
        if (configureApplet && configureApplet->isEnabled()) {
            desktopMenu.addAction(configureApplet);
        }

        QAction *runAssociatedApplication = applet->d->actions->action("run associated application");
        if (runAssociatedApplication && runAssociatedApplication->isEnabled()) {
            desktopMenu.addAction(runAssociatedApplication);
        }
    }

    KMenu *containmentMenu = new KMenu(i18nc("%1 is the name of the containment", "%1 Options", q->name()), &desktopMenu);
    addContainmentActions(*containmentMenu, event);
    if (!containmentMenu->isEmpty()) {
        int enabled = 0;
        //count number of real actions
        QListIterator<QAction *> actionsIt(containmentMenu->actions());
        while (enabled < 3 && actionsIt.hasNext()) {
            QAction *action = actionsIt.next();
            if (action->isVisible() && !action->isSeparator()) {
                ++enabled;
            }
        }

        if (enabled) {
            //if there is only one, don't create a submenu
            if (enabled < 2) {
                foreach (QAction *action, containmentMenu->actions()) {
                    if (action->isVisible() && !action->isSeparator()) {
                        desktopMenu.addAction(action);
                    }
                }
            } else {
                desktopMenu.addMenu(containmentMenu);
            }
        }
    }

    if (q->immutability() == Mutable) {
        QAction *closeApplet = applet->d->actions->action("remove");
        //kDebug() << "checking for removal" << closeApplet;
        if (closeApplet) {
            if (!desktopMenu.isEmpty()) {
                desktopMenu.addSeparator();
            }

            //kDebug() << "adding close action" << closeApplet->isEnabled() << closeApplet->isVisible();
            desktopMenu.addAction(closeApplet);
        }
    }
}

Applet* ContainmentPrivate::appletAt(const QPointF &point)
{
    Applet *applet = 0;

    QGraphicsItem *item = q->scene()->itemAt(point);
    if (item == q) {
        item = 0;
    }

    while (item) {
        if (item->isWidget()) {
            applet = qobject_cast<Applet*>(static_cast<QGraphicsWidget*>(item));
            if (applet) {
                if (applet->isContainment()) {
                    applet = 0;
                }
                break;
            }
        }
        AppletHandle *handle = dynamic_cast<AppletHandle*>(item);
        if (handle) {
            //pretend it was on the applet
            applet = handle->applet();
            break;
        }
        item = item->parentItem();
    }
    return applet;
}

void Containment::setFormFactor(FormFactor formFactor)
{
    if (d->formFactor == formFactor) {
        return;
    }

    //kDebug() << "switching FF to " << formFactor;
    d->formFactor = formFactor;

    if (isContainment() &&
        (d->type == PanelContainment || d->type == CustomPanelContainment)) {
        // we are a panel and we have chaged our orientation
        d->positionPanel(true);
    }

    QMetaObject::invokeMethod(d->toolBox.data(), "reposition");

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

    bool emitGeomChange = false;

    if ((location == TopEdge || location == BottomEdge) &&
        (d->location == TopEdge || d->location == BottomEdge)) {
        emitGeomChange = true;
    }

    if ((location == RightEdge || location == LeftEdge) &&
        (d->location == RightEdge || d->location == LeftEdge)) {
        emitGeomChange = true;
    }

    d->location = location;

    foreach (Applet *applet, d->applets) {
        applet->updateConstraints(Plasma::LocationConstraint);
    }

    if (emitGeomChange) {
        // our geometry on the scene will not actually change,
        // but for the purposes of views it has
        emit geometryChanged();
    }

    updateConstraints(Plasma::LocationConstraint);

    KConfigGroup c = config();
    c.writeEntry("location", (int)location);
    emit configNeedsSaving();
}

void Containment::addSiblingContainment()
{
    emit addSiblingContainment(this);
}

void Containment::clearApplets()
{
    foreach (Applet *applet, d->applets) {
        applet->d->cleanUpAndDelete();
    }

    d->applets.clear();
}

Applet *Containment::addApplet(const QString &name, const QVariantList &args,
                               const QRectF &appletGeometry)
{
    return d->addApplet(name, args, appletGeometry);
}

void Containment::addApplet(Applet *applet, const QPointF &pos, bool delayInit)
{
    if (!isContainment() || (!delayInit && immutability() != Mutable)) {
        return;
    }

    if (!applet) {
        kDebug() << "adding null applet!?!";
        return;
    }

    if (d->applets.contains(applet)) {
        kDebug() << "already have this applet!";
    }

    Containment *currentContainment = applet->containment();

    if (d->type == PanelContainment) {
        //panels don't want backgrounds, which is important when setting geometry
        setBackgroundHints(NoBackground);
    }

    if (currentContainment && currentContainment != this) {
        emit currentContainment->appletRemoved(applet);
        if (currentContainment->d->focusedApplet == applet) {
            currentContainment->d->focusedApplet = 0;
        }

        disconnect(applet, 0, currentContainment, 0);
        KConfigGroup oldConfig = applet->config();
        currentContainment->d->applets.removeAll(applet);
        applet->setParentItem(this);
        applet->setParent(this);

        // now move the old config to the new location
        //FIXME: this doesn't seem to get the actual main config group containing plugin=, etc
        KConfigGroup c = config().group("Applets").group(QString::number(applet->id()));
        oldConfig.reparent(&c);
        applet->d->resetConfigurationObject();

        disconnect(applet, SIGNAL(activate()), currentContainment, SIGNAL(activate()));
    } else {
        applet->setParentItem(this);
        applet->setParent(this);
    }

    d->applets << applet;

    connect(applet, SIGNAL(configNeedsSaving()), this, SIGNAL(configNeedsSaving()));
    connect(applet, SIGNAL(releaseVisualFocus()), this, SIGNAL(releaseVisualFocus()));
    connect(applet, SIGNAL(appletDestroyed(Plasma::Applet*)), this, SLOT(appletDestroyed(Plasma::Applet*)));
    connect(applet, SIGNAL(newStatus(Plasma::ItemStatus)), this, SLOT(checkStatus(Plasma::ItemStatus)));
    connect(applet, SIGNAL(activate()), this, SIGNAL(activate()));

    if (pos != QPointF(-1, -1)) {
        applet->setPos(pos);
    }

    if (!delayInit && !currentContainment) {
        applet->restore(*applet->d->mainConfigGroup());
        applet->init();
        Plasma::Animation *anim = Plasma::Animator::create(Plasma::Animator::AppearAnimation);
        if (anim) {
            connect(anim, SIGNAL(finished()), this, SLOT(appletAppearAnimationComplete()));
            anim->setTargetWidget(applet);
            //FIXME: small hack until we have proper js anim support; allows 'zoom' to work in the
            //'right' direction for appearance
            anim->setDirection(QAbstractAnimation::Backward);
            anim->start(QAbstractAnimation::DeleteWhenStopped);
        } else {
            d->appletAppeared(applet);
        }
    }

    applet->setFlag(QGraphicsItem::ItemIsMovable, true);
    applet->updateConstraints(Plasma::AllConstraints);
    if (!delayInit) {
        applet->flushPendingConstraintsEvents();
    }
    emit appletAdded(applet, pos);

    if (!currentContainment) {
        applet->updateConstraints(Plasma::StartupCompletedConstraint);
        if (!delayInit) {
            applet->flushPendingConstraintsEvents();
        }
    }

    if (!delayInit) {
        applet->d->scheduleModificationNotification();
    }
}

Applet::List Containment::applets() const
{
    return d->applets;
}

void Containment::setScreen(int newScreen, int newDesktop)
{
    d->setScreen(newScreen, newDesktop);
}

void ContainmentPrivate::setScreen(int newScreen, int newDesktop, bool preventInvalidDesktops)
{
    // What we want to do in here is:
    //   * claim the screen as our own
    //   * signal whatever may be watching this containment about the switch
    //   * if we are a full screen containment, then:
    //      * resize to match the screen if we're that kind of containment
    //      * kick other full-screen containments off this screen
    //          * if we had a screen, then give our screen to the containment
    //            we kick out
    //
    // a screen of -1 means no associated screen.
    Corona *corona = q->corona();
    Q_ASSERT(corona);

    //if it's an offscreen widget, don't allow to claim a screen, after all it's *off*screen
    if (corona->offscreenWidgets().contains(q)) {
        return;
    }

    int numScreens = corona->numScreens();
    if (newScreen < -1) {
        newScreen = -1;
    }

    // -1 == All desktops
    if (newDesktop < -1 || (preventInvalidDesktops && newDesktop > KWindowSystem::numberOfDesktops() - 1)) {
        newDesktop = -1;
    }

    //kDebug() << activity() << "setting screen to " << newScreen << newDesktop << "and type is" << type;

    Containment *swapScreensWith(0);
    const bool isDesktopContainment = type == Containment::DesktopContainment ||
                                      type == Containment::CustomContainment;
    if (isDesktopContainment) {
        // we want to listen to changes in work area if our screen changes
        if (toolBox) {
            if (screen < 0 && newScreen > -1) {
                QObject::connect(KWindowSystem::self(), SIGNAL(workAreaChanged()), toolBox.data(), SLOT(reposition()), Qt::UniqueConnection);
            } else if (newScreen < 0) {
                QObject::disconnect(KWindowSystem::self(), SIGNAL(workAreaChanged()), toolBox.data(), SLOT(reposition()));
            }
        }

        if (newScreen > -1) {
            // sanity check to make sure someone else doesn't have this screen already!
            Containment *currently = corona->containmentForScreen(newScreen, newDesktop);
            if (currently && currently != q) {
                kDebug() << "currently is on screen" << currently->screen()
                         << "desktop" << currently->desktop()
                         << "and is" << currently->activity()
                         << (QObject*)currently << "i'm" << (QObject*)q;
                currently->setScreen(-1, currently->desktop());
                swapScreensWith = currently;
            }
        }
    }

    if (newScreen < numScreens && newScreen > -1 && isDesktopContainment) {
        q->resize(corona->screenGeometry(newScreen).size());
    }

    int oldDesktop = desktop;
    desktop = newDesktop;

    int oldScreen = screen;
    screen = newScreen;

    q->updateConstraints(Plasma::ScreenConstraint);

    if (oldScreen != newScreen || oldDesktop != newDesktop) {
        /*
        kDebug() << "going to signal change for" << q
                 << ", old screen & desktop:" << oldScreen << oldDesktop
                 << ", new:" << screen << desktop;
                 */
        KConfigGroup c = q->config();
        c.writeEntry("screen", screen);
        c.writeEntry("desktop", desktop);
        if (newScreen != -1) {
            lastScreen = newScreen;
            lastDesktop = newDesktop;
            c.writeEntry("lastScreen", lastScreen);
            c.writeEntry("lastDesktop", lastDesktop);
        }
        emit q->configNeedsSaving();
        emit q->screenChanged(oldScreen, newScreen, q);
    }

    if (swapScreensWith) {
        //kDebug() << "setScreen due to swap, part 2";
        swapScreensWith->setScreen(oldScreen, oldDesktop);
    }

    checkRemoveAction();

    if (newScreen >= 0) {
        emit q->activate();
    }
}

int Containment::screen() const
{
    return d->screen;
}

int Containment::lastScreen() const
{
    return d->lastScreen;
}

int Containment::desktop() const
{
    return d->desktop;
}

int Containment::lastDesktop() const
{
    return d->lastDesktop;
}

KPluginInfo::List Containment::listContainments(const QString &category,
                                                const QString &parentApp)
{
    return listContainmentsOfType(QString(), category, parentApp);
}


KPluginInfo::List Containment::listContainmentsOfType(const QString &type,
                                                      const QString &category,
                                                      const QString &parentApp)
{
    QString constraint;

    if (parentApp.isEmpty()) {
        constraint.append("(not exist [X-KDE-ParentApp] or [X-KDE-ParentApp] == '')");
    } else {
        constraint.append("[X-KDE-ParentApp] == '").append(parentApp).append("'");
    }

    if (!type.isEmpty()) {
        if (!constraint.isEmpty()) {
            constraint.append(" and ");
        }

        constraint.append("'").append(type).append("' ~in [X-Plasma-ContainmentCategories]");
    }

    if (!category.isEmpty()) {
        if (!constraint.isEmpty()) {
            constraint.append(" and ");
        }

        constraint.append("[X-KDE-PluginInfo-Category] == '").append(category).append("'");
        if (category == "Miscellaneous") {
            constraint.append(" or (not exist [X-KDE-PluginInfo-Category] or [X-KDE-PluginInfo-Category] == '')");
        }
    }

    KService::List offers = KServiceTypeTrader::self()->query("Plasma/Containment", constraint);
    //kDebug() << "constraint was" << constraint << "which got us" << offers.count() << "matches";
    return KPluginInfo::fromServices(offers);
}

KPluginInfo::List Containment::listContainmentsForMimetype(const QString &mimetype)
{
    const QString constraint = QString("'%1' in [X-Plasma-DropMimeTypes]").arg(mimetype);
    //kDebug() << mimetype << constraint;
    const KService::List offers = KServiceTypeTrader::self()->query("Plasma/Containment", constraint);
    return KPluginInfo::fromServices(offers);
}

QStringList Containment::listContainmentTypes()
{
    KPluginInfo::List containmentInfos = listContainments();
    QSet<QString> types;

    foreach (const KPluginInfo &containmentInfo, containmentInfos) {
        QStringList theseTypes = containmentInfo.service()->property("X-Plasma-ContainmentCategories").toStringList();
        foreach (const QString &type, theseTypes) {
            types.insert(type);
        }
    }

    return types.toList();
}

void Containment::dragEnterEvent(QGraphicsSceneDragDropEvent *event)
{
    //kDebug() << immutability() << Mutable << (immutability() == Mutable);
    event->setAccepted(immutability() == Mutable &&
                       (event->mimeData()->hasFormat(static_cast<Corona*>(scene())->appletMimeType()) ||
                        KUrl::List::canDecode(event->mimeData()) ||
                        event->mimeData()->hasFormat(ExtenderItemMimeData::mimeType())));

    if (!event->isAccepted()) {
        // check to see if we have an applet that accepts the format.
        QStringList formats = event->mimeData()->formats();

        foreach (const QString &format, formats) {
            KPluginInfo::List appletList = Applet::listAppletInfoForMimetype(format);
            if (!appletList.isEmpty()) {
                event->setAccepted(true);
                break;
            }
        }

        if (!event->isAccepted()) {
            foreach (const QString &format, formats) {
                KPluginInfo::List wallpaperList = Wallpaper::listWallpaperInfoForMimetype(format);
                if (!wallpaperList.isEmpty()) {
                    event->setAccepted(true);
                    break;
                }
            }
        }
    }

    if (event->isAccepted()) {
        if (d->dropZoneStarted) {
            showDropZone(event->pos().toPoint());
        } else {
            if (!d->showDropZoneDelayTimer) {
                d->showDropZoneDelayTimer = new QTimer(this);
                d->showDropZoneDelayTimer->setInterval(300);
                d->showDropZoneDelayTimer->setSingleShot(true);
                connect(d->showDropZoneDelayTimer, SIGNAL(timeout()), this, SLOT(showDropZoneDelayed()));
            }

            d->dropPoints.insert(0, event->pos());
            d->showDropZoneDelayTimer->start();
        }
    }
}

void Containment::dragLeaveEvent(QGraphicsSceneDragDropEvent *event)
{
    //kDebug() << event->pos() << size().height() << size().width();
    if (d->showDropZoneDelayTimer) {
        d->showDropZoneDelayTimer->stop();
    }

    if (event->pos().y() < 1 || event->pos().y() > size().height() ||
        event->pos().x() < 1 || event->pos().x() > size().width()) {
        showDropZone(QPoint());
        d->dropZoneStarted = false;
    }
}

void ContainmentPrivate::showDropZoneDelayed()
{
    dropZoneStarted = true;
    q->showDropZone(dropPoints.value(0).toPoint());
    dropPoints.remove(0);
}

void Containment::dragMoveEvent(QGraphicsSceneDragDropEvent *event)
{
    QGraphicsItem *item = scene()->itemAt(event->scenePos());
    event->setAccepted(item == this || item == d->toolBox.data() || !item);
    //kDebug() << event->isAccepted() << d->showDropZoneDelayTimer->isActive();
    if (!event->isAccepted()) {
        if (d->showDropZoneDelayTimer) {
            d->showDropZoneDelayTimer->stop();
        }
    } else if (!d->showDropZoneDelayTimer->isActive() && immutability() == Plasma::Mutable) {
        showDropZone(event->pos().toPoint());
    }
}

void Containment::dropEvent(QGraphicsSceneDragDropEvent *event)
{
    if (isContainment()) {
        d->dropData(event->scenePos(), event->screenPos(), event);
    } else {
        Applet::dropEvent(event);
    }
}

void ContainmentPrivate::dropData(QPointF scenePos, QPoint screenPos, QGraphicsSceneDragDropEvent *dropEvent)
{
    if (q->immutability() != Mutable) {
        return;
    }

    QPointF pos = q->mapFromScene(scenePos);
    const QMimeData *mimeData = 0;

    if (dropEvent) {
        mimeData = dropEvent->mimeData();
    } else {
        QClipboard *clipboard = QApplication::clipboard();
        mimeData = clipboard->mimeData(QClipboard::Selection);
        //TODO if that's not supported (ie non-linux) should we try clipboard instead of selection?
    }

    if (!mimeData) {
        //Selection is either empty or not supported on this OS
        kDebug() << "no mime data";
        return;
    }

    //kDebug() << event->mimeData()->text();

    QString appletMimetype(q->corona() ? q->corona()->appletMimeType() : QString());

    if (!appletMimetype.isEmpty() && mimeData->hasFormat(appletMimetype)) {
        QString data = mimeData->data(appletMimetype);
        const QStringList appletNames = data.split('\n', QString::SkipEmptyParts);
        foreach (const QString &appletName, appletNames) {
            //kDebug() << "doing" << appletName;
            QRectF geom(pos, QSize(0, 0));
            q->addApplet(appletName, QVariantList(), geom);
        }
        if (dropEvent) {
            dropEvent->acceptProposedAction();
        }
    } else if (mimeData->hasFormat(ExtenderItemMimeData::mimeType())) {
        kDebug() << "mimetype plasma/extenderitem is dropped, creating internal:extender";
        //Handle dropping extenderitems.
        const ExtenderItemMimeData *extenderData = qobject_cast<const ExtenderItemMimeData*>(mimeData);
        if (extenderData) {
            ExtenderItem *item = extenderData->extenderItem();
            QRectF geometry(pos - extenderData->pointerOffset(), item->size());
            kDebug() << "desired geometry: " << geometry;
            Applet *applet = qobject_cast<ExtenderApplet *>(item->extender() ?  item->extender()->applet() : 0);
            if (applet) {
                qreal left, top, right, bottom;
                applet->getContentsMargins(&left, &top, &right, &bottom);
                applet->setPos(geometry.topLeft() - QPointF(int(left), int(top)));
                applet->show();
            } else {
                applet = addApplet("internal:extender", QVariantList(), geometry, 0, true);
                applet->hide();
                applet->init();
                appletAppeared(applet);
                applet->flushPendingConstraintsEvents();
                applet->d->scheduleModificationNotification();
                applet->adjustSize();
                applet->show();
            }
            item->setExtender(applet->extender());
        }
    } else if (KUrl::List::canDecode(mimeData)) {
        //TODO: collect the mimetypes of available script engines and offer
        //      to create widgets out of the matching URLs, if any
        const KUrl::List urls = KUrl::List::fromMimeData(mimeData);
        foreach (const KUrl &url, urls) {
            if (AccessManager::supportedProtocols().contains(url.protocol())) {
                AccessAppletJob *job = AccessManager::self()->accessRemoteApplet(url);
                if (dropEvent) {
                    dropPoints[job] = dropEvent->pos();
                } else {
                    dropPoints[job] = scenePos;
                }
                QObject::connect(AccessManager::self(), SIGNAL(finished(Plasma::AccessAppletJob*)),
                                 q, SLOT(remoteAppletReady(Plasma::AccessAppletJob*)));
            }
#ifndef PLASMA_NO_KIO
            else {
                KMimeType::Ptr mime = KMimeType::findByUrl(url);
                QString mimeName = mime->name();
                QRectF geom(pos, QSize());
                QVariantList args;
                args << url.url();
                kDebug() << "can decode" << mimeName << args;

                // It may be a directory or a file, let's stat
                KIO::JobFlags flags = KIO::HideProgressInfo;
                KIO::MimetypeJob *job = KIO::mimetype(url, flags);
                if (dropEvent) {
                    dropPoints[job] = dropEvent->pos();
                } else {
                    dropPoints[job] = scenePos;
                }

                QObject::connect(job, SIGNAL(result(KJob*)), q, SLOT(dropJobResult(KJob*)));
                QObject::connect(job, SIGNAL(mimetype(KIO::Job*,QString)),
                                 q, SLOT(mimeTypeRetrieved(KIO::Job*,QString)));

                KMenu *choices = new KMenu("Content dropped");
                choices->addAction(KIcon("process-working"), i18n("Fetching file type..."));
                if (dropEvent) {
                    choices->popup(dropEvent->screenPos());
                } else {
                    choices->popup(screenPos);
                }

                dropMenus[job] = choices;
            }
#endif
        }

        if (dropEvent) {
            dropEvent->acceptProposedAction();
        }
    } else {
        QStringList formats = mimeData->formats();
        QHash<QString, KPluginInfo> seenPlugins;
        QHash<QString, QString> pluginFormats;

        foreach (const QString &format, formats) {
            KPluginInfo::List plugins = Applet::listAppletInfoForMimetype(format);

            foreach (const KPluginInfo &plugin, plugins) {
                if (seenPlugins.contains(plugin.pluginName())) {
                    continue;
                }

                seenPlugins.insert(plugin.pluginName(), plugin);
                pluginFormats.insert(plugin.pluginName(), format);
            }
        }
        //kDebug() << "Mimetype ..." << formats << seenPlugins.keys() << pluginFormats.values();

        QString selectedPlugin;

        if (seenPlugins.isEmpty()) {
            // do nothing
        } else if (seenPlugins.count() == 1) {
            selectedPlugin = seenPlugins.constBegin().key();
        } else {
            KMenu choices;
            QHash<QAction *, QString> actionsToPlugins;
            foreach (const KPluginInfo &info, seenPlugins) {
                QAction *action;
                if (!info.icon().isEmpty()) {
                    action = choices.addAction(KIcon(info.icon()), info.name());
                } else {
                    action = choices.addAction(info.name());
                }

                actionsToPlugins.insert(action, info.pluginName());
            }

            QAction *choice = choices.exec(screenPos);
            if (choice) {
                selectedPlugin = actionsToPlugins[choice];
            }
        }

        if (!selectedPlugin.isEmpty()) {
            if (!dropEvent) {
                // since we may have entered an event loop up above with the menu,
                // the clipboard item may no longer be valid, as QClipboard resets
                // the object behind the back of the application with a zero timer
                // so we fetch it again here
                QClipboard *clipboard = QApplication::clipboard();
                mimeData = clipboard->mimeData(QClipboard::Selection);
            }

            KTemporaryFile tempFile;
            if (mimeData && tempFile.open()) {
                //TODO: what should we do with files after the applet is done with them??
                tempFile.setAutoRemove(false);

                {
                    QDataStream stream(&tempFile);
                    QByteArray data = mimeData->data(pluginFormats[selectedPlugin]);
                    stream.writeRawData(data, data.size());
                }

                QRectF geom(pos, QSize());
                QVariantList args;
                args << tempFile.fileName();
                kDebug() << args;
                tempFile.close();

                q->addApplet(selectedPlugin, args, geom);
            }
        }
    }
}

void ContainmentPrivate::clearDataForMimeJob(KIO::Job *job)
{
#ifndef PLASMA_NO_KIO
    QObject::disconnect(job, 0, q, 0);
    dropPoints.remove(job);
    KMenu *choices = dropMenus.take(job);
    delete choices;
    job->kill();
#endif // PLASMA_NO_KIO
}

void ContainmentPrivate::remoteAppletReady(Plasma::AccessAppletJob *job)
{
    QPointF pos = dropPoints.take(job);
    if (job->error()) {
        //TODO: nice user visible error handling (knotification probably?)
        kDebug() << "remote applet access failed: " << job->errorText();
        return;
    }

    if (!job->applet()) {
        kDebug() << "how did we end up here? if applet is null, the job->error should be nonzero";
        return;
    }

    q->addApplet(job->applet(), pos);
}

void ContainmentPrivate::dropJobResult(KJob *job)
{
#ifndef PLASMA_NO_KIO
    KIO::TransferJob* tjob = dynamic_cast<KIO::TransferJob*>(job);
    if (!tjob) {
        kDebug() << "job is not a KIO::TransferJob, won't handle the drop...";
        clearDataForMimeJob(tjob);
        return;
    }
    if (job->error()) {
        kDebug() << "ERROR" << tjob->error() << ' ' << tjob->errorString();
    }
    // We call mimetypeRetrieved since there might be other mechanisms
    // for finding suitable applets. Cleanup happens there as well.
    mimeTypeRetrieved(qobject_cast<KIO::Job *>(job), QString());
#endif // PLASMA_NO_KIO
}

void ContainmentPrivate::mimeTypeRetrieved(KIO::Job *job, const QString &mimetype)
{
#ifndef PLASMA_NO_KIO
    kDebug() << "Mimetype Job returns." << mimetype;
    KIO::TransferJob* tjob = dynamic_cast<KIO::TransferJob*>(job);
    if (!tjob) {
        kDebug() << "job should be a TransferJob, but isn't";
        clearDataForMimeJob(job);
        return;
    }
    KPluginInfo::List appletList = Applet::listAppletInfoForUrl(tjob->url());
    if (mimetype.isEmpty() && !appletList.count()) {
        clearDataForMimeJob(job);
        kDebug() << "No applets found matching the url (" << tjob->url() << ") or the mimetype (" << mimetype << ")";
        return;
    } else {

        QPointF posi; // will be overwritten with the event's position
        if (dropPoints.keys().contains(tjob)) {
            posi = dropPoints[tjob];
            kDebug() << "Received a suitable dropEvent at" << posi;
        } else {
            kDebug() << "Bailing out. Cannot find associated dropEvent related to the TransferJob";
            clearDataForMimeJob(job);
            return;
        }

        KMenu *choices = dropMenus.value(tjob);
        if (!choices) {
            kDebug() << "Bailing out. No QMenu found for this job.";
            clearDataForMimeJob(job);
            return;
        }

        QVariantList args;
        args << tjob->url().url() << mimetype;

        kDebug() << "Creating menu for:" << mimetype  << posi << args;

        appletList << Applet::listAppletInfoForMimetype(mimetype);
        KPluginInfo::List wallpaperList;
        if (drawWallpaper) {
            if (wallpaper && wallpaper->supportsMimetype(mimetype)) {
                wallpaperList << wallpaper->d->wallpaperDescription;
            } else {
                wallpaperList = Wallpaper::listWallpaperInfoForMimetype(mimetype);
            }
        }

        if (!appletList.isEmpty() || !wallpaperList.isEmpty()) {
            choices->clear();
            QHash<QAction *, QString> actionsToApplets;
            choices->addTitle(i18n("Widgets"));
            foreach (const KPluginInfo &info, appletList) {
                kDebug() << info.name();
                QAction *action;
                if (!info.icon().isEmpty()) {
                    action = choices->addAction(KIcon(info.icon()), info.name());
                } else {
                    action = choices->addAction(info.name());
                }

                actionsToApplets.insert(action, info.pluginName());
                kDebug() << info.pluginName();
            }
            actionsToApplets.insert(choices->addAction(i18n("Icon")), "icon");

            QHash<QAction *, QString> actionsToWallpapers;
            if (!wallpaperList.isEmpty())  {
                choices->addTitle(i18n("Wallpaper"));

                QMap<QString, KPluginInfo> sorted;
                foreach (const KPluginInfo &info, appletList) {
                    sorted.insert(info.name(), info);
                }

                foreach (const KPluginInfo &info, wallpaperList) {
                    QAction *action;
                    if (!info.icon().isEmpty()) {
                        action = choices->addAction(KIcon(info.icon()), info.name());
                    } else {
                        action = choices->addAction(info.name());
                    }

                    actionsToWallpapers.insert(action, info.pluginName());
                }
            }

            QAction *choice = choices->exec();
            if (choice) {
                // Put the job on hold so it can be recycled to fetch the actual content,
                // which is to be expected when something's dropped onto the desktop and
                // an applet is to be created with this URL
                if (!mimetype.isEmpty() && !tjob->error()) {
                    tjob->putOnHold();
                    KIO::Scheduler::publishSlaveOnHold();
                }
                QString plugin = actionsToApplets.value(choice);
                if (plugin.isEmpty()) {
                    //set wallpapery stuff
                    plugin = actionsToWallpapers.value(choice);
                    if (!wallpaper || plugin != wallpaper->pluginName()) {
                        kDebug() << "Wallpaper dropped:" << tjob->url();
                        q->setWallpaper(plugin);
                    }

                    if (wallpaper) {
                        kDebug() << "Wallpaper dropped:" << tjob->url();
                        wallpaper->setUrls(KUrl::List() << tjob->url());
                    }
                } else {
                    addApplet(actionsToApplets[choice], args, QRectF(posi, QSize()));
                }

                clearDataForMimeJob(job);
                return;
            }
        } else {
            // we can at least create an icon as a link to the URL
            addApplet("icon", args, QRectF(posi, QSize()));
        }
    }

    clearDataForMimeJob(job);
#endif // PLASMA_NO_KIO
}

#ifndef KDE_NO_DEPRECATED
const QGraphicsItem *Containment::toolBoxItem() const
{
    return d->toolBox.data();
}
#endif

void Containment::setToolBox(AbstractToolBox *toolBox)
{
    if (d->toolBox.data()) {
        d->toolBox.data()->deleteLater();
    }
    d->toolBox = toolBox;
}

AbstractToolBox *Containment::toolBox() const
{
    return d->toolBox.data();
}

void Containment::resizeEvent(QGraphicsSceneResizeEvent *event)
{
    Applet::resizeEvent(event);

    if (isContainment()) {
        if (d->isPanelContainment()) {
            d->positionPanel();
        } else if (corona()) {
            QMetaObject::invokeMethod(corona(), "layoutContainments");
        }

        if (d->wallpaper) {
            d->wallpaper->setBoundingRect(QRectF(QPointF(0, 0), size()));
        }
    }
}

void Containment::keyPressEvent(QKeyEvent *event)
{
    //kDebug() << "keyPressEvent with" << event->key()
    //         << "and hoping and wishing for a" << Qt::Key_Tab;
    if (event->key() == Qt::Key_Tab) { // && event->modifiers() == 0) {
        if (!d->applets.isEmpty()) {
            kDebug() << "let's give focus to...." << (QObject*)d->applets.first();
            d->applets.first()->setFocus(Qt::TabFocusReason);
        }
    }
}

void Containment::wheelEvent(QGraphicsSceneWheelEvent *event)
{
    event->ignore();
    if (d->appletAt(event->scenePos())) {
        return; //no unexpected click-throughs
    }

    if (d->wallpaper && d->wallpaper->isInitialized()) {
        QGraphicsItem *item = scene()->itemAt(event->scenePos());
        if (item == this) {
            event->ignore();
            d->wallpaper->wheelEvent(event);

            if (event->isAccepted()) {
                return;
            }
        }
    }

    QString trigger = ContainmentActions::eventToString(event);

    if (d->prepareContainmentActions(trigger, event->screenPos())) {
        d->actionPlugins()->value(trigger)->contextEvent(event);
        event->accept();
    } else {
        event->ignore();
        Applet::wheelEvent(event);
    }
}

bool Containment::sceneEventFilter(QGraphicsItem *watched, QEvent *event)
{
    return Applet::sceneEventFilter(watched, event);
}

QVariant Containment::itemChange(GraphicsItemChange change, const QVariant &value)
{
    //FIXME if the applet is moved to another containment we need to unfocus it

    if (isContainment() &&
        (change == QGraphicsItem::ItemSceneHasChanged ||
         change == QGraphicsItem::ItemPositionHasChanged)) {
        switch (d->type) {
            case PanelContainment:
            case CustomPanelContainment:
                d->positionPanel();
                break;
            default:
                if (corona()) {
                    QMetaObject::invokeMethod(corona(), "layoutContainments");
                }
                break;
        }
    }

    return Applet::itemChange(change, value);
}

void Containment::enableAction(const QString &name, bool enable)
{
    QAction *action = this->action(name);
    if (action) {
        action->setEnabled(enable);
        action->setVisible(enable);
    }
}

void Containment::addToolBoxAction(QAction *action)
{
    d->createToolBox();
    if (d->toolBox) {
        d->toolBox.data()->addTool(action);
    }
}

void Containment::removeToolBoxAction(QAction *action)
{
    if (d->toolBox) {
        d->toolBox.data()->removeTool(action);
    }
}

void Containment::setToolBoxOpen(bool open)
{
    if (open) {
        openToolBox();
    } else {
        closeToolBox();
    }
}

bool Containment::isToolBoxOpen() const
{
    return (d->toolBox && d->toolBox.data()->isShowing());
}

void Containment::openToolBox()
{
    if (d->toolBox && !d->toolBox.data()->isShowing()) {
        d->toolBox.data()->setShowing(true);
        emit toolBoxVisibilityChanged(true);
    }
}

void Containment::closeToolBox()
{
    if (d->toolBox && d->toolBox.data()->isShowing()) {
        d->toolBox.data()->setShowing(false);
        emit toolBoxVisibilityChanged(false);
    }
}

void Containment::addAssociatedWidget(QWidget *widget)
{
    Applet::addAssociatedWidget(widget);
    if (d->focusedApplet) {
        d->focusedApplet->addAssociatedWidget(widget);
    }

    foreach (const Applet *applet, d->applets) {
        if (applet->d->activationAction) {
            widget->addAction(applet->d->activationAction);
        }
    }
}

void Containment::removeAssociatedWidget(QWidget *widget)
{
    Applet::removeAssociatedWidget(widget);
    if (d->focusedApplet) {
        d->focusedApplet->removeAssociatedWidget(widget);
    }

    foreach (const Applet *applet, d->applets) {
        if (applet->d->activationAction) {
            widget->removeAction(applet->d->activationAction);
        }
    }
}

void Containment::setDrawWallpaper(bool drawWallpaper)
{
    d->drawWallpaper = drawWallpaper;
    if (drawWallpaper) {
        KConfigGroup cfg = config();
        const QString wallpaper = cfg.readEntry("wallpaperplugin", defaultWallpaper);
        const QString mode = cfg.readEntry("wallpaperpluginmode", defaultWallpaperMode);
        setWallpaper(wallpaper, mode);
    } else {
        delete d->wallpaper;
        d->wallpaper = 0;
    }
}

bool Containment::drawWallpaper()
{
    return d->drawWallpaper;
}

void Containment::setWallpaper(const QString &pluginName, const QString &mode)
{
    KConfigGroup cfg = config();
    bool newPlugin = true;
    bool newMode = true;

    if (d->drawWallpaper) {
        if (d->wallpaper) {
            // we have a wallpaper, so let's decide whether we need to swap it out
            if (d->wallpaper->pluginName() != pluginName) {
                delete d->wallpaper;
                d->wallpaper = 0;
            } else {
                // it's the same plugin, so let's save its state now so when
                // we call restore later on we're safe
                newMode = d->wallpaper->renderingMode().name() != mode;
                newPlugin = false;
            }
        }

        if (!pluginName.isEmpty() && !d->wallpaper) {
            d->wallpaper = Plasma::Wallpaper::load(pluginName);
        }

        if (d->wallpaper) {
            d->wallpaper->setParent(this);
            d->wallpaper->setBoundingRect(QRectF(QPointF(0, 0), size()));
            d->wallpaper->setRenderingMode(mode);

            if (newPlugin) {
                cfg.writeEntry("wallpaperplugin", pluginName);
            }

            if (d->wallpaper->isInitialized()) {
                KConfigGroup wallpaperConfig = KConfigGroup(&cfg, "Wallpaper");
                wallpaperConfig = KConfigGroup(&wallpaperConfig, pluginName);
                d->wallpaper->restore(wallpaperConfig);
            }

            if (newMode) {
                cfg.writeEntry("wallpaperpluginmode", mode);
            }
        }

        update();
    }

    if (!d->wallpaper) {
        cfg.deleteEntry("wallpaperplugin");
        cfg.deleteEntry("wallpaperpluginmode");
    }

    if (newPlugin || newMode) {
        if (newPlugin && d->wallpaper) {
            connect(d->wallpaper, SIGNAL(configureRequested()), this, SLOT(requestConfiguration()));
            connect(d->wallpaper, SIGNAL(configNeedsSaving()), this, SIGNAL(configNeedsSaving()));
        }

        emit configNeedsSaving();
    }
}

Plasma::Wallpaper *Containment::wallpaper() const
{
    return d->wallpaper;
}

void Containment::setContainmentActions(const QString &trigger, const QString &pluginName)
{
    KConfigGroup cfg = containmentActionsConfig();
    ContainmentActions *plugin = 0;

    if (d->actionPlugins()->contains(trigger)) {
        plugin = d->actionPlugins()->value(trigger);
        if (plugin->pluginName() != pluginName) {
            d->actionPlugins()->remove(trigger);
            delete plugin;
            plugin=0;
        }
    }
    if (pluginName.isEmpty()) {
        cfg.deleteEntry(trigger);
    } else if (plugin) {
        //it already existed, just reload config
        if (plugin->isInitialized()) {
            plugin->setContainment(this); //to be safe
            //FIXME make a truly unique config group
            KConfigGroup pluginConfig = KConfigGroup(&cfg, trigger);
            plugin->restore(pluginConfig);
        }
    } else {
        switch (d->containmentActionsSource) {
        case ContainmentPrivate::Activity:
            //FIXME
        case ContainmentPrivate::Local:
            plugin = ContainmentActions::load(this, pluginName);
            break;
        default:
            plugin = ContainmentActions::load(0, pluginName);
        }
        if (plugin) {
            cfg.writeEntry(trigger, pluginName);
            d->actionPlugins()->insert(trigger, plugin);
        } else {
            //bad plugin... gets removed. is this a feature or a bug?
            cfg.deleteEntry(trigger);
        }
    }

    emit configNeedsSaving();
}

QStringList Containment::containmentActionsTriggers()
{
    return d->actionPlugins()->keys();
}

QString Containment::containmentActions(const QString &trigger)
{
    ContainmentActions *c = d->actionPlugins()->value(trigger);
    return c ? c->pluginName() : QString();
}

void Containment::setActivity(const QString &activity)
{
    Context *context = d->context();
    if (context->currentActivity() != activity) {
        context->setCurrentActivity(activity);
    }
}

void ContainmentPrivate::onContextChanged(Plasma::Context *con)
{
    foreach (Applet *a, applets) {
        a->updateConstraints(ContextConstraint);
    }

    KConfigGroup c = q->config();
    QString act = con->currentActivityId();

    //save anything that's been set (boy I hope this avoids overwriting things)
    //FIXME of course if the user sets the name to an empty string we have a bug
    //but once we get context retrieving the name as soon as the id is set, this issue should go away
    if (!act.isEmpty()) {
        c.writeEntry("activityId", act);
    }
    act = con->currentActivity();
    if (!act.isEmpty()) {
        c.writeEntry("activity", act);
    }

    if (toolBox) {
        toolBox.data()->update();
    }
    emit q->configNeedsSaving();
    emit q->contextChanged(con);
}

QString Containment::activity() const
{
    return d->context()->currentActivity();
}

Context *Containment::context() const
{
    return d->context();
}

Context *ContainmentPrivate::context()
{
    if (!con) {
        con = new Context(q);
        q->connect(con, SIGNAL(changed(Plasma::Context*)),
                   q, SLOT(onContextChanged(Plasma::Context*)));
    }

    return con;
}

KActionCollection* ContainmentPrivate::actions()
{
    return static_cast<Applet*>(q)->d->actions;
}

void ContainmentPrivate::focusApplet(Plasma::Applet *applet)
{
    if (focusedApplet == applet) {
        return;
    }

    QList<QWidget *> widgets = actions()->associatedWidgets();
    if (focusedApplet) {
        foreach (QWidget *w, widgets) {
            focusedApplet->removeAssociatedWidget(w);
        }
    }

    if (applet && applets.contains(applet)) {
        //kDebug() << "switching to" << applet->name();
        focusedApplet = applet;
        foreach (QWidget *w, widgets) {
            focusedApplet->addAssociatedWidget(w);
        }

        if (!focusedApplet->hasFocus()) {
            focusedApplet->setFocus(Qt::ShortcutFocusReason);
        }
    } else {
        focusedApplet = 0;
    }
}

void Containment::focusNextApplet()
{
    if (d->applets.isEmpty()) {
        return;
    }
    int index = d->focusedApplet ? d->applets.indexOf(d->focusedApplet) + 1 : 0;
    if (index >= d->applets.size()) {
        index = 0;
    }
    kDebug() << "index" << index;
    d->focusApplet(d->applets.at(index));
}

void Containment::focusPreviousApplet()
{
    if (d->applets.isEmpty()) {
        return;
    }
    int index = d->focusedApplet ? d->applets.indexOf(d->focusedApplet) - 1 : -1;
    if (index < 0) {
        index = d->applets.size() - 1;
    }
    kDebug() << "index" << index;
    d->focusApplet(d->applets.at(index));
}

void Containment::destroy()
{
    destroy(true);
}

void Containment::showConfigurationInterface()
{
    Applet::showConfigurationInterface();
}

void Containment::configChanged()
{
}

void ContainmentPrivate::configChanged()
{
    if (drawWallpaper) {
        KConfigGroup group = q->config();
        q->setWallpaper(group.readEntry("wallpaperplugin", defaultWallpaper),
                        group.readEntry("wallpaperpluginmode", defaultWallpaperMode));
    }
}

void ContainmentPrivate::requestConfiguration()
{
    emit q->configureRequested(q);
}

void ContainmentPrivate::checkStatus(Plasma::ItemStatus appletStatus)
{
    //kDebug() << "================== "<< appletStatus << q->status();
    if (appletStatus == q->status()) {
        emit q->newStatus(appletStatus);
        return;
    }

    if (appletStatus < q->status()) {
        // check to see if any other applet has a higher status, and stick with that
        // if we do
        foreach (Applet *applet, applets) {
            if (applet->status() > appletStatus) {
                appletStatus = applet->status();
            }
        }
    }

    q->setStatus(appletStatus);
}

void Containment::destroy(bool confirm)
{
    if (immutability() != Mutable || Applet::d->transient) {
        return;
    }

    if (isContainment() && confirm) {
        //FIXME: should not be blocking
        const QString title = i18nc("@title:window %1 is the name of the containment", "Remove %1", name());
        KGuiItem remove = KStandardGuiItem::remove();
        remove.setText(title);
        if (KMessageBox::warningContinueCancel(view(),
            i18nc("%1 is the name of the containment", "Do you really want to remove this %1?", name()),
            title, remove) != KMessageBox::Continue) {
            return;
        }
    }

    Applet::destroy();
}

void ContainmentPrivate::createToolBox()
{
    if (!toolBox && KAuthorized::authorizeKAction("plasma/containment_context_menu")) {
        toolBox = Plasma::AbstractToolBox::load(q->corona()->preferredToolBoxPlugin(type), QVariantList(), q);

        if (toolBox) {
            QObject::connect(toolBox.data(), SIGNAL(toggled()), q, SIGNAL(toolBoxToggled()));
            QObject::connect(toolBox.data(), SIGNAL(toggled()), q, SLOT(updateToolBoxVisibility()));

            positionToolBox();
        }
    }
}

void ContainmentPrivate::positionToolBox()
{
    QMetaObject::invokeMethod(toolBox.data(), "reposition");
}

void ContainmentPrivate::updateToolBoxVisibility()
{
    emit q->toolBoxVisibilityChanged(toolBox.data()->isShowing());
}

void ContainmentPrivate::triggerShowAddWidgets()
{
    emit q->showAddWidgetsInterface(QPointF());
}

void ContainmentPrivate::containmentConstraintsEvent(Plasma::Constraints constraints)
{
    if (!q->isContainment()) {
        return;
    }

    //kDebug() << "got containmentConstraintsEvent" << constraints << (QObject*)toolBox;
    if (constraints & Plasma::ImmutableConstraint) {
        //update actions
        checkRemoveAction();
        const bool unlocked = q->immutability() == Mutable;
        q->setAcceptDrops(unlocked);
        q->enableAction("add widgets", unlocked);

        // tell the applets too
        foreach (Applet *a, applets) {
            a->setImmutability(q->immutability());
            a->updateConstraints(ImmutableConstraint);
        }
    }

    // pass on the constraints that are relevant here
    Constraints appletConstraints = NoConstraint;
    if (constraints & FormFactorConstraint) {
        appletConstraints |= FormFactorConstraint;
    }

    if (constraints & ScreenConstraint) {
        appletConstraints |= ScreenConstraint;
    }

    if (appletConstraints != NoConstraint) {
        foreach (Applet *applet, applets) {
            applet->updateConstraints(appletConstraints);
        }
    }

    if (toolBox && (constraints & Plasma::SizeConstraint ||
                    constraints & Plasma::FormFactorConstraint ||
                    constraints & Plasma::ScreenConstraint ||
                    constraints & Plasma::StartupCompletedConstraint)) {
        //kDebug() << "Positioning toolbox";
        positionToolBox();
    }

    if (constraints & Plasma::StartupCompletedConstraint && type < Containment::CustomContainment) {
        q->addToolBoxAction(q->action("remove"));
        checkRemoveAction();
    }
}

Applet *ContainmentPrivate::addApplet(const QString &name, const QVariantList &args,
                                      const QRectF &appletGeometry, uint id, bool delayInit)
{
    if (!q->isContainment()) {
        return 0;
    }

    if (!delayInit && q->immutability() != Mutable) {
        kDebug() << "addApplet for" << name << "requested, but we're currently immutable!";
        return 0;
    }

    QGraphicsView *v = q->view();
    if (v) {
        v->setCursor(Qt::BusyCursor);
    }

    Applet *applet = Applet::load(name, id, args);
    if (v) {
        v->unsetCursor();
    }

    if (!applet) {
        kDebug() << "Applet" << name << "could not be loaded.";
        applet = new Applet(0, QString(), id);
        applet->setFailedToLaunch(true, i18n("Could not find requested component: %1", name));
    }

    //kDebug() << applet->name() << "sizehint:" << applet->sizeHint() << "geometry:" << applet->geometry();

    q->addApplet(applet, appletGeometry.topLeft(), delayInit);
    return applet;
}

bool ContainmentPrivate::regionIsEmpty(const QRectF &region, Applet *ignoredApplet) const
{
    foreach (Applet *applet, applets) {
        if (applet != ignoredApplet && applet->geometry().intersects(region)) {
            return false;
        }
    }
    return true;
}

void ContainmentPrivate::appletDestroyed(Plasma::Applet *applet)
{
    applets.removeAll(applet);
    if (focusedApplet == applet) {
        focusedApplet = 0;
    }

    emit q->appletRemoved(applet);
    emit q->configNeedsSaving();
}

void ContainmentPrivate::appletAppearAnimationComplete()
{
    Animation *anim = qobject_cast<Animation *>(q->sender());
    if (anim) {
        Applet *applet = qobject_cast<Applet*>(anim->targetWidget());
        if (applet) {
            appletAppeared(applet);
        }
    }
}

void ContainmentPrivate::appletAppeared(Applet *applet)
{
    //kDebug() << type << Containment::DesktopContainment;
    KConfigGroup *cg = applet->d->mainConfigGroup();
    applet->save(*cg);
    emit q->configNeedsSaving();
}

void ContainmentPrivate::positionPanel(bool force)
{
    if (!q->scene()) {
        kDebug() << "no scene yet";
        return;
    }

    // already positioning the panel - avoid infinite loops
    if (ContainmentPrivate::s_positioningPanels) {
        return;
    }

    // we position panels in negative coordinates, and stack all horizontal
    // and all vertical panels with each other.


    const QPointF p = q->pos();

    if (!force &&
        p.y() + q->size().height() < -INTER_CONTAINMENT_MARGIN &&
        q->scene()->collidingItems(q).isEmpty()) {
        // already positioned and not running into any other panels
        return;
    }


    QPointF newPos = preferredPanelPos(q->corona());
    if (p != newPos) {
        ContainmentPrivate::s_positioningPanels = true;
        q->setPos(newPos);
        ContainmentPrivate::s_positioningPanels = false;
    }
}

bool ContainmentPrivate::isPanelContainment() const
{
    return type == Containment::PanelContainment || type == Containment::CustomPanelContainment;
}

QPointF ContainmentPrivate::preferredPos(Corona *corona) const
{
    Q_ASSERT(corona);

    if (isPanelContainment()) {
        //kDebug() << "is a panel, so put it at" << preferredPanelPos(corona);
        return preferredPanelPos(corona);
    }

    QPointF pos(0, 0);
    QTransform t;
    while (QGraphicsItem *i = corona->itemAt(pos, t)) {
        pos.setX(i->scenePos().x() + i->boundingRect().width() + 10);
    }

    //kDebug() << "not a panel, put it at" << pos;
    return pos;
}

QPointF ContainmentPrivate::preferredPanelPos(Corona *corona) const
{
    Q_ASSERT(corona);

    //TODO: research how non-Horizontal, non-Vertical (e.g. Planar) panels behave here
    bool horiz = formFactor == Plasma::Horizontal;
    qreal bottom = horiz ? 0 : VERTICAL_STACKING_OFFSET;
    qreal lastHeight = 0;

    // this should be ok for small numbers of panels, but if we ever end
    // up managing hundreds of them, this simplistic alogrithm will
    // likely be too slow.
    foreach (const Containment *other, corona->containments()) {
        if (other == q ||
            !other->d->isPanelContainment() ||
            horiz != (other->formFactor() == Plasma::Horizontal)) {
            // only line up with panels of the same orientation
            continue;
        }

        if (horiz) {
            qreal y = other->pos().y();
            if (y < bottom) {
                lastHeight = other->size().height();
                bottom = y;
            }
        } else {
            qreal width = other->size().width();
            qreal x = other->pos().x() + width;
            if (x > bottom) {
                lastHeight = width;
                bottom = x + lastHeight;
            }
        }
    }

    // give a space equal to the height again of the last item so there is
    // room to grow.
    QPointF newPos;
    if (horiz) {
        bottom -= lastHeight + INTER_CONTAINMENT_MARGIN;
        //TODO: fix x position for non-flush-left panels
        kDebug() << "moved to" << QPointF(0, bottom - q->size().height());
        newPos = QPointF(0, bottom - q->size().height());
    } else {
        bottom += lastHeight + INTER_CONTAINMENT_MARGIN;
        //TODO: fix y position for non-flush-top panels
        kDebug() << "moved to" << QPointF(bottom + q->size().width(), -INTER_CONTAINMENT_MARGIN - q->size().height());
        newPos = QPointF(bottom + q->size().width(), -INTER_CONTAINMENT_MARGIN - q->size().height());
    }

    return newPos;
}


bool ContainmentPrivate::prepareContainmentActions(const QString &trigger, const QPoint &screenPos, KMenu *menu)
{
    ContainmentActions *plugin = actionPlugins()->value(trigger);
    if (!plugin) {
        return false;
    }
    plugin->setContainment(q);

    if (!plugin->isInitialized()) {
        KConfigGroup cfg = q->containmentActionsConfig();
        KConfigGroup pluginConfig = KConfigGroup(&cfg, trigger);
        plugin->restore(pluginConfig);
    }

    if (plugin->configurationRequired()) {
        KMenu *localMenu = menu ? menu : new KMenu();

        localMenu->addTitle(i18n("This plugin needs to be configured"));
        localMenu->addAction(q->action("configure"));

        if (!menu) {
            localMenu->exec(screenPos);
            delete localMenu;
        }

        return false;
    } else if (menu) {
        QList<QAction*> actions = plugin->contextualActions();
        if (actions.isEmpty()) {
            //it probably didn't bother implementing the function. give the user a chance to set
            //a better plugin.  note that if the user sets no-plugin this won't happen...
            if (!isPanelContainment() && q->action("configure")) {
                menu->addAction(q->action("configure"));
            }
        } else {
            menu->addActions(actions);
        }
    }

    return true;
}

KConfigGroup Containment::containmentActionsConfig()
{
    KConfigGroup cfg;
    switch (d->containmentActionsSource) {
    case ContainmentPrivate::Local:
        cfg = config();
        cfg = KConfigGroup(&cfg, "ActionPlugins");
        break;
    case ContainmentPrivate::Activity:
        cfg = KConfigGroup(corona()->config(), "Activities");
        cfg = KConfigGroup(&cfg, d->context()->currentActivityId());
        cfg = KConfigGroup(&cfg, "ActionPlugins");
        break;
    default:
        cfg = KConfigGroup(corona()->config(), "ActionPlugins");
    }
    return cfg;
}

QHash<QString, ContainmentActions*> * ContainmentPrivate::actionPlugins()
{
    switch (containmentActionsSource) {
        case Activity:
            //FIXME
        case Local:
            return &localActionPlugins;
        default:
            return &globalActionPlugins;
    }
}

} // Plasma namespace

#include "containment.moc"

