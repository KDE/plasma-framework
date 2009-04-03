/*
 *   Copyright 2007 by Aaron Seigo <aseigo@kde.org>
 *   Copyright 2008 by Ménard Alexis <darktears31@gmail.com>
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

#include <QFile>
#include <QGraphicsSceneContextMenuEvent>
#include <QGraphicsView>
#include <QMimeData>
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QGraphicsLayout>
#include <QGraphicsLinearLayout>

#include <kaction.h>
#include <kapplication.h>
#include <kauthorized.h>
#include <kicon.h>
#include <kmenu.h>
#include <kmessagebox.h>
#include <kmimetype.h>
#include <krun.h>
#include <kservicetypetrader.h>
#include <kstandarddirs.h>
#include <ktemporaryfile.h>
#include <kwindowsystem.h>

#include "animator.h"
#include "context.h"
#include "corona.h"
#include "extenderitem.h"
#include "svg.h"
#include "wallpaper.h"

#include "private/applet_p.h"
#include "private/applethandle_p.h"
#include "private/desktoptoolbox_p.h"
#include "private/extenderitemmimedata_p.h"
#include "private/paneltoolbox_p.h"

namespace Plasma
{

bool ContainmentPrivate::s_positioning = false;
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

Containment::~Containment()
{
    delete d;
}

void Containment::init()
{
    if (!isContainment()) {
        return;
    }

    setCacheMode(NoCache);
    setFlag(QGraphicsItem::ItemIsMovable, false);
    setFlag(QGraphicsItem::ItemClipsChildrenToShape, false);
    setAcceptDrops(true);
    setAcceptsHoverEvents(true);

    //TODO: would be nice to not do this on init, as it causes Animator to init
    connect(Animator::self(), SIGNAL(animationFinished(QGraphicsItem*,Plasma::Animator::Animation)),
            this, SLOT(containmentAppletAnimationComplete(QGraphicsItem*,Plasma::Animator::Animation)));

    if (d->type == NoContainmentType) {
        setContainmentType(DesktopContainment);
    }

    //common actions
    bool unlocked = immutability() == Mutable;

    KAction *appletBrowserAction = new KAction(i18n("Add Widgets..."), this);
    appletBrowserAction->setIcon(KIcon("list-add"));
    appletBrowserAction->setVisible(unlocked);
    appletBrowserAction->setEnabled(unlocked);
    connect(appletBrowserAction, SIGNAL(triggered()), this, SLOT(triggerShowAddWidgets()));
    appletBrowserAction->setShortcut(QKeySequence("alt+d,a"));
    d->actions().addAction("add widgets", appletBrowserAction);

    KAction *action = new KAction(i18n("Next Widget"), this);
    //no icon
    connect(action, SIGNAL(triggered()), this, SLOT(focusNextApplet()));
    action->setShortcut(QKeySequence("alt+d,n"));
    d->actions().addAction("next applet", action);

    action = new KAction(i18n("Previous Widget"), this);
    //no icon
    connect(action, SIGNAL(triggered()), this, SLOT(focusPreviousApplet()));
    action->setShortcut(QKeySequence("alt+d,p"));
    d->actions().addAction("previous applet", action);

    if (immutability() != SystemImmutable && corona()) {
        QAction *lockDesktopAction = corona()->action("lock widgets");
        //keep a pointer so nobody notices it moved to corona
        if (lockDesktopAction) {
            d->actions().addAction("lock widgets", lockDesktopAction);
        }
    }

    if (d->type != PanelContainment &&
        d->type != CustomPanelContainment) {
        KAction *zoomAction = new KAction(i18n("Zoom In"), this);
        zoomAction->setIcon(KIcon("zoom-in"));
        connect(zoomAction, SIGNAL(triggered(bool)), this, SLOT(zoomIn()));
        //two shortcuts because I hate ctrl-+ but others expect it
        QList<QKeySequence> keys;
        keys << QKeySequence("alt+d,+");
        keys << QKeySequence("alt+d,=");
        zoomAction->setShortcuts(keys);
        d->actions().addAction("zoom in", zoomAction);

        if (corona()) {
            QAction *action = corona()->action("zoom out");
            if (action) {
                d->actions().addAction("zoom out", action);
            }
            action = corona()->action("add sibling containment");
            if (action) {
                d->actions().addAction("add sibling containment", action);
            }
        }

        if (d->type == DesktopContainment && d->toolBox) {
            d->toolBox->addTool(this->action("add widgets"));
            d->toolBox->addTool(this->action("zoom in"));

            //TODO: do we need some way to allow this be overridden?
            //      it's always available because shells rely on this
            //      to offer their own custom configuration as well
            QAction *configureContainment = this->action("configure");
            if (configureContainment) {
                d->toolBox->addTool(configureContainment);
            }
        }

        //Set a default wallpaper the first time the containment is created,
        //for instance from the toolbox by the user
        if (d->drawWallpaper) {
            setDrawWallpaper(true);
        }
    }
}

// helper function for sorting the list of applets
bool appletConfigLessThan(const KConfigGroup &c1, const KConfigGroup &c2)
{
    QPointF p1 = c1.readEntry("geometry", QRectF()).topLeft();
    QPointF p2 = c2.readEntry("geometry", QRectF()).topLeft();

    if (!qFuzzyCompare(p1.x(), p2.x())) {
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
    setGeometry(geo);

    setLocation((Plasma::Location)group.readEntry("location", (int)d->location));
    setFormFactor((Plasma::FormFactor)group.readEntry("formfactor", (int)d->formFactor));
    setScreen(group.readEntry("screen", d->screen), group.readEntry("desktop", d->desktop));
    setActivity(group.readEntry("activity", QString()));

    flushPendingConstraintsEvents();
    restoreContents(group);
    setImmutability((ImmutabilityType)group.readEntry("immutability", (int)Mutable));

    setWallpaper(group.readEntry("wallpaperplugin", defaultWallpaper),
                 group.readEntry("wallpaperpluginmode", defaultWallpaperMode));
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
    group.writeEntry("screen", d->screen);
    group.writeEntry("desktop", d->desktop);
    group.writeEntry("formfactor", (int)d->formFactor);
    group.writeEntry("location", (int)d->location);
    group.writeEntry("activity", d->context()->currentActivity());

    if (d->toolBox) {
        d->toolBox->save(group);
    }

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

        Applet *applet = d->addApplet(plugin, QVariantList(),
                                      appletConfig.readEntry("geometry", QRectF()),
                                      appId, true);
        applet->restore(appletConfig);
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

    delete d->toolBox;
    d->toolBox = 0;
    d->type = type;

    if (!isContainment()) {
        return;
    }

    if (type == DesktopContainment || type == PanelContainment) {
        d->createToolBox();
    }

    d->checkRemoveAction();
}

Corona *Containment::corona() const
{
    return dynamic_cast<Corona*>(scene());
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
    event->ignore();
    if (d->wallpaper && d->wallpaper->isInitialized()) {
        QGraphicsItem *item = scene()->itemAt(event->scenePos());
        if (item == this) {
            d->wallpaper->mousePressEvent(event);
        }
    }

    if (event->isAccepted()) {
        setFocus(Qt::MouseFocusReason);
    } else {
        event->accept();
        Applet::mousePressEvent(event);
    }
}

void Containment::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    event->ignore();
    if (d->wallpaper && d->wallpaper->isInitialized()) {
        QGraphicsItem *item = scene()->itemAt(event->scenePos());
        if (item == this) {
            d->wallpaper->mouseReleaseEvent(event);
        }
    }

    if (!event->isAccepted()) {
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
    d->showContextMenu(mapToScene(containmentPos), screenPos, false);
}

void Containment::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    //kDebug() << "let's see if we manage to get a context menu here, huh";
    if (!isContainment() || !scene() || !KAuthorized::authorizeKAction("desktop_contextmenu")) {
        Applet::contextMenuEvent(event);
        return;
    }

    if (!d->showContextMenu(event->scenePos(), event->screenPos(), true)) {
        Applet::contextMenuEvent(event);
    } else {
        event->accept();
    }
}

void ContainmentPrivate::containmentActions(KMenu &desktopMenu)
{
    if (static_cast<Corona*>(q->scene())->immutability() != Mutable &&
        !KAuthorized::authorizeKAction("unlock_desktop")) {
        //kDebug() << "immutability";
        return;
    }

    //get base context actions
    QList<QAction*> actions = q->contextualActions();

    //find the separator to insert the activity settings before it
    QAction *separatorAction = 0;

    //TODO: should a submenu be created if there are too many containment specific
    //      actions? see folderview containment
    foreach (QAction *action, actions) {
        if (action) {
            desktopMenu.addAction(action);
            if (action->isSeparator()) {
                separatorAction = action;
            }
        }
    }

    desktopMenu.addSeparator();

    if (type == Containment::DesktopContainment) {
        desktopMenu.addAction(q->action("configure"));
    }
}

void ContainmentPrivate::appletActions(KMenu &desktopMenu, Applet *applet, bool includeApplet)
{
    QList<QAction*> actions;

    if (includeApplet) {
        actions = applet->contextualActions();
        if (!actions.isEmpty()) {
            foreach (QAction *action, actions) {
                if (action) {
                    desktopMenu.addAction(action);
                }
            }
        }
    }

    if (applet->hasConfigurationInterface()) {
        QAction *configureApplet = applet->d->actions.action("configure");
        if (configureApplet) {
            desktopMenu.addAction(configureApplet);
        }
    }

    KMenu *containmentMenu = new KMenu(i18nc("%1 is the name of the containment", "%1 Options", q->name()), &desktopMenu);
    containmentActions(*containmentMenu);
    if (!containmentMenu->isEmpty()) {
        int enabled = 0;
        //count number of real actions
        foreach(QAction *action, containmentMenu->actions()) {
            if(action->isEnabled() && !action->isSeparator()) {
                enabled++;
            }
        }

        if (enabled > 0) {
            desktopMenu.addSeparator();
        }

        //if there is only one, don't create a submenu
        if(enabled < 2) {
            foreach(QAction *action, containmentMenu->actions()) {
                desktopMenu.addAction(action); 
            }
        } else {
            desktopMenu.addMenu(containmentMenu);
        }
    }

    if (static_cast<Corona*>(q->scene())->immutability() == Mutable) {
        if (!desktopMenu.isEmpty()) {
            desktopMenu.addSeparator();
        }

        QAction *closeApplet = applet->d->actions.action("remove");
        if (!closeApplet) { //unlikely but not impossible
            closeApplet = new QAction(i18nc("%1 is the name of the applet", "Remove this %1", applet->name()), &desktopMenu);
            closeApplet->setIcon(KIcon("edit-delete"));
            QObject::connect(closeApplet, SIGNAL(triggered(bool)), applet, SLOT(destroy()));
        }
        desktopMenu.addAction(closeApplet);
    }
}

bool ContainmentPrivate::showContextMenu(const QPointF &point,
                                         const QPoint &screenPos, bool includeApplet)
{
    Applet *applet = 0;

    QGraphicsItem *item = q->scene()->itemAt(point);
    if (item == q) {
        item = 0;
    }

    while (item) {
        applet = qgraphicsitem_cast<Applet*>(item);
        if (applet && !applet->isContainment()) {
            break;
        }

        // applet may have a value due to finding a containment!
        applet = 0;
        item = item->parentItem();
    }

    KMenu desktopMenu;
    //kDebug() << "context menu event " << (QObject*)applet;
    if (applet) {
        appletActions(desktopMenu, applet, includeApplet);
    } else {
        containmentActions(desktopMenu);
    }

    if (!desktopMenu.isEmpty()) {
        //kDebug() << "executing at" << screenPos;
        desktopMenu.exec(screenPos);
        return true;
    }

    return false;
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

    if (d->toolBox) {
        if (d->formFactor == Vertical) {
            d->toolBox->setCorner(ToolBox::Bottom);
            //defaults to horizontal
        } else if (QApplication::layoutDirection() == Qt::RightToLeft) {
            d->toolBox->setCorner(ToolBox::Left);
        } else {
            d->toolBox->setCorner(ToolBox::Right);
        }
    }

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
        disconnect(applet, 0, currentContainment, 0);
        applet->removeSceneEventFilter(currentContainment);
        KConfigGroup oldConfig = applet->config();
        currentContainment->d->applets.removeAll(applet);
        if (currentContainment->d->handles.contains(applet)) {
            currentContainment->d->handles.remove(applet);
        }
        applet->setParentItem(this);

        // now move the old config to the new location
        //FIXME: this doesn't seem to get the actual main config group containing plugin=, etc
        KConfigGroup c = config().group("Applets").group(QString::number(applet->id()));
        oldConfig.reparent(&c);
        applet->d->resetConfigurationObject();

        disconnect(applet, SIGNAL(activate()), currentContainment, SIGNAL(activate()));
    } else {
        applet->setParentItem(this);
    }

    d->applets << applet;

    connect(applet, SIGNAL(configNeedsSaving()), this, SIGNAL(configNeedsSaving()));
    connect(applet, SIGNAL(releaseVisualFocus()), this, SIGNAL(releaseVisualFocus()));
    connect(applet, SIGNAL(appletDestroyed(Plasma::Applet*)), this, SLOT(appletDestroyed(Plasma::Applet*)));
    connect(applet, SIGNAL(activate()), this, SIGNAL(activate()));

    if (pos != QPointF(-1, -1)) {
        applet->setPos(pos);
    }

    if (delayInit || currentContainment) {
        if (d->type == DesktopContainment) {
            applet->installSceneEventFilter(this);
            //applet->setWindowFlags(Qt::Window);
        }
    } else {
        applet->init();
        Animator::self()->animateItem(applet, Animator::AppearAnimation);
    }

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
    Q_ASSERT(corona());
    int numScreens = corona()->numScreens();
    if (newScreen < -1) {
        newScreen = -1;
    }

    // -1 == All desktops
    if (newDesktop < -1 || newDesktop > KWindowSystem::numberOfDesktops() - 1) {
        newDesktop = -1;
    }

    kDebug() << "setting screen to " << newScreen << newDesktop << "and type is" << d->type;

    Containment *swapScreensWith(0);
    if (d->type == DesktopContainment || d->type >= CustomContainment) {
        // we want to listen to changes in work area if our screen changes
        if (d->screen < 0 && newScreen > -1) {
            connect(KWindowSystem::self(), SIGNAL(workAreaChanged()), this, SLOT(positionToolBox()));
        } else if (newScreen < 0) {
            disconnect(KWindowSystem::self(), SIGNAL(workAreaChanged()), this, SLOT(positionToolBox()));
        }

        if (newScreen > -1 && corona()) {
            // sanity check to make sure someone else doesn't have this screen already!
            Containment *currently = corona()->containmentForScreen(newScreen, newDesktop);
            if (currently && currently != this) {
                kDebug() << "currently is on screen" << currently->screen()
                         << "and is" << currently->name()
                         << (QObject*)currently << (QObject*)this;
                currently->setScreen(-1, newDesktop);
                swapScreensWith = currently;
            }
        }
    }

    if (newScreen < numScreens && newScreen > -1) {
        if (d->type == DesktopContainment ||
            d->type >= CustomContainment) {
            resize(corona()->screenGeometry(newScreen).size());
        }
    }

    int oldDesktop = d->desktop;
    d->desktop = newDesktop;

    int oldScreen = d->screen;
    d->screen = newScreen;

    updateConstraints(Plasma::ScreenConstraint);

    if (oldScreen != newScreen) {
        emit screenChanged(oldScreen, newScreen, this);

        KConfigGroup c = config();
        c.writeEntry("screen", d->screen);
        emit configNeedsSaving();
    }

    if (swapScreensWith) {
        swapScreensWith->setScreen(oldScreen, oldDesktop);
    }

    d->checkRemoveAction();
}

int Containment::screen() const
{
    return d->screen;
}

int Containment::desktop() const
{
    return d->desktop;
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
        constraint.append("not exist [X-KDE-ParentApp]");
    } else {
        constraint.append("[X-KDE-ParentApp] == '").append(parentApp).append("'");
    }

    if (!type.isEmpty()) {
        if (!constraint.isEmpty()) {
            constraint.append(" and ");
        }

        constraint.append("'").append(type).append("' ~in [X-Plasma-ContainmentCategories] ~in '");
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
    QString constraint = QString("'%1' in [X-Plasma-DropMimeTypes]").arg(mimetype);
    //kDebug() << mimetype << constraint;
    KService::List offers = KServiceTypeTrader::self()->query("Plasma/Containment", constraint);
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
    }

    if (event->isAccepted() && view()) {
        showDropZone(view()->mapFromScene(event->scenePos()));
    }
}

void Containment::dragMoveEvent(QGraphicsSceneDragDropEvent *event)
{
    QGraphicsItem *item = scene()->itemAt(event->scenePos());
    event->setAccepted(item == this || !item);
    Plasma::Containment *c = containment();
    if (c && c->immutability() == Plasma::Mutable &&
        (event->mimeData()->hasFormat(static_cast<Plasma::Corona*>(scene())->appletMimeType()) ||
        KUrl::List::canDecode(event->mimeData())) && view()) {
            showDropZone(view()->mapFromScene(event->scenePos()));
     }
}

void Containment::dropEvent(QGraphicsSceneDragDropEvent *event)
{
    //kDebug() << event->mimeData()->text();
    if (!isContainment()) {
        Applet::dropEvent(event);
        return;
    }

    QString mimetype(static_cast<Corona*>(scene())->appletMimeType());

    if (event->mimeData()->hasFormat(mimetype) && scene()) {
        QString data = event->mimeData()->data(mimetype);
        QStringList appletNames = data.split('\n', QString::SkipEmptyParts);

        foreach (const QString &appletName, appletNames) {
            //kDebug() << "doing" << appletName;
            QRectF geom(mapFromScene(event->scenePos()), QSize(0, 0));
            addApplet(appletName, QVariantList(), geom);
        }
        event->acceptProposedAction();
    } else if (event->mimeData()->hasFormat(ExtenderItemMimeData::mimeType())) {
        kDebug() << "mimetype plasma/extenderitem is dropped, creating internal:extender";
        //Handle dropping extenderitems.
        const ExtenderItemMimeData *mimeData = qobject_cast<const ExtenderItemMimeData*>(event->mimeData());
        if (mimeData) {
            ExtenderItem *item = mimeData->extenderItem();
            QRectF geometry(event->pos(), item->size());
            kDebug() << "desired geometry: " << geometry;
            Applet *applet = addApplet("internal:extender", QVariantList(), geometry);
            item->setExtender(applet->extender());
        }
    } else if (KUrl::List::canDecode(event->mimeData())) {
        //TODO: collect the mimetypes of available script engines and offer
        //      to create widgets out of the matching URLs, if any
        KUrl::List urls = KUrl::List::fromMimeData(event->mimeData());
        foreach (const KUrl &url, urls) {
            KMimeType::Ptr mime = KMimeType::findByUrl(url);
            QString mimeName = mime->name();
            QRectF geom(event->pos(), QSize());
            QVariantList args;
            args << url.url();
            //             kDebug() << mimeName;
            KPluginInfo::List appletList = Applet::listAppletInfoForMimetype(mimeName);

            if (!appletList.isEmpty()) {
                //TODO: should we show a dialog here to choose which plasmoid load if
                //!appletList.isEmpty()
                QMenu choices;
                QHash<QAction *, QString> actionsToPlugins;
                foreach (const KPluginInfo &info, appletList) {
                    QAction *action;
                    if (!info.icon().isEmpty()) {
                        action = choices.addAction(KIcon(info.icon()), info.name());
                    } else {
                        action = choices.addAction(info.name());
                    }

                    actionsToPlugins.insert(action, info.pluginName());
                }

                actionsToPlugins.insert(choices.addAction(i18n("Icon")), "icon");
                QAction *choice = choices.exec(event->screenPos());
                if (choice) {
                    addApplet(actionsToPlugins[choice], args, geom);
                }
            } else if (url.protocol() != "data") {
                // We don't try to do anything with data: URIs
                // no special applet associated with this mimetype, let's
                addApplet("icon", args, geom);
            }
        }
        event->acceptProposedAction();
    } else {
        QStringList formats = event->mimeData()->formats();
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

        QString selectedPlugin;

        if (seenPlugins.isEmpty()) {
            // do nothing, we have no matches =/
        }

        if (seenPlugins.count() == 1) {
            selectedPlugin = seenPlugins.constBegin().key();
        } else {
            QMenu choices;
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

            QAction *choice = choices.exec(event->screenPos());
            if (choice) {
                selectedPlugin = actionsToPlugins[choice];
            }
        }

        if (!selectedPlugin.isEmpty()) {
            KTemporaryFile tempFile;
            if (tempFile.open()) {
                //TODO: what should we do with files after the applet is done with them??
                tempFile.setAutoRemove(false);

                {
                    QDataStream stream(&tempFile);
                    QByteArray data = event->mimeData()->data(pluginFormats[selectedPlugin]);
                    stream.writeRawData(data, data.size());
                }

                QRectF geom(event->pos(), QSize());
                QVariantList args;
                args << tempFile.fileName();
                kDebug() << args;
                tempFile.close();

                addApplet(selectedPlugin, args, geom);
            }
        }
    }
}

const QGraphicsItem *Containment::toolBoxItem() const
{
    return d->toolBox;
}

void Containment::resizeEvent(QGraphicsSceneResizeEvent *event)
{
    Applet::resizeEvent(event);

    if (!ContainmentPrivate::s_positioning) {
        switch (d->type) {
            case Containment::PanelContainment:
            case Containment::CustomPanelContainment:
                d->positionPanel();
                break;
            default:
                d->positionContainments();
                break;
        }
    }

    if (d->wallpaper) {
        d->wallpaper->setBoundingRect(QRectF(QPointF(0, 0), size()));
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
    if (d->wallpaper && d->wallpaper->isInitialized()) {
        QGraphicsItem *item = scene()->itemAt(event->scenePos());
        if (item == this) {
            event->ignore();
            d->wallpaper->wheelEvent(event);

            if (event->isAccepted()) {
                return;
            }

            event->accept();
        }
    }

    if (d->type == DesktopContainment) {
        QGraphicsItem *item = scene()->itemAt(event->scenePos());
        if (item == this) {
            int numDesktops = KWindowSystem::numberOfDesktops();
            int currentDesktop = KWindowSystem::currentDesktop();

            if (event->delta() < 0) {
                KWindowSystem::setCurrentDesktop(currentDesktop % numDesktops + 1);
            } else {
                KWindowSystem::setCurrentDesktop((numDesktops + currentDesktop - 2) % numDesktops + 1);
            }

            event->accept();
            return;
        }
    }

    event->ignore();
    Applet::wheelEvent(event);
}

bool Containment::sceneEventFilter(QGraphicsItem *watched, QEvent *event)
{
    Applet *applet = qgraphicsitem_cast<Applet*>(watched);

    // Otherwise we're watching something we shouldn't be...
    Q_ASSERT(applet != 0);
    if (!d->applets.contains(applet)) {
        return false;
    }

    //kDebug() << "got sceneEvent";
    switch (event->type()) {
    case QEvent::GraphicsSceneHoverEnter:
        //kDebug() << "got hoverenterEvent" << immutability() << " " << applet->immutability();
        if (immutability() == Mutable && applet->immutability() == Mutable) {
            QGraphicsSceneHoverEvent *he = static_cast<QGraphicsSceneHoverEvent*>(event);
            if (d->handles.contains(applet)) {
                AppletHandle *handle = d->handles.value(applet);
                if (handle) {
                    handle->setHoverPos(he->pos());
                }
            } else {
                //kDebug() << "generated applet handle";
                AppletHandle *handle = new AppletHandle(this, applet, he->pos());
                d->handles[applet] = handle;
                connect(handle, SIGNAL(disappearDone(AppletHandle*)),
                        this, SLOT(handleDisappeared(AppletHandle*)));
                connect(applet, SIGNAL(geometryChanged()),
                        handle, SLOT(appletResized()));
            }
        }
        break;
    case QEvent::GraphicsSceneHoverMove:
        if (immutability() == Mutable && applet->immutability() == Mutable) {
            QGraphicsSceneHoverEvent *he = static_cast<QGraphicsSceneHoverEvent*>(event);
            if (d->handles.contains(applet)) {
                AppletHandle *handle = d->handles.value(applet);
                if (handle) {
                    handle->setHoverPos(he->pos());
                }
            }
        }
        break;
    default:
        break;
    }

    return false;
}

QVariant Containment::itemChange(GraphicsItemChange change, const QVariant &value)
{
    //FIXME if the applet is moved to another containment we need to unfocus it

    if (isContainment() && !ContainmentPrivate::s_positioning &&
        (change == QGraphicsItem::ItemSceneHasChanged || change == QGraphicsItem::ItemPositionHasChanged)) {
        switch (d->type) {
            case PanelContainment:
            case CustomPanelContainment:
                d->positionPanel();
                break;
            default:
                d->positionContainments();
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
    if (!d->toolBox && (d->type == CustomPanelContainment || d->type >= CustomContainment)) {
        d->createToolBox();
    }

    if (d->toolBox) {
        d->toolBox->addTool(action);
    }
}

void Containment::removeToolBoxAction(QAction *action)
{
    if (d->toolBox) {
        d->toolBox->removeTool(action);
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

void Containment::openToolBox()
{
    if (d->toolBox) {
        d->toolBox->showToolBox();
    }
}

void Containment::closeToolBox()
{
    if (d->toolBox) {
        d->toolBox->hideToolBox();
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
        QString wallpaper = cfg.readEntry("wallpaperplugin", defaultWallpaper);
        QString mode = cfg.readEntry("wallpaperpluginmode", defaultWallpaperMode);
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
            d->wallpaper->setBoundingRect(QRectF(QPointF(0, 0), size()));
            d->wallpaper->setRenderingMode(mode);

            if (newPlugin) {
                connect(d->wallpaper, SIGNAL(update(const QRectF&)),
                        this, SLOT(updateRect(const QRectF&)));
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
            connect(d->wallpaper, SIGNAL(configureRequested()), this, SLOT(configureRequested()));
            connect(d->wallpaper, SIGNAL(configNeedsSaving()), this, SLOT(configNeedsSaving()));
        }

        emit configNeedsSaving();
    }
}

Plasma::Wallpaper *Containment::wallpaper() const
{
    return d->wallpaper;
}

void Containment::setActivity(const QString &activity)
{
    Context *context = d->context();
    if (context->currentActivity() != activity) {
        context->setCurrentActivity(activity);

        foreach (Applet *a, d->applets) {
            a->updateConstraints(ContextConstraint);
        }

        KConfigGroup c = config();
        c.writeEntry("activity", activity);
        emit configNeedsSaving();
    }
}

QString Containment::activity() const
{
    return d->context()->currentActivity();
}

Context *ContainmentPrivate::context()
{
    if (!con) {
        con = new Context(q);
        q->connect(con, SIGNAL(changed(Plasma::Context*)),
                   q, SIGNAL(contextChanged(Plasma::Context*)));
    }

    return con;
}

KActionCollection &ContainmentPrivate::actions()
{
    return static_cast<Applet*>(q)->d->actions;
}

void ContainmentPrivate::focusApplet(Plasma::Applet *applet)
{
    if (focusedApplet == applet) {
        return;
    }

    QList<QWidget *> widgets = actions().associatedWidgets();
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

void ContainmentPrivate::requestConfiguration()
{
    emit q->configureRequested(q);
}

void Containment::destroy(bool confirm)
{
    if (immutability() != Mutable) {
        return;
    }

    if (isContainment()) {
        //don't remove a desktop that's in use
        //FIXME: this should probably be based on whether any views care or not!
        //       sth like: foreach (view) { view->requires(this); }
        Q_ASSERT(corona());
        if (d->type != PanelContainment && d->type != CustomPanelContainment &&
            (d->screen != -1 || d->screen >= corona()->numScreens())) {
            kDebug() << (QObject*)this << "containment has a screen number?" << d->screen;
            return;
        }

        //FIXME maybe that %1 should be the containment type not the name
        if (!confirm ||
            KMessageBox::warningContinueCancel(
                view(),
                i18nc("%1 is the name of the containment", "Do you really want to remove this %1?", name()),
                i18nc("@title:window %1 is the name of the containment", "Remove %1", name()), KStandardGuiItem::remove()) == KMessageBox::Continue) {
            //clearApplets();
            Applet::destroy();
        }
    } else {
        Applet::destroy();
    }
}

void ContainmentPrivate::zoomIn()
{
    emit q->zoomRequested(q, Plasma::ZoomIn);
    positionToolBox();
}

void ContainmentPrivate::zoomOut()
{
    emit q->zoomRequested(q, Plasma::ZoomOut);
    positionToolBox();
}

ToolBox *ContainmentPrivate::createToolBox()
{
    if (!toolBox) {
        switch (type) {
        case Containment::PanelContainment:
        case Containment::CustomPanelContainment:
            toolBox = new PanelToolBox(q);
            toolBox->setSize(KIconLoader::SizeSmallMedium);
            toolBox->setIconSize(QSize(KIconLoader::SizeSmall, KIconLoader::SizeSmall));
            if (q->immutability() != Mutable) {
                toolBox->hide();
            }
            break;
        default:
            toolBox = new DesktopToolBox(q);
            toolBox->setSize(KIconLoader::SizeSmallMedium);
            toolBox->setIconSize(QSize(KIconLoader::SizeSmall, KIconLoader::SizeSmall));
            break;
        }

        if (toolBox) {
            QObject::connect(toolBox, SIGNAL(toggled()), q, SIGNAL(toolBoxToggled()));
            toolBox->load();
            positionToolBox();
        }
    }

    return toolBox;
}

void ContainmentPrivate::positionToolBox()
{
    if (toolBox) {
        toolBox->reposition();
    }
}

void ContainmentPrivate::triggerShowAddWidgets()
{
    emit q->showAddWidgetsInterface(QPointF());
}

void ContainmentPrivate::handleDisappeared(AppletHandle *handle)
{
    if (handles.contains(handle->applet())) {
        handles.remove(handle->applet());
        handle->detachApplet();
        handle->deleteLater();
    }
}

void ContainmentPrivate::checkRemoveAction()
{
    q->enableAction("remove", (q->immutability() == Mutable &&
                              (screen == -1 ||
                               type == Plasma::Containment::PanelContainment ||
                               type == Plasma::Containment::CustomPanelContainment)));
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
        bool unlocked = q->immutability() == Mutable;
        q->setAcceptDrops(unlocked);
        q->enableAction("add widgets", unlocked);

        // tell the applets too
        foreach (Applet *a, applets) {
            a->updateConstraints(ImmutableConstraint);
        }

        if (toolBox) {
            if (type == Containment::PanelContainment || type == Containment::CustomPanelContainment) {
                toolBox->setVisible(unlocked);
            } else {
                toolBox->setIsMovable(unlocked);
            }
        }

        //clear handles on lock
        if (!unlocked) {
            QMap<Applet*, AppletHandle*> h = handles;
            handles.clear();

            foreach (AppletHandle *handle, h) {
                handle->disconnect(q);
                handle->deleteLater();
            }
        }
    }

    if (constraints & Plasma::FormFactorConstraint) {
        foreach (Applet *applet, applets) {
            applet->updateConstraints(Plasma::FormFactorConstraint);
        }
    }

    if (toolBox && (constraints & Plasma::SizeConstraint ||
                    constraints & Plasma::FormFactorConstraint ||
                    constraints & Plasma::ScreenConstraint ||
                    constraints & Plasma::StartupCompletedConstraint)) {
        //kDebug() << "Positioning toolbox";
        positionToolBox();
    }

    if (toolBox &&
        constraints & Plasma::StartupCompletedConstraint &&
        type < Containment::CustomContainment) {
        toolBox->addTool(q->action("remove"));
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

    if (handles.contains(applet)) {
        AppletHandle *handle = handles.value(applet);
        handles.remove(applet);
        handle->deleteLater();
    }

    emit q->appletRemoved(applet);
    emit q->configNeedsSaving();
}

void ContainmentPrivate::containmentAppletAnimationComplete(QGraphicsItem *item, Plasma::Animator::Animation anim)
{
    if (anim == Animator::AppearAnimation &&
        item->parentItem() == q) {
        Applet *applet = qgraphicsitem_cast<Applet*>(item);

        if (applet) {
            if (type == Containment::DesktopContainment) {
                applet->installSceneEventFilter(q);
            }

            KConfigGroup *cg = applet->d->mainConfigGroup();
            applet->save(*cg);
            emit q->configNeedsSaving();
            //applet->setWindowFlags(Qt::Window);
        }
    }
}

bool containmentSortByPosition(const Containment *c1, const Containment *c2)
{
    return c1->id() < c2->id();
}

void ContainmentPrivate::positionContainments()
{
    Corona *c = q->corona();
    if (!c || ContainmentPrivate::s_positioning) {
        return;
    }

    ContainmentPrivate::s_positioning = true;

    //TODO: we should avoid running this too often; consider compressing requests
    //      with a timer.
    QList<Containment*> containments = c->containments();
    QMutableListIterator<Containment*> it(containments);

    while (it.hasNext()) {
        Containment *containment = it.next();
        if (containment->d->type == Containment::PanelContainment ||
            containment->d->type == Containment::CustomPanelContainment) {
            // weed out all containments we don't care about at all
            // e.g. Panels and ourself
            it.remove();
            continue;
        }
    }

    if (containments.isEmpty()) {
        ContainmentPrivate::s_positioning = false;
        return;
    }

    qSort(containments.begin(), containments.end(), containmentSortByPosition);
    it.toFront();

    int column = 0;
    int x = 0;
    int y = 0;
    int rowHeight = 0;
    //int count = 0;

    //kDebug() << "+++++++++++++++++++++++++++++++++++++++++++++++++++" << containments.count();
    while (it.hasNext()) {
        Containment *containment = it.next();
        containment->setPos(x, y);
        //kDebug() << ++count << "setting to" << x << y;

        int height = containment->size().height();
        if (height > rowHeight) {
            rowHeight = height;
        }

        ++column;

        if (column == CONTAINMENT_COLUMNS) {
            column = 0;
            x = 0;
            y += rowHeight + INTER_CONTAINMENT_MARGIN + TOOLBOX_MARGIN;
            rowHeight = 0;
        } else {
            x += containment->size().width() + INTER_CONTAINMENT_MARGIN;
        }
        //kDebug() << "column: " << column << "; x " << x << "; y" << y << "; width was"
        //         << containment->size().width();
    }
    //kDebug() << "+++++++++++++++++++++++++++++++++++++++++++++++++++";

    ContainmentPrivate::s_positioning = false;
}

void ContainmentPrivate::positionPanel(bool force)
{
    if (!q->scene()) {
        kDebug() << "no scene yet";
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

    //TODO: research how non-Horizontal, non-Vertical (e.g. Planar) panels behave here
    bool horiz = formFactor == Plasma::Horizontal;
    qreal bottom = horiz ? 0 : VERTICAL_STACKING_OFFSET;
    qreal lastHeight = 0;

    // this should be ok for small numbers of panels, but if we ever end
    // up managing hundreds of them, this simplistic alogrithm will
    // likely be too slow.
    foreach (const Containment *other, q->corona()->containments()) {
        if (other == q ||
            (other->d->type != Containment::PanelContainment &&
             other->d->type != Containment::CustomPanelContainment) ||
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

    kDebug() << "positioning" << (horiz ? "" : "non-") << "horizontal panel; forced?" << force;
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

    if (p != newPos) {
        ContainmentPrivate::s_positioning = true;
        q->setPos(newPos);
        ContainmentPrivate::s_positioning = false;
    }
}

} // Plasma namespace

#include "containment.moc"

