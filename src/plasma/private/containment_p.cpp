/*
    SPDX-FileCopyrightText: 2007 Aaron Seigo <aseigo@kde.org>
    SPDX-FileCopyrightText: 2008 Ménard Alexis <darktears31@gmail.com>
    SPDX-FileCopyrightText: 2009 Chani Armitage <chani@kde.org>
    SPDX-FileCopyrightText: 2012 Marco Martin <notmart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "private/containment_p.h"

#include <KLocalizedString>
#include <QDebug>

#include "config-plasma.h"

#include "pluginloader.h"

#include "debug_p.h"
#include "private/applet_p.h"
#include "timetracker.h"

namespace Plasma
{
const char ContainmentPrivate::defaultWallpaper[] = "org.kde.image";

ContainmentPrivate::ContainmentPrivate(Containment *c)
    : q(c)
    , formFactor(Types::Planar)
    , location(Types::Floating)
    , lastScreen(-1)
    , type(Plasma::Types::NoContainmentType) // never had a screen
    , uiReady(false)
    , appletsUiReady(false)
{
    // if the parent is an applet (i.e we are the systray)
    // we want to follow screen changed signals from the parent's containment
    auto appletParent = qobject_cast<Plasma::Applet *>(c->parent());
    if (appletParent) {
        QObject::connect(appletParent->containment(), &Containment::screenChanged, c, &Containment::screenChanged);
    }
}

Plasma::ContainmentPrivate::~ContainmentPrivate()
{
    applets.clear();
}

void ContainmentPrivate::addDefaultActions(KActionCollection *actions, Containment *c)
{
    actions->setConfigGroup(QStringLiteral("Shortcuts-Containment"));

    // adjust applet actions
    QAction *appAction = qobject_cast<QAction *>(actions->action(QStringLiteral("remove")));
    appAction->setShortcut(QKeySequence(Qt::ALT | Qt::Key_D, Qt::ALT | Qt::Key_R));
    if (c && c->d->isPanelContainment()) {
        appAction->setText(i18n("Remove this Panel"));
    } else {
        appAction->setText(i18n("Remove this Activity"));
    }

    appAction = qobject_cast<QAction *>(actions->action(QStringLiteral("configure")));
    if (appAction) {
        appAction->setShortcut(QKeySequence(Qt::ALT | Qt::Key_D, Qt::ALT | Qt::Key_S));
        appAction->setText(i18n("Activity Settings"));
    }

    // add our own actions
    QAction *appletBrowserAction = actions->add<QAction>(QStringLiteral("add widgets"));
    appletBrowserAction->setAutoRepeat(false);
    appletBrowserAction->setText(i18n("Add Widgets..."));
    appletBrowserAction->setIcon(QIcon::fromTheme(QStringLiteral("list-add")));
    appletBrowserAction->setShortcut(QKeySequence(Qt::ALT | Qt::Key_D, Qt::Key_A));
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
    // qCDebug(LOG_PLASMA) << "================== "<< appletStatus << q->status();
    if (appletStatus == q->status()) {
        return;
    }

    if (appletStatus < q->status() || appletStatus == Plasma::Types::HiddenStatus) {
        // check to see if any other applet has a higher status, and stick with that if we do
        // we'll treat HiddenStatus as lowest as we cannot change the enum value which is highest anymore
        for (Applet *applet : std::as_const(applets)) {
            if (applet->status() > appletStatus && applet->status() != Plasma::Types::HiddenStatus) {
                appletStatus = applet->status();
            }
        }
    }

    if (appletStatus != Plasma::Types::HiddenStatus) {
        q->setStatus(appletStatus);
    }
}

void ContainmentPrivate::triggerShowAddWidgets()
{
    Q_EMIT q->showAddWidgetsInterface(QPointF());
}

void ContainmentPrivate::containmentConstraintsEvent(Plasma::Types::Constraints constraints)
{
    if (!q->isContainment()) {
        return;
    }

    // qCDebug(LOG_PLASMA) << "got containmentConstraintsEvent" << constraints;
    if (constraints & Plasma::Types::ImmutableConstraint) {
        // update actions
        const bool unlocked = q->immutability() == Types::Mutable;

        QAction *action = q->actions()->action(QStringLiteral("remove"));
        if (action) {
            action->setEnabled(unlocked);
            action->setVisible(unlocked);
        }

        action = q->actions()->action(QStringLiteral("add widgets"));
        if (action) {
            action->setEnabled(unlocked);
            action->setVisible(unlocked);
        }

        // tell the applets too
        for (Applet *a : std::as_const(applets)) {
            /*Why qMin?
             * the applets immutability() is the maximum between internal applet immutability
             * and the immutability of its containment.
             * so not set higher immutability in the internal member of Applet
             * or the applet will not be able to be unlocked properly
             */
            a->setImmutability(qMin(q->immutability(), a->d->immutability));
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
        for (Applet *applet : std::as_const(applets)) {
            applet->updateConstraints(appletConstraints);
        }
    }
}

Applet *ContainmentPrivate::createApplet(const QString &name, const QVariantList &args, uint id)
{
    if (!q->isContainment()) {
        return nullptr;
    }

    if (q->immutability() != Types::Mutable && !args.contains(QVariant::fromValue(QStringLiteral("org.kde.plasma:force-create")))) {
#ifndef NDEBUG
        // qCDebug(LOG_PLASMA) << "addApplet for" << name << "requested, but we're currently immutable!";
#endif
        return nullptr;
    }

    Applet *applet = PluginLoader::self()->loadApplet(name, id, args);

    if (!applet) {
        qCWarning(LOG_PLASMA) << "Applet" << name << "could not be loaded.";
        applet = new Applet(nullptr, KPluginMetaData(), QVariantList{QVariant(), QVariant(), id});
        applet->setLaunchErrorMessage(i18n("Could not find requested component: %1", name));
    }

    q->addApplet(applet);
    // mirror behavior of resorecontents: if an applet is not valid, set it immediately to uiReady
    if (!applet->pluginMetaData().isValid()) {
        applet->updateConstraints(Plasma::Types::UiReadyConstraint);
    }
    return applet;
}

void ContainmentPrivate::appletDeleted(Plasma::Applet *applet)
{
    applets.removeAll(applet);

    Q_EMIT q->appletRemoved(applet);
    Q_EMIT q->configNeedsSaving();
}

bool ContainmentPrivate::isPanelContainment() const
{
    return type == Plasma::Types::PanelContainment || type == Plasma::Types::CustomPanelContainment;
}

void ContainmentPrivate::setStarted()
{
    if (!q->Applet::d->started) {
        q->Applet::d->started = true;

        if (uiReady) {
            Q_EMIT q->uiReadyChanged(true);
        }
    }
}

void ContainmentPrivate::setUiReady()
{
    // if we are the containment and there is still some incomplete applet, we're still incomplete
    if (!uiReady) {
        uiReady = true;
        if (q->Applet::d->started && (appletsUiReady || applets.isEmpty()) && loadingApplets.isEmpty()) {
            Q_EMIT q->uiReadyChanged(true);
        }
    }
}

void ContainmentPrivate::appletLoaded(Applet *applet)
{
    loadingApplets.remove(applet);

    if (loadingApplets.isEmpty() && !appletsUiReady) {
        appletsUiReady = true;
        if (q->Applet::d->started && uiReady) {
            Q_EMIT q->uiReadyChanged(true);
        }
    }
}

}
