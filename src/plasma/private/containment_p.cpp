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


#include <kaction.h>
#include <kactioncollection.h>
#include <kiconloader.h>
#include <klocalizedstring.h>
#include <kwindowsystem.h>

#include "config-plasma.h"

#include "containmentactions.h"
#include "corona.h"
#include "pluginloader.h"

#include "private/applet_p.h"


namespace Plasma
{

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
    appletBrowserAction->setData(Plasma::AddAction);

    KAction *action = actions->addAction("next applet");
    action->setText(i18n("Next Widget"));
    //no icon
    action->setShortcut(KShortcut("alt+d, n"));
    action->setData(Plasma::ControlAction);

    action = actions->addAction("previous applet");
    action->setText(i18n("Previous Widget"));
    //no icon
    action->setShortcut(KShortcut("alt+d, p"));
    action->setData(Plasma::ControlAction);
}

void ContainmentPrivate::setScreen(int newScreen)
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

    //kDebug() << activity() << "setting screen to " << newScreen << "and type is" << type;

    Containment *swapScreensWith(0);
    const bool isDesktopContainment = type == Plasma::DesktopContainment ||
                                      type == Plasma::CustomContainment;
    if (isDesktopContainment) {
        if (newScreen > -1) {
            // sanity check to make sure someone else doesn't have this screen already!
            Containment *currently = corona->containmentForScreen(newScreen);
            if (currently && currently != q) {
#ifndef NDEBUG
                kDebug() << "currently is on screen" << currently->screen()
                         << "desktop" << currently->desktop()
                         << "and is" << currently->activity()
                         << (QObject*)currently << "i'm" << (QObject*)q;
#endif
                currently->setScreen(-1);
                swapScreensWith = currently;
            }
        }
    }

    int oldScreen = screen;
    screen = newScreen;

    q->updateConstraints(Plasma::ScreenConstraint);

    if (oldScreen != newScreen) {
        /*
#ifndef NDEBUG
        kDebug() << "going to signal change for" << q
#endif
                 << ", old screen & desktop:" << oldScreen
                 << ", new:" << screen << desktop;
                 */
        KConfigGroup c = q->config();
        c.writeEntry("screen", screen);
        emit q->configNeedsSaving();
        emit q->screenChanged(oldScreen, newScreen, q);
    }

    if (swapScreensWith) {
        //kDebug() << "setScreen due to swap, part 2";
        swapScreensWith->setScreen(oldScreen);
    }

    checkRemoveAction();

    if (newScreen >= 0) {
        emit q->activate();
    }
}

KConfigGroup ContainmentPrivate::containmentActionsConfig() const
{
    KConfigGroup cfg;
    switch (containmentActionsSource) {
    case ContainmentActions::Local:
        cfg = q->config();
        cfg = KConfigGroup(&cfg, "ActionPlugins");
        break;
    case ContainmentActions::Activity:
        cfg = KConfigGroup(q->corona()->config(), "Activities");
        cfg = KConfigGroup(&cfg, activityId);
        cfg = KConfigGroup(&cfg, "ActionPlugins");
        break;
    default:
        cfg = KConfigGroup(q->corona()->config(), "ActionPlugins");
    }
    return cfg;
}

KActionCollection* ContainmentPrivate::actions()
{
    return static_cast<Applet*>(q)->d->actions;
}

void ContainmentPrivate::configChanged()
{
    KConfigGroup group = q->config();
    q->setWallpaper(group.readEntry("wallpaperplugin", defaultWallpaper));
}

void ContainmentPrivate::requestConfiguration()
{
    emit q->configureRequested(q);
}

void ContainmentPrivate::checkStatus(Plasma::ItemStatus appletStatus)
{
    //kDebug() << "================== "<< appletStatus << q->status();
    if (appletStatus == q->status()) {
        emit q->statusChanged(appletStatus);
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

    //kDebug() << "got containmentConstraintsEvent" << constraints;
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

    if (constraints & Plasma::StartupCompletedConstraint && type < Plasma::CustomContainment) {
        checkRemoveAction();
    }
}

Applet *ContainmentPrivate::addApplet(const QString &name, const QVariantList &args, uint id)
{
    if (!q->isContainment()) {
        return 0;
    }

    if (q->immutability() != Mutable) {
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
        applet->setLaunchErrorMessage(i18n("Could not find requested component: %1", name));
    }

    //kDebug() << applet->title() << "sizehint:" << applet->sizeHint() << "geometry:" << applet->geometry();

    q->addApplet(applet);
    return applet;
}

void ContainmentPrivate::appletDeleted(Plasma::Applet *applet)
{
    applets.removeAll(applet);

    emit q->appletRemoved(applet);
    emit q->configNeedsSaving();
}

bool ContainmentPrivate::isPanelContainment() const
{
    return type == Plasma::PanelContainment || type == Plasma::CustomPanelContainment;
}

}
