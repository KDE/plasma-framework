/*
    SPDX-FileCopyrightText: 2007 Aaron Seigo <aseigo@kde.org>
    SPDX-FileCopyrightText: 2008 Ménard Alexis <darktears31@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef CONTAINMENT_P_H
#define CONTAINMENT_P_H

#include <KActionCollection>
#include <QSet>

#include "applet.h"
#include "containmentactions.h"
#include "corona.h"
#include "plasma.h"

class KJob;

namespace KIO
{
class Job;
}

namespace Plasma
{
class Containment;

class ContainmentPrivate
{
public:
    ContainmentPrivate(Containment *c);
    ~ContainmentPrivate();

    void triggerShowAddWidgets();
    void checkStatus(Plasma::Types::ItemStatus status);

    /**
     * Called when constraints have been updated on this containment to provide
     * constraint services common to all containments. Containments should still
     * implement their own constraintsEvent method
     */
    void containmentConstraintsEvent(Plasma::Types::Constraints constraints);

    bool isPanelContainment() const;
    void appletDeleted(Applet *);
    void configChanged();

    Applet *createApplet(const QString &name, const QVariantList &args = QVariantList(), uint id = 0);

    /**
     * FIXME: this should completely go from here
     * @return the config group that containmentactions plugins go in
     * @since 4.6
     */
    KConfigGroup containmentActionsConfig() const;

    /**
     * add the regular actions & keyboard shortcuts onto Applet's collection
     */
    static void addDefaultActions(KActionCollection *actions, Containment *c = nullptr);

    void setUiReady();
    void setStarted();
    void appletLoaded(Applet *applet);

    Containment *q;
    Types::FormFactor formFactor;
    Types::Location location;
    Types::ContainmentDisplayHints containmentDisplayHints = Types::NoContainmentDisplayHint;

    QList<Applet *> applets;
    // Applets still considered not ready
    QSet<Applet *> loadingApplets;
    QString wallpaper;
    QHash<QString, ContainmentActions *> localActionPlugins;
    int lastScreen;
    QString activityId;
    Types::ContainmentType type;
    bool uiReady : 1;
    bool appletsUiReady : 1;

    static const char defaultWallpaper[];
};

} // Plasma namespace

#endif
