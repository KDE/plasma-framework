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
#include "containment_p.h"

#include <QAction>
#include <QDesktopWidget>
#include <QFile>
#include <QGraphicsSceneContextMenuEvent>
#include <QGraphicsView>
#include <QMimeData>
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QGraphicsLayout>
#include <QGraphicsLinearLayout>

#include <KApplication>
#include <KAuthorized>
#include <KIcon>
#include <KMenu>
#include <KMessageBox>
#include <KMimeType>
#include <KRun>
#include <KServiceTypeTrader>
#include <KStandardDirs>
#include <KWindowSystem>

#include "applet_p.h"
#include "applethandle_p.h"
#include "corona.h"
#include "animator.h"
#include "desktoptoolbox_p.h"
#include "paneltoolbox_p.h"
#include "svg.h"

namespace Plasma
{

Containment::StyleOption::StyleOption()
    : QStyleOptionGraphicsItem(),
      view(0)
{

}

Containment::StyleOption::StyleOption(const Containment::StyleOption & other)
    : QStyleOptionGraphicsItem(other),
      view(other.view)
{
}

Containment::StyleOption::StyleOption(const QStyleOptionGraphicsItem &other)
    : QStyleOptionGraphicsItem(other),
      view(0)
{
}

Containment::Containment(QGraphicsItem* parent,
                         const QString& serviceId,
                         uint containmentId)
    : Applet(parent, serviceId, containmentId),
      d(new Private(this))
{
    // WARNING: do not access config() OR globalConfig() in this method!
    //          that requires a scene, which is not available at this point
    setPos(0, 0);
    setBackgroundHints(NoBackground);
    setContainmentType(CustomContainment);
}

Containment::Containment(QObject* parent, const QVariantList& args)
    : Applet(parent, args),
      d(new Private(this))
{
    // WARNING: do not access config() OR globalConfig() in this method!
    //          that requires a scene, which is not available at this point
    setPos(0, 0);
    setBackgroundHints(NoBackground);
}

Containment::~Containment()
{
    delete d;
}

void Containment::init()
{
    setCacheMode(NoCache);
    setFlag(QGraphicsItem::ItemIsMovable, false);
    setFlag(QGraphicsItem::ItemClipsChildrenToShape, false);
    setAcceptDrops(true);
    setAcceptsHoverEvents(true);

    //TODO: would be nice to not do this on init, as it causes Animator to init
    connect(Animator::self(), SIGNAL(animationFinished(QGraphicsItem*,Plasma::Animator::Animation)),
            this, SLOT(appletAnimationComplete(QGraphicsItem*,Plasma::Animator::Animation)));

    if (d->type == NoContainmentType) {
        setContainmentType(DesktopContainment);
    }

    //common actions
    bool unlocked = immutability() == Mutable;

    QAction *appletBrowserAction = new QAction(i18n("Add Widgets..."), this);
    appletBrowserAction->setIcon(KIcon("list-add"));
    appletBrowserAction->setVisible(unlocked);
    appletBrowserAction->setEnabled(unlocked);
    connect(appletBrowserAction, SIGNAL(triggered()), this, SLOT(triggerShowAddWidgets()));
    appletBrowserAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    appletBrowserAction->setShortcut(QKeySequence("ctrl+a"));
    d->actions().addAction("add widgets", appletBrowserAction);

    QAction *action = new QAction(i18n("Next Applet"), this);
    //no icon
    connect(action, SIGNAL(triggered()), this, SLOT(focusNextApplet()));
    action->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    action->setShortcut(QKeySequence("ctrl+n"));
    d->actions().addAction("next applet", action);

    action = new QAction(i18n("Previous Applet"), this);
    //no icon
    connect(action, SIGNAL(triggered()), this, SLOT(focusPreviousApplet()));
    action->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    action->setShortcut(QKeySequence("ctrl+p"));
    d->actions().addAction("previous applet", action);

    if (immutability() != SystemImmutable) {
        //FIXME I'm not certain this belongs in Containment
        //but it sure is nice to have the keyboard shortcut in every containment by default
        QAction *lockDesktopAction = new QAction(unlocked ? i18n("Lock Widgets") : i18n("Unlock Widgets"), this);
        lockDesktopAction->setIcon(KIcon(unlocked ? "object-locked" : "object-unlocked"));
        connect(lockDesktopAction, SIGNAL(triggered(bool)), this, SLOT(toggleDesktopImmutability()));
        lockDesktopAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
        lockDesktopAction->setShortcut(QKeySequence("ctrl+l"));
        d->actions().addAction("lock widgets", lockDesktopAction);
    }

    if (d->type == DesktopContainment) { //this means custom containments are left out
        QAction *zoomAction = new QAction(i18n("Zoom In"), this);
        zoomAction->setIcon(KIcon("zoom-in"));
        connect(zoomAction, SIGNAL(triggered(bool)), this, SLOT(zoomIn()));
        zoomAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
        zoomAction->setShortcut(QKeySequence("ctrl+="));
        d->actions().addAction("zoom in", zoomAction);

        zoomAction = new QAction(i18n("Zoom Out"), this);
        zoomAction->setIcon(KIcon("zoom-out"));
        connect(zoomAction, SIGNAL(triggered(bool)), this, SLOT(zoomOut()));
        zoomAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
        zoomAction->setShortcut(QKeySequence("ctrl+-"));
        d->actions().addAction("zoom out", zoomAction);

        QAction *activityAction = new QAction(i18n("Add Activity"), this);
        activityAction->setIcon(KIcon("list-add"));
        activityAction->setVisible(unlocked);
        activityAction->setEnabled(unlocked);
        connect(activityAction, SIGNAL(triggered(bool)), this, SLOT(addSiblingContainment()));
        activityAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
        activityAction->setShortcut(QKeySequence("ctrl+shift+a"));
        d->actions().addAction("addSiblingContainment", activityAction);

        d->toolbox->addTool(this->action("add widgets"));
        d->toolbox->addTool(this->action("zoom in"));
        d->toolbox->addTool(this->action("zoom out"));
        if (immutability() != SystemImmutable) {
            d->toolbox->addTool(this->action("lock widgets"));
        }
        d->toolbox->addTool(this->action("addSiblingContainment"));
    }

    Applet::init();
}

// helper function for sorting the list of applets
bool appletConfigLessThan(const KConfigGroup &c1, const KConfigGroup &c2)
{
    QPointF p1 = c1.readEntry("geometry", QRectF()).topLeft();
    QPointF p2 = c2.readEntry("geometry", QRectF()).topLeft();
    if (p1.x() != p2.x()) {
        return p1.x() < p2.x();
    }
    return p1.y() < p2.y();
}

void Containment::restore(KConfigGroup &group)
{
    /*kDebug() << "!!!!!!!!!!!!initConstraints" << group.name() << containmentType();
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
    setScreen(group.readEntry("screen", d->screen));

    flushPendingConstraintsEvents();
    //kDebug() << "Containment" << id() << "geometry is" << geometry() << "config'd with" << appletConfig.name();
    restoreContents(group);
    setImmutability((ImmutabilityType)group.readEntry("immutability", (int)Mutable));
}

void Containment::save(KConfigGroup &group) const
{
    // locking is saved in Applet::save
    Applet::save(group);
    group.writeEntry("screen", d->screen);
    group.writeEntry("formfactor", (int)d->formFactor);
    group.writeEntry("location", (int)d->location);
    saveContents(group);
}

void Containment::saveContents(KConfigGroup &group) const
{
    KConfigGroup applets(&group, "Applets");
    foreach (const Applet* applet, d->applets) {
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
    qSort(appletConfigs.begin(), appletConfigs.end(), appletConfigLessThan);

    foreach (KConfigGroup appletConfig, appletConfigs) {
        int appId = appletConfig.name().toUInt();
        //kDebug() << "the name is" << appletConfig.name();
        QString plugin = appletConfig.readEntry("plugin", QString());

        if (plugin.isEmpty()) {
            continue;
        }

        Applet *applet = d->addApplet(plugin, QVariantList(), appletConfig.readEntry("geometry", QRectF()), appId, true);
        applet->restore(appletConfig);
    }
}

Containment::Type Containment::containmentType() const
{
    return d->type;
}

void Containment::setContainmentType(Containment::Type type)
{
    d->type = type;

    if (isContainment() && type == DesktopContainment) {
        if (!d->toolbox) {
            d->createToolbox();
        }
    } else if (isContainment() && type == PanelContainment) {
        if (!d->toolbox) {
            d->createToolbox();
            d->toolbox->setSize(22);
            d->toolbox->setIconSize(QSize(16, 16));
        }
    } else {
        delete d->toolbox;
        d->toolbox = 0;
    }
}

Corona* Containment::corona() const
{
    return dynamic_cast<Corona*>(scene());
}

void Containment::contextMenuEvent(QGraphicsSceneContextMenuEvent* event)
{
    //kDebug() << "let's see if we manage to get a context menu here, huh";
    if (!isContainment() || !scene() || !KAuthorized::authorizeKAction("desktop_contextmenu")) {
        Applet::contextMenuEvent(event);
        return;
    }

    QPointF point = event->scenePos();
    QGraphicsItem* item = scene()->itemAt(point);
    if (item == this) {
        item = 0;
    }

    Applet* applet = 0;

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
        bool hasEntries = false;

        QList<QAction*> actions = applet->contextualActions();
        if (!actions.isEmpty()) {
            foreach(QAction* action, actions) {
                desktopMenu.addAction(action);
            }
            hasEntries = true;
        }

        if (applet->hasConfigurationInterface()) {
            QAction* configureApplet = applet->d->actions.action("configure");
            if (configureApplet) {
                desktopMenu.addAction(configureApplet);
                hasEntries = true;
            }
        }

        QList<QAction*> containmentActions = contextualActions();
        if (!containmentActions.isEmpty()) {
            if (hasEntries) {
                desktopMenu.addSeparator();
            }
            hasEntries = true;
            QMenu *containmentActionMenu = &desktopMenu;

            if (!actions.isEmpty() && containmentActions.count() > 2) {
                containmentActionMenu = new KMenu(i18n("%1 Options", name()), &desktopMenu);
                desktopMenu.addMenu(containmentActionMenu);
            }

            foreach(QAction* action, containmentActions) {
                containmentActionMenu->addAction(action);
            }
        }

        if (scene() && static_cast<Corona*>(scene())->immutability() == Mutable) {
            if (hasEntries) {
                desktopMenu.addSeparator();
            }

            QAction* closeApplet = applet->d->actions.action("remove");
            if (! closeApplet) { //unlikely but not impossible
                kDebug() << "wtf!! no remove action!!!!!!!!";
                closeApplet = new QAction(i18n("Remove this %1", applet->name()), &desktopMenu);
                closeApplet->setIcon(KIcon("edit-delete"));
                connect(closeApplet, SIGNAL(triggered(bool)), applet, SLOT(destroy()));
            }
            desktopMenu.addAction(closeApplet);
            hasEntries = true;
        }

        if (!hasEntries) {
            Applet::contextMenuEvent(event);
            kDebug() << "no entries";
            return;
        }
    } else {
        if (!scene() || (static_cast<Corona*>(scene())->immutability() != Mutable && !KAuthorized::authorizeKAction("unlock_desktop"))) {
            //kDebug() << "immutability";
            Applet::contextMenuEvent(event);
            return;
        }

        QList<QAction*> actions = contextualActions();

        if (actions.count() < 1) {
            //kDebug() << "no applet, but no actions";
            Applet::contextMenuEvent(event);
            return;
        }

        foreach(QAction* action, actions) {
            desktopMenu.addAction(action);
        }
    }

    event->accept();
    //kDebug() << "executing at" << event->screenPos();
    desktopMenu.exec(event->screenPos());
}

void Containment::setFormFactor(FormFactor formFactor)
{
    if (d->formFactor == formFactor && layout()) {
        return;
    }

    //kDebug() << "switching FF to " << formFactor;
    FormFactor was = d->formFactor;

    d->formFactor = formFactor;

    if (isContainment() && containmentType() == PanelContainment && was != formFactor) {
        // we are a panel and we have chaged our orientation
        d->positionPanel(true);
    }

    updateConstraints(Plasma::FormFactorConstraint);
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

    foreach (Applet* applet, d->applets) {
        applet->updateConstraints(Plasma::LocationConstraint);
    }

    if (emitGeomChange) {
        // our geometry on the scene will not actually change,
        // but for the purposes of views it has
        emit geometryChanged();
    }

    updateConstraints(Plasma::LocationConstraint);
}

void Containment::addSiblingContainment()
{
    emit addSiblingContainment(this);
}

void Containment::clearApplets()
{
    qDeleteAll(d->applets);
    d->applets.clear();
}

Applet* Containment::addApplet(const QString& name, const QVariantList& args, const QRectF &appletGeometry)
{
    return d->addApplet(name, args, appletGeometry);
}

//pos must be relative to the containment already. use mapfromscene.
//what we're trying to do here for panels is make the applet go to the requested position,
//or somewhere close to it, and get integrated properly into the containment as if it were created
//there.
void Containment::addApplet(Applet *applet, const QPointF &pos, bool delayInit)
{
    if (!delayInit && immutability() != Mutable) {
        return;
    }

    if (!applet) {
        kDebug() << "adding null applet!?!";
        return;
    }

    Containment *currentContainment = applet->containment();

    if (containmentType() == PanelContainment) {
        //panels don't want backgrounds, which is important when setting geometry
        setBackgroundHints(NoBackground);
    }

    if (currentContainment && currentContainment != this) {
        applet->removeSceneEventFilter(currentContainment);
        KConfigGroup oldConfig = applet->config();
        currentContainment->d->applets.removeAll(applet);
        if (currentContainment->d->handles.contains(applet)) {
            currentContainment->d->handles.remove(applet);
        }
        applet->setParentItem(this);

        // now move the old config to the new location
        KConfigGroup c = config().group("Applets").group(QString::number(applet->id()));
        oldConfig.reparent(&c);
        applet->d->resetConfigurationObject();
    } else {
        applet->setParentItem(this);
    }

    d->applets << applet;

    connect(applet, SIGNAL(destroyed(QObject*)), this, SLOT(appletDestroyed(QObject*)));

    if (pos != QPointF(-1, -1)) {
        applet->setPos(pos);
    }

    if (delayInit) {
        if (containmentType() == DesktopContainment) {
            applet->installSceneEventFilter(this);
        }
    } else {
        applet->init();
        Animator::self()->animateItem(applet, Animator::AppearAnimation);
    }

    applet->updateConstraints(Plasma::AllConstraints | Plasma::StartupCompletedConstraint);
    if (!delayInit) {
        applet->flushPendingConstraintsEvents();
        emit configNeedsSaving();
    }

    emit appletAdded(applet, pos);
}

Applet::List Containment::applets() const
{
    return d->applets;
}

void Containment::setScreen(int screen)
{
    // screen of -1 means no associated screen.
    if (containmentType() == DesktopContainment || containmentType() >= CustomContainment) {
#ifndef Q_OS_WIN
        // we want to listen to changes in work area if our screen changes
        if (d->screen < 0 && screen > -1) {
            connect(KWindowSystem::self(), SIGNAL(workAreaChanged()), this, SLOT(positionToolbox()));
        } else if (screen < 0) {
            disconnect(KWindowSystem::self(), SIGNAL(workAreaChanged()), this, SLOT(positionToolbox()));
        }
#endif
        if (screen > -1 && corona()) {
            // sanity check to make sure someone else doesn't have this screen already!
            Containment* currently = corona()->containmentForScreen(screen);
            if (currently && currently != this) {
                //kDebug() << "currently is on screen" << currently->screen() << "and is" << currently->name() << (QObject*)currently << (QObject*)this;
                currently->setScreen(-1);
            }
        }
    }

    //kDebug() << "setting screen to" << screen << "and we are a" << containmentType();
    QDesktopWidget *desktop = QApplication::desktop();
    int numScreens = desktop->numScreens();
    if (screen < -1) {
        screen = -1;
    }

    //kDebug() << "setting screen to " << screen << "and type is" << containmentType();
    if (screen < numScreens && screen > -1) {
        if (containmentType() == DesktopContainment ||
            containmentType() >= CustomContainment) {
            resize(desktop->screenGeometry(screen).size());
        }
    }

    int oldScreen = d->screen;
    d->screen = screen;
    updateConstraints(Plasma::ScreenConstraint);
    if (oldScreen != screen) {
        emit screenChanged(oldScreen, screen, this);
    }
}

int Containment::screen() const
{
    return d->screen;
}

QPoint Containment::effectiveScreenPos() const
{
    if (d->screen < 0) {
        return QPoint();
    }

    QRect r = QApplication::desktop()->screenGeometry(d->screen);
    if (containmentType() == PanelContainment) {
        QRectF p = geometry();

        switch (d->location) {
            case TopEdge:
                return QPoint(r.left() + p.x(), r.top());
                break;
            case BottomEdge:
                return QPoint(r.left() + p.x(), r.bottom() - p.height());
                break;
            case LeftEdge:
                return QPoint(r.left(), r.top() + (p.bottom() + INTER_CONTAINMENT_MARGIN));
                break;
            case RightEdge:
                return QPoint(r.right() - p.width(), r.top() + (p.bottom() + INTER_CONTAINMENT_MARGIN));
                break;
            default:
                //FIXME: implement properly for Floating!
                return p.topLeft().toPoint();
                break;
        }
    } else {
        //NOTE: if we ever support non-origin'd desktop containments
        //      this assumption here will have to change
        return r.topLeft();
    }
}

KPluginInfo::List Containment::listContainments(const QString &category,
                                                const QString &parentApp)
{
    QString constraint;

    if (parentApp.isEmpty()) {
        constraint.append("not exist [X-KDE-ParentApp]");
    } else {
        constraint.append("[X-KDE-ParentApp] == '").append(parentApp).append("'");
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

void Containment::dropEvent(QGraphicsSceneDragDropEvent *event)
{
    //kDebug() << event->mimeData()->text();

    QString mimetype(static_cast<Corona*>(scene())->appletMimeType());

    if (event->mimeData()->hasFormat(mimetype) && scene()) {
        QString plasmoidName;
        plasmoidName = event->mimeData()->data(mimetype);
        QRectF geom(mapFromScene(event->scenePos()), QSize(0, 0));
        addApplet(plasmoidName, QVariantList(), geom);
        event->acceptProposedAction();
    } else if (KUrl::List::canDecode(event->mimeData())) {
        //TODO: collect the mimetypes of available script engines and offer
        //      to create widgets out of the matching URLs, if any
        KUrl::List urls = KUrl::List::fromMimeData(event->mimeData());
        foreach (const KUrl& url, urls) {
            KMimeType::Ptr mime = KMimeType::findByUrl(url);
            QString mimeName = mime->name();
            QRectF geom(event->scenePos(), QSize(0, 0));
            QVariantList args;
            args << url.url();
            //             kDebug() << mimeName;
            KPluginInfo::List appletList = Applet::listAppletInfoForMimetype(mimeName);

            if (appletList.isEmpty()) {
                // no special applet associated with this mimetype, let's
                addApplet("icon", args, geom);
            } else {
                //TODO: should we show a dialog here to choose which plasmoid load if
                //!appletList.isEmpty()
                QMenu choices;
                QHash<QAction*, QString> actionsToPlugins;
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
            }
        }
        event->acceptProposedAction();
    }
}

void Containment::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    //FIXME Qt4.4 check to see if this is still necessary to avoid unnecessary repaints
    //            check with QT_FLUSH_PAINT=1 and mouse through applets that accept hover,
    //            applets that don't and system windows
    if (event->spontaneous()) {
        Applet::hoverEnterEvent(event);
    }
    Q_UNUSED(event)
}

void Containment::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    //FIXME Qt4.4 check to see if this is still necessary to avoid unnecessary repaints
    //            check with QT_FLUSH_PAINT=1 and mouse through applets that accept hover,
    //            applets that don't and system windows
//    Applet::hoverLeaveEvent(event);
    Q_UNUSED(event)
}

void Containment::keyPressEvent(QKeyEvent *event)
{
    kDebug() << "keyPressEvent with" << event->key() << "and hoping and wishing for a" << Qt::Key_Tab;
    if (event->key() == Qt::Key_Tab) { // && event->modifiers() == 0) {
        kDebug() << "let's give focus to...." << (QObject*)d->applets.first();
        if (!d->applets.isEmpty()) {
            d->applets.first()->setFocus(Qt::TabFocusReason);
        }
    }
}

bool Containment::sceneEventFilter(QGraphicsItem *watched, QEvent *event)
{
    Applet *applet = qgraphicsitem_cast<Applet*>(watched);

    // Otherwise we're watching something we shouldn't be...
    //kDebug() << "got sceneEvent";
    Q_ASSERT(applet!=0);
    if (!d->applets.contains(applet)) {
        return false;
    }

    switch (event->type()) {
    case QEvent::GraphicsSceneHoverEnter:
        //kDebug() << "got hoverenterEvent" << immutability() << " " << applet->immutability();
        if (immutability() == Mutable && applet->immutability() == Mutable) {
            if (d->handles.contains(applet)) {
                d->handles[applet]->startFading(AppletHandle::FadeIn);
            } else {
                //kDebug() << "generated applet handle";
                //TODO: there should be a small delay on showing these. they pop up too quickly/easily
                //      right now
                AppletHandle *handle = new AppletHandle(this, applet);
                d->handles[applet] = handle;
                connect(handle, SIGNAL(disappearDone(AppletHandle*)),
                        this, SLOT(handleDisappeared(AppletHandle*)));
                connect(applet, SIGNAL(geometryChanged()),
                        handle, SLOT(appletResized()));
            }
        }
        break;
    case QEvent::GraphicsSceneHoverLeave:
        //kDebug() << "got hoverLeaveEvent";
        if (d->handles.contains(applet)) {
            QGraphicsSceneHoverEvent *he = static_cast<QGraphicsSceneHoverEvent *>(event);
            if (!d->handles[applet]->boundingRect().contains(d->handles[applet]->mapFromScene(he->scenePos()))) {
                d->handles[applet]->startFading(AppletHandle::FadeOut);
            }
        }
    default:
        break;
    }

    return false;
}

QVariant Containment::itemChange(GraphicsItemChange change, const QVariant &value)
{
    Q_UNUSED(value)
    //FIXME if the applet is moved to another containment we need to unfocus it

    if (isContainment() &&
        (change == QGraphicsItem::ItemSceneHasChanged || change == QGraphicsItem::ItemPositionHasChanged) &&
        !d->positioning) {
        switch (containmentType()) {
            case PanelContainment:
                d->positionPanel();
                break;
            default:
                d->positionContainment();
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

void Containment::addToolboxTool(QAction *action)
{
    if (d->toolbox) {
        d->toolbox->addTool(action);
    }
}

void Containment::removeToolboxTool(QAction *action)
{
    if (d->toolbox) {
        d->toolbox->removeTool(action);
    }
}

void Containment::setToolboxOpen(bool open)
{
    if (open) {
        openToolbox();
    } else {
        closeToolbox();
    }
}

void Containment::openToolbox()
{
    if (d->toolbox) {
        d->toolbox->showToolbox();
    }
}

void Containment::closeToolbox()
{
    if (d->toolbox) {
        d->toolbox->hideToolbox();
    }
}

void Containment::addAssociatedWidget(QWidget *widget)
{
    Applet::addAssociatedWidget(widget);
    if (d->focusedApplet) {
        d->focusedApplet->addAssociatedWidget(widget);
    }
}

void Containment::removeAssociatedWidget(QWidget *widget)
{
    Applet::removeAssociatedWidget(widget);
    if (d->focusedApplet) {
        d->focusedApplet->removeAssociatedWidget(widget);
    }
}

KActionCollection& Containment::Private::actions()
{
    return static_cast<Applet*>(q)->d->actions;
}

void Containment::Private::focusApplet(Plasma::Applet *applet)
{
    if (focusedApplet == applet) {
        return;
    }

    if (q->screen() == -1) {
        //what's the point of having focus if you're not on screen? :)
        //also, clicking a containment to switch to it makes the ZUI waay nicer.
        emit q->focusRequested(q);
    }

    QList<QWidget *> widgets = actions().associatedWidgets();
    if (focusedApplet) {
        foreach (QWidget *w, widgets) {
            focusedApplet->removeAssociatedWidget(w);
        }
    }
    //but what if applet isn't really one of our applets?
    //FIXME should we really unfocus the old applet?
    if (applets.contains(applet)) {
        kDebug() << "switching to" << applet->name();
        focusedApplet = applet;
        if (focusedApplet) {
            foreach (QWidget *w, widgets) {
                focusedApplet->addAssociatedWidget(w);
            }
        }
        applet->setFocus(Qt::ShortcutFocusReason);
    } else {
        focusedApplet = 0;
    }
}

void Containment::focusNextApplet()
{
    kDebug();
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
    kDebug();
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
    if (immutability() != Mutable) {
        return;
    }
    if (isContainment()) {
        //FIXME maybe that %1 should be the containment type not the name
        if (KMessageBox::warningContinueCancel(0, i18n("Do you really want to remove this %1?", name()),
                    i18n("Remove %1", name()), KStandardGuiItem::remove()) == KMessageBox::Continue ) {
            clearApplets();
            corona()->destroyContainment(this);
        }
    } else {
        Applet::destroy();
    }
}


// Private class implementation

void Containment::Private::toggleDesktopImmutability()
{
    if (q->corona()) {
        if (q->corona()->immutability() == Mutable) { 
            q->corona()->setImmutability(UserImmutable);
        } else if (q->corona()->immutability() == UserImmutable) { 
            q->corona()->setImmutability(Mutable);
        }
    } else {
        if (q->immutability() == Mutable) {
            q->setImmutability(UserImmutable);
        } else if (q->immutability() == UserImmutable) { 
            q->setImmutability(Mutable);
        }
    }

    //setLockToolText();
}

void Containment::Private::zoomIn()
{
    emit q->zoomRequested(q, Plasma::ZoomIn);
}

void Containment::Private::zoomOut()
{
    emit q->zoomRequested(q, Plasma::ZoomOut);
}

Toolbox* Containment::Private::createToolbox()
{
    if (!toolbox) {
        switch (type) {
        case PanelContainment:
            toolbox = new PanelToolbox(q);
            connect(toolbox, SIGNAL(toggled()), q, SIGNAL(toolboxToggled()));
            break;
        //defaults to DesktopContainment right now
        default:
            toolbox = new DesktopToolbox(q);
            connect(toolbox, SIGNAL(toggled()), toolbox, SLOT(toggle()));
            break;
        }
        positionToolbox();

    }

    return toolbox;
}

void Containment::Private::positionToolbox()
{
    if (!toolbox) {
        return;
    }

    //The placement assumes that the geometry width/height is no more than the screen
    if (type == PanelContainment) {
        if (q->formFactor() == Vertical) {
            toolbox->setOrientation(Qt::Vertical);
            toolbox->setPos(q->geometry().width()/2 - toolbox->boundingRect().width()/2, q->geometry().height());
        //defaulting to Horizontal right now
        } else {
            toolbox->setOrientation(Qt::Horizontal);
            if (QApplication::layoutDirection() == Qt::RightToLeft) {
                toolbox->setPos(q->geometry().left(), q->geometry().height()/2 - toolbox->boundingRect().height()/2);
            } else {
                toolbox->setPos(q->geometry().width(), q->geometry().height()/2 - toolbox->boundingRect().height()/2);
            }
        }
    } else {
        QRectF r;
        QDesktopWidget *desktop = QApplication::desktop();
        r = desktop->availableGeometry(screen);
        if (q->view() && !q->view()->transform().isScaling()) {
            toolbox->setPos(r.topRight());
        } else {
            toolbox->setPos(q->mapFromScene(QPointF(q->geometry().topRight())));
        }
    }
    
}

void Containment::Private::triggerShowAddWidgets()
{
    emit q->showAddWidgetsInterface(QPointF());
}

void Containment::Private::handleDisappeared(AppletHandle *handle)
{
    handles.remove(handle->applet());
    handle->deleteLater();
}

void Containment::Private::containmentConstraintsEvent(Plasma::Constraints constraints)
{
    if (!q->isContainment()) {
        return;
    }

    //kDebug() << "got containmentConstraintsEvent" << constraints << (QObject*)toolbox;
    if (constraints & Plasma::ImmutableConstraint) {
        //update actions
        bool unlocked = q->immutability() == Mutable;
        QAction *action = actions().action("add widgets");
        if (action) {
            action->setVisible(unlocked);
            action->setEnabled(unlocked);
        }
        //FIXME immutability changes conflict with zoom changes
        /*action = actions().action("addSiblingContainment");
        if (action) {
            action->setVisible(unlocked);
            action->setEnabled(unlocked);
        }*/
        action = actions().action("lock widgets");
        if (action) {
            action->setText(unlocked ? i18n("Lock Widgets") : i18n("Unlock Widgets"));
            action->setIcon(KIcon(unlocked ? "object-locked" : "object-unlocked"));
        }

        // tell the applets too
        foreach (Applet *a, applets) {
            a->updateConstraints(ImmutableConstraint);
        }

        if (q->isContainment() && type == PanelContainment) {
            if (unlocked) {
                toolbox->show();
            } else {
                toolbox->hide();
            }
        }
    }

    if ((constraints & Plasma::SizeConstraint || constraints & Plasma::ScreenConstraint) &&
         toolbox) {
        positionToolbox();
    }

    if (constraints & Plasma::FormFactorConstraint) {
        if (toolbox) {
            if (q->formFactor() == Vertical) {
                toolbox->setOrientation(Qt::Vertical);
                //defaults to horizontal
            } else {
                toolbox->setOrientation(Qt::Horizontal);
            }
        }

        foreach (Applet *applet, applets) {
            applet->updateConstraints(Plasma::FormFactorConstraint);
        }
    }

    if (constraints & Plasma::SizeConstraint) {
        switch (q->containmentType()) {
            case PanelContainment:
                positionPanel();
                break;
            default:
                positionContainment();
                break;
        }
    }
}

Applet* Containment::Private::addApplet(const QString& name, const QVariantList& args,
                                        const QRectF& appletGeometry, uint id, bool delayInit)
{
    if (!delayInit && q->immutability() != Mutable) {
        kDebug() << "addApplet for" << name << "requested, but we're currently immutable!";
        return 0;
    }

    QGraphicsView *v = q->view();
    if (v) {
        v->setCursor(Qt::BusyCursor);
    }

    Applet* applet = Applet::load(name, id, args);
    if (v) {
        v->unsetCursor();
    }

    if (!applet) {
        kDebug() << "Applet" << name << "could not be loaded.";
        applet = new Applet;
    }

    //kDebug() << applet->name() << "sizehint:" << applet->sizeHint() << "geometry:" << applet->geometry();

    connect(applet, SIGNAL(configNeedsSaving()), q, SIGNAL(configNeedsSaving()));
    connect(applet, SIGNAL(releaseVisualFocus()), q, SIGNAL(releaseVisualFocus()));
    q->addApplet(applet, appletGeometry.topLeft(), delayInit);
    return applet;
}

bool Containment::Private::regionIsEmpty(const QRectF &region, Applet *ignoredApplet) const
{
    foreach (Applet *applet, applets) {
        if (applet != ignoredApplet && applet->geometry().intersects(region)) {
            return false;
        }
    }
    return true;
}

void Containment::Private::appletDestroyed(QObject* object)
{
    // we do a static_cast here since it really isn't an Applet by this
    // point anymore since we are in the qobject dtor. we don't actually
    // try and do anything with it, we just need the value of the pointer
    // so this unsafe looking code is actually just fine.
    Applet* applet = static_cast<Plasma::Applet*>(object);
    applets.removeAll(applet);
    if (focusedApplet == applet) {
        focusedApplet = 0;
    }
    emit q->appletRemoved(applet);
    emit q->configNeedsSaving();
}

void Containment::Private::appletAnimationComplete(QGraphicsItem *item, Plasma::Animator::Animation anim)
{
    if (anim == Animator::AppearAnimation) {
        if (q->containmentType() == DesktopContainment &&
            item->parentItem() == q &&
            qgraphicsitem_cast<Applet*>(item)) {
                item->installSceneEventFilter(q);
        }
    }
}

void Containment::Private::positionContainment()
{
    Corona *c = q->corona();
    if (!c) {
        return;
    }

    QList<Containment*> containments = c->containments();
    QMutableListIterator<Containment*> it(containments);

    while (it.hasNext()) {
        Containment *containment = it.next();
        if (containment == q ||
            containment->containmentType() == PanelContainment) {
            // weed out all containments we don't care about at all
            // e.g. Panels and ourself
            it.remove();
            continue;
        }

        if (q->collidesWithItem(containment)) {
            break;
        }
    }

    if (!it.hasNext()) {
        // we made it all the way through the list, we have no
        // collisions
        return;
    }

    // we need to find how many screens are to our top and left
    // to calculate the proper offsets for the margins.
    int width = 0;
    int height = 0;

    QDesktopWidget *desktop = QApplication::desktop();
    int numScreens = desktop->numScreens();

    for (int i = 0; i < numScreens; ++i) {
        QRect otherScreen = desktop->screenGeometry(i);

        if (width < otherScreen.width()) {
            width = otherScreen.width();
        }

        if (height < otherScreen.height()) {
            height = otherScreen.height();
        }
    }

    //this magic number (4) is the number of columns to try before going to the next row
    width = (width + INTER_CONTAINMENT_MARGIN) * 4;
    height += INTER_CONTAINMENT_MARGIN;

    // a mildly naive "find the first slot" approach
    QRectF r = q->boundingRect();
    QPointF topLeft(0, 0);
    q->setPos(topLeft);

    positioning = true;
    while (true) {
        it.toFront();

        while (it.hasNext()) {
            Containment *containment = it.next();
            if (q->collidesWithItem(containment)) {
                break;
            }

            QPointF pos = containment->pos();
            if (pos.x() <= topLeft.x() && pos.y() <= topLeft.y()) {
                // we don't colid with this containment, and it's above
                // and to the left of us, so let's not bother checking
                // it again if we go through this loop again
                it.remove();
            }
        }

        if (!it.hasNext()) {
            // success! no collisions!
            break;
        }

        if (topLeft.x() + (r.width() * 2) + INTER_CONTAINMENT_MARGIN > width) {
            // we ran out of width room, try another row
            topLeft = QPoint(0, topLeft.y() + height);
        } else {
            topLeft.setX(topLeft.x() + r.width() + INTER_CONTAINMENT_MARGIN);
        }

        kDebug() << "trying at" << topLeft;
        q->setPos(topLeft);
        //kDebug() << collidingItems().count() << collidingItems()[0] << (QGraphicsItem*)this;
    }

    positioning = false;
}

void Containment::Private::positionPanel(bool force)
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
    bool horiz = q->formFactor() == Plasma::Horizontal;
    qreal bottom = horiz ? 0 : VERTICAL_STACKING_OFFSET;
    qreal lastHeight = 0;

    // this should be ok for small numbers of panels, but we ever end
    // up managing hundreds of them, this simplistic alogrithm will
    // likely be too slow.
    foreach (const Containment* other, q->corona()->containments()) {
        if (other == q ||
            other->containmentType() != PanelContainment ||
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

    positioning = true;
    if (p != newPos) {
        q->setPos(newPos);
        emit q->geometryChanged();
    }
    positioning = false;
}


} // Plasma namespace

#include "containment.moc"

