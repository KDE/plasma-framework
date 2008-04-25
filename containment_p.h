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

static const int INTER_CONTAINMENT_MARGIN = 6;
static const int VERTICAL_STACKING_OFFSET = 10000;

namespace Plasma
{

class Containment;
class Toolbox;

class Containment::Private
{
public:
    Private(Containment* c)
        : q(c),
          formFactor(Planar),
          location(Floating),
          screen(-1), // no screen
          toolBox(0),
          type(Containment::NoContainmentType),
          positioning(false)
    {
    }

    ~Private()
    {
        qDeleteAll(applets);
        applets.clear();
    }

    Toolbox* createToolBox();
    void positionToolBox();
    void triggerShowAddWidgets();

    /**
     * Called when constraints have been updated on this containment to provide
     * constraint services common to all containments. Containments should still
     * implement their own constraintsUpdated method
     */
    void containmentConstraintsUpdated(Plasma::Constraints constraints);

    bool regionIsEmpty(const QRectF &region, Applet *ignoredApplet=0) const;
    void positionPanel(bool force = false);
    void positionContainment();
    void setLockToolText();
    void handleDisappeared(AppletHandle *handle);
    void destroyApplet();
    void appletDestroyed(QObject*);
    void appletAnimationComplete(QGraphicsItem *item, Plasma::Animator::Animation anim);
    void zoomIn();
    void zoomOut();

    /**
     * Locks or unlocks plasma's applets.
     * When plasma is locked, applets cannot be transformed, added or deleted
     * but they can still be configured.
     */
    void toggleDesktopImmutability();

    Applet* addApplet(const QString& name, const QVariantList& args = QVariantList(),
                      const QRectF &geometry = QRectF(-1, -1, -1, -1), uint id = 0,
                      bool delayedInit = false);

    Containment *q;
    FormFactor formFactor;
    Location location;
    Applet::List applets;
    QMap<Applet*, AppletHandle*> handles;
    int screen;
    Toolbox *toolBox;
    Containment::Type type;
    bool positioning;
};

} // Plasma namespace

#endif
