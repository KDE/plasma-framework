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

#include <kactioncollection.h>
#include <QDebug>
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

const char ContainmentPrivate::defaultWallpaper[] = "org.kde.image";

void ContainmentPrivate::addDefaultActions(KActionCollection *actions, Containment *c)
{
    actions->setConfigGroup("Shortcuts-Containment");

    //adjust applet actions
    QAction *appAction = qobject_cast<QAction *>(actions->action("remove"));
    appAction->setShortcut(QKeySequence("alt+d, alt+r"));
    if (c && c->d->isPanelContainment()) {
        appAction->setText(i18n("Remove this Panel"));
    } else {
        appAction->setText(i18n("Remove this Activity"));
    }

    appAction = qobject_cast<QAction *>(actions->action("configure"));
    if (appAction) {
        appAction->setShortcut(QKeySequence("alt+d, alt+s"));
        appAction->setText(i18n("Activity Settings"));
    }

    //add our own actions
    QAction *appletBrowserAction = actions->add<QAction>("add widgets");
    appletBrowserAction->setAutoRepeat(false);
    appletBrowserAction->setText(i18n("Add Widgets..."));
    appletBrowserAction->setIcon(QIcon::fromTheme("list-add"));
    appletBrowserAction->setShortcut(QKeySequence("alt+d, a"));
    appletBrowserAction->setData(Plasma::Types::AddAction);
}

KConfigGroup ContainmentPrivate::containmentActionsConfig() const
{
    KConfigGroup cfg = KConfigGroup(q->corona()->config(), "ActionPlugins");
    return KConfigGroup(&cfg, QString::number(type));
}

void ContainmentPrivate::configChanged()
{
    KConfigGroup group = q->config();
    q->setWallpaper(group.readEntry("wallpaperplugin", defaultWallpaper));
}

void ContainmentPrivate::checkStatus(Plasma::Types::ItemStatus appletStatus)
{
    //qDebug() << "================== "<< appletStatus << q->status();
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

void ContainmentPrivate::containmentConstraintsEvent(Plasma::Types::Constraints constraints)
{
    if (!q->isContainment()) {
        return;
    }

    //qDebug() << "got containmentConstraintsEvent" << constraints;
    if (constraints & Plasma::Types::ImmutableConstraint) {
        //update actions
        const bool unlocked = q->immutability() == Types::Mutable;

        QAction *action = q->actions()->action("remove");
        if (action) {
            action->setEnabled(unlocked);
            action->setVisible(unlocked);
        }

        action = q->actions()->action("add widgets");
        if (action) {
            action->setEnabled(unlocked);
            action->setVisible(unlocked);
        }

        // tell the applets too
        foreach (Applet *a, applets) {
            a->setImmutability(q->immutability());
            a->updateConstraints(Types::ImmutableConstraint);
        }
    }

    // pass on the constraints that are relevant here
    Types::Constraints appletConstraints = Types::NoConstraint;
    if (constraints & Types::FormFactorConstraint) {
        appletConstraints |= Types::FormFactorConstraint;
    }

    if (constraints & Types::ScreenConstraint) {
        appletConstraints |= Types::ScreenConstraint;
    }

    if (appletConstraints != Types::NoConstraint) {
        foreach (Applet *applet, applets) {
            applet->updateConstraints(appletConstraints);
        }
    }
}

Applet *ContainmentPrivate::createApplet(const QString &name, const QVariantList &args, uint id)
{
    if (!q->isContainment()) {
        return 0;
    }

    if (q->immutability() != Types::Mutable) {
#ifndef NDEBUG
        // qDebug() << "addApplet for" << name << "requested, but we're currently immutable!";
#endif
        return 0;
    }

    Applet *applet = PluginLoader::self()->loadApplet(name, id, args);

    if (!applet) {
        qWarning() << "Applet" << name << "could not be loaded.";
        applet = new Applet(0, QString(), id);
        applet->setLaunchErrorMessage(i18n("Could not find requested component: %1", name));
    }

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
    return type == Plasma::Types::PanelContainment || type == Plasma::Types::CustomPanelContainment;
}

}
