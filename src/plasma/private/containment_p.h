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

#ifndef CONTAINMENT_P_H
#define CONTAINMENT_P_H

#include <kactioncollection.h>
#include <kmenu.h>

#include "plasma.h"
#include "applet.h"
#include "corona.h"
#include "containmentactions.h"


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
    ContainmentPrivate(Containment *c)
        : q(c),
          formFactor(Planar),
          location(Floating),
          screen(-1), // no screen
          type(Plasma::NoContainmentType),
          drawWallpaper(false),
          containmentActionsSource(ContainmentActions::Global)
    {
    }

    ~ContainmentPrivate()
    {
        // qDeleteAll is broken with Qt4.8, delete it by hand
        while (!applets.isEmpty()) {
            delete applets.first();
        }
        applets.clear();
    }

    void triggerShowAddWidgets();
    void requestConfiguration();
    void checkStatus(Plasma::ItemStatus status);
    void setScreen(int newScreen);

    /**
     * Called when constraints have been updated on this containment to provide
     * constraint services common to all containments. Containments should still
     * implement their own constraintsEvent method
     */
    void containmentConstraintsEvent(Plasma::Constraints constraints);

    bool isPanelContainment() const;
    void setLockToolText();
    void appletDeleted(Applet*);
    void checkRemoveAction();
    void configChanged();

    Applet *addApplet(const QString &name, const QVariantList &args = QVariantList(), uint id = 0);

    KActionCollection *actions();

    /**
     * FIXME: this should completely go from here
     * @return the config group that containmentactions plugins go in
     * @since 4.6
     */
    KConfigGroup containmentActionsConfig() const;

    /**
     * add the regular actions & keyboard shortcuts onto Applet's collection
     */
    static void addDefaultActions(KActionCollection *actions, Containment *c = 0);

    Containment *q;
    FormFactor formFactor;
    Location location;
    QList<Applet *> applets;
    QString wallpaper;
    QHash<QString, ContainmentActions*> localActionPlugins;
    int screen;
    QString activityId;
    ContainmentType type;
    bool drawWallpaper : 1;

    ContainmentActions::ContainmentActionsSource containmentActionsSource;
    static QHash<QString, ContainmentActions*> globalActionPlugins;
    static const char defaultWallpaper[];
    static const char defaultWallpaperMode[];
};

} // Plasma namespace

#endif
