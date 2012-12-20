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

#include "private/containment_p.h"

#include <QApplication>
#include <QClipboard>
#include <QMimeData>
#include <QMimeDatabase>
#include <QDropEvent>
#include <qtemporaryfile.h>

#include <kaction.h>
#include <kactioncollection.h>
#include <kcoreauthorized.h>
#include <klocale.h>
#include <kurlmimedata.h>
#include <kwindowsystem.h>

#include "config-plasma.h"

#if !PLASMA_NO_KIO
#include "kio/jobclasses.h" // for KIO::JobFlags
#include "kio/job.h"
#include "kio/scheduler.h"
#endif

#include "containmentactions.h"
#include "containmentactionspluginsconfig.h"
#include "corona.h"
#include "pluginloader.h"
#include "svg.h"

#include "remote/accessappletjob.h"
#include "remote/accessmanager.h"

#include "private/applet_p.h"
#include "private/containmentactionspluginsconfig_p.h"


namespace Plasma
{

bool ContainmentPrivate::s_positioningPanels = false;
QHash<QString, ContainmentActions*> ContainmentPrivate::globalActionPlugins;

const char ContainmentPrivate::defaultWallpaper[] = "image";
const char ContainmentPrivate::defaultWallpaperMode[] = "SingleImage";

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
    appletBrowserAction->setIcon(KDE::icon("list-add"));
    appletBrowserAction->setShortcut(KShortcut("alt+d, a"));
    appletBrowserAction->setData(Containment::AddTool);

    KAction *action = actions->addAction("next applet");
    action->setText(i18n("Next Widget"));
    //no icon
    action->setShortcut(KShortcut("alt+d, n"));
    action->setData(Containment::ControlTool);

    action = actions->addAction("previous applet");
    action->setText(i18n("Previous Widget"));
    //no icon
    action->setShortcut(KShortcut("alt+d, p"));
    action->setData(Containment::ControlTool);
}

void ContainmentPrivate::initApplets()
{
    foreach (Applet *applet, applets) {
        applet->restore(*applet->d->mainConfigGroup());
        applet->init();
#ifndef NDEBUG
        kDebug() << "!!{} STARTUP TIME" << QTime().msecsTo(QTime::currentTime()) << "Applet" << applet->name();
#endif
    }

    q->flushPendingConstraintsEvents();

    foreach (Applet *applet, applets) {
        applet->flushPendingConstraintsEvents();
    }

#ifndef NDEBUG
    kDebug() << "!!{} STARTUP TIME" << QTime().msecsTo(QTime::currentTime()) << "Containment's applets initialized" << q->name();
#endif
}

void ContainmentPrivate::checkContainmentFurniture()
{
    if (q->isContainment() &&
        (type == Containment::DesktopContainment || type == Containment::PanelContainment)) {
    }
}

void ContainmentPrivate::addContainmentActions(KMenu &desktopMenu, QEvent *event)
{
    if (q->corona()->immutability() != Mutable &&
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
    //TODO: port away qgv
    /* should decide in a different way if this is a dashboard containment
    if (corona->offscreenWidgets().contains(q)) {
        return;
    }*/

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
        if (newScreen > -1) {
            // sanity check to make sure someone else doesn't have this screen already!
            Containment *currently = corona->containmentForScreen(newScreen, newDesktop);
            if (currently && currently != q) {
#ifndef NDEBUG
                kDebug() << "currently is on screen" << currently->screen()
                         << "desktop" << currently->desktop()
                         << "and is" << currently->activity()
                         << (QObject*)currently << "i'm" << (QObject*)q;
#endif
                currently->setScreen(-1, currently->desktop());
                swapScreensWith = currently;
            }
        }
    }

    int oldDesktop = desktop;
    desktop = newDesktop;

    int oldScreen = screen;
    screen = newScreen;

    q->updateConstraints(Plasma::ScreenConstraint);

    if (oldScreen != newScreen || oldDesktop != newDesktop) {
        /*
#ifndef NDEBUG
        kDebug() << "going to signal change for" << q
#endif
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


KActionCollection* ContainmentPrivate::actions()
{
    return static_cast<Applet*>(q)->d->actions;
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
#ifndef NDEBUG
        kDebug() << "addApplet for" << name << "requested, but we're currently immutable!";
#endif
        return 0;
    }

    Applet *applet = PluginLoader::self()->loadApplet(name, id, args);

    if (!applet) {
#ifndef NDEBUG
        kDebug() << "Applet" << name << "could not be loaded.";
#endif
        applet = new Applet(0, QString(), id);
        applet->setFailedToLaunch(true, i18n("Could not find requested component: %1", name));
    }

    //kDebug() << applet->name() << "sizehint:" << applet->sizeHint() << "geometry:" << applet->geometry();

    q->addApplet(applet, appletGeometry.topLeft(), delayInit);
    return applet;
}

void ContainmentPrivate::appletDeleted(Plasma::Applet *applet)
{
    applets.removeAll(applet);

    emit q->appletRemoved(applet);
    emit q->configNeedsSaving();
}

void ContainmentPrivate::appletAppeared(Applet *applet)
{
    //kDebug() << type << Containment::DesktopContainment;
    KConfigGroup *cg = applet->d->mainConfigGroup();
    applet->save(*cg);
    emit q->configNeedsSaving();
}

bool ContainmentPrivate::isPanelContainment() const
{
    return type == Containment::PanelContainment || type == Containment::CustomPanelContainment;
}

bool ContainmentPrivate::prepareContainmentActions(const QString &trigger, const QPoint &screenPos, KMenu *menu)
{
    ContainmentActions *plugin = actionPlugins()->value(trigger);
    if (!plugin) {
        return false;
    }

    if (plugin->containment() != q) {
        plugin->setContainment(q);

        // now configure it
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

}
