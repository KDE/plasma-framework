/*
 *   Copyright 2007 by Aaron Seigo <aseigo@kde.org>
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

#ifndef PLASMA_CONTAINMENT_H
#define PLASMA_CONTAINMENT_H

#include <QtGui/QGraphicsItem>
#include <QtGui/QWidget>

#include <kplugininfo.h>
#include <ksharedconfig.h>
#include <kgenericfactory.h>

#include <plasma/applet.h>
#include <plasma/phase.h>

#include "widgets/icon.h"

namespace Plasma
{

class AppletHandle;
class DataEngine;
class Package;
class Corona;

/**
 * @short The base class for plugins that provide backgrounds and applet grouping containers
 *
 * Containment objects provide the means to group applets into functional sets.
 * They also provide the following:
 *
 * creation of focussing event 
 * - drawing of the background image (which can be interactive)
 * - form factors (e.g. panel, desktop, full screen, etc)
 * - applet layout management
 *
 * Since containment is actually just a Plasma::Applet, all the techniques used
 * for writing the visual presentation of Applets is applicable to Containtments.
 * Containments are differentiated from Applets by being marked with the ServiceType
 * of Plasma/Containment. Plugins registered with both the Applet and the Containment
 * ServiceTypes can be loaded for us in either situation.
 *
 * See techbase.kde.org for a tutorial on writing Containments using this class.
 */
class PLASMA_EXPORT Containment : public Applet
{
    Q_OBJECT

    public:
        typedef QList<Applet*> List;
        typedef QHash<QString, Applet*> Dict;

        enum Type { NoContainmentType = -1 /**< @internal */,
                    DesktopContainment = 0 /**< A desktop containment */,
                    PanelContainment /**< A desktop panel */,
                    CustomContainment /**< A containment that is neither a desktop nor a panel,
                                           but something application specific */
                  };

        /**
         * @arg parent the QGraphicsItem this applet is parented to
         * @arg serviceId the name of the .desktop file containing the
         *      information about the widget
         * @arg appletId a unique id used to differentiate between multiple
         *      instances of the same Applet type
         */
        explicit Containment(QGraphicsItem* parent = 0,
                             const QString& serviceId = QString(),
                             uint containmentId = 0);

        /**
         * This constructor is to be used with the plugin loading systems
         * found in KPluginInfo and KService. The argument list is expected
         * to have two elements: the KService service ID for the desktop entry
         * and an applet ID which must be a base 10 number.
         *
         * @arg parent a QObject parent; you probably want to pass in 0
         * @arg args a list of strings containing two entries: the service id
         *      and the applet id
         */
        Containment(QObject* parent, const QVariantList& args);

        ~Containment();

        /**
         * Reimplemented from Applet
         */
        void init();

        /**
         * Returns the type of containment
         */
        Type containmentType() const;

        /**
         * Sets the type of this containment.
         */
        void setContainmentType(Containment::Type type);

        /**
         * Returns the current form factor the applets in this Containment
         * are being displayed in.
         *
         * @see Plasma::FormFactor
         */
        FormFactor formFactor() const;

        /**
         * Returns the location of this Containment
         *
         * @see Plasma::Location
         */
        Location location() const;

        /**
         * Returns a list of all known containments.
         *
         * @param category Only applets matchin this category will be returned.
         *                 Useful in conjunction with knownCategories.
         *                 If "Misc" is passed in, then applets without a
         *                 Categories= entry are also returned.
         *                 If an empty string is passed in, all applets are
         *                 returned.
         * @param parentApp the application to filter applets on. Uses the
         *                  X-KDE-ParentApp entry (if any) in the plugin info.
         *                  The default value of QString() will result in a
         *                  list containing only applets not specifically
         *                  registered to an application.
         * @return list of applets
         **/
        static KPluginInfo::List knownContainments(const QString &category = QString(),
                                                   const QString &parentApp = QString());

        /**
         * Returns a list of all known applets associated with a certain mimetype
         *
         * @return list of applets
         **/
        static KPluginInfo::List knownContainmentsForMimetype(const QString &mimetype);

        /**
         * Adds an applet to this Containment
         *
         * @param name the plugin name for the applet, as given by
         *        KPluginInfo::pluginName()
         * @param args argument list to pass to the plasmoid
         * @param id to assign to this applet, or 0 to auto-assign it a new id
         * @param geometry where to place the applet, or to auto-place it if an invalid
         *                 is provided
         * @param delayedInit if true, init() will not be called on the applet
         *
         * @return a pointer to the applet on success, or 0 on failure
         */
        Applet* addApplet(const QString& name, const QVariantList& args = QVariantList(),
                          uint id = 0, const QRectF &geometry = QRectF(-1, -1, -1, -1),
                          bool delayedInit = false);

        /**
         * @return the applets currently in this Containment
         */
        Applet::List applets() const;

        /**
         * Removes all applets from this Containment
         */
        void clearApplets();

        /**
         * add existing applet to this containment
         */
        void addApplet(Applet * applet);

        /**
         * Sets the physical screen this Containment is associated with.
         *
         * @param screen the screen number this containment is the desktop for, or -1
         *               if it is not serving as the desktop for any screen
         */
        void setScreen(int screen);

        /**
         * @return the screen number this containment is serving as the desktop for
         *         or -1 if none
         */
        int screen() const;

        /**
         * @internal
         */
        void saveConstraints(KConfigGroup* group) const;

        /**
         * @internal
         */
        void loadConstraints(KConfigGroup* group);

        /**
         * Emits the launchActivated() signal
         */ 
        void emitLaunchActivated();

        /**
         * Constructs a toolbox item and adds it to the toolbox. The toolbox takes over ownership of the item. Returns the constructed tool.
         * 
         * @arg name of the tool
         * @arg name of the icon
         * @arg text to be displayed on the icon
         *
         * @return the constructed tool
         */
        Plasma::Widget * addToolBoxTool(const QString &toolName = QString(), const QString &iconName = QString(), const QString &iconText = QString());

        /**
         * Enables or disables a toolbox tool by name
         *
         * @arg name the name of the tool
         * @arg enable true to enable, false to disable
         */
        void enableToolBoxTool(const QString &toolname, bool enable);

        /**
         * Returns whether or not a given toolbox tool is enabled
         */
        bool isToolboxToolEnabled(const QString &toolname) const;

        /**
         * @internal
         * Called when constraints have been updated on this containment to provide
         * constraint services common to all containments. Containments should still
         * implement their own constraintsUpdated method
         */
        void containmentConstraintsUpdated(Plasma::Constraints constraints);

        /**
         * Open the Plasma toolbox
         */
        void showToolbox();

        /**
         * Close the Plasma toolbox
         */
        void hideToolbox();

    Q_SIGNALS:
        /**
         * This signal is emitted when a new applet is created by the containment
         */
        void appletAdded(Plasma::Applet* applet);

        /**
         * This signal is emitted when an applet is destroyed
         */
        void appletRemoved(Plasma::Applet* applet);

        /**
         * This signal indicates that an application launch, window
         * creation or window focus event was triggered. This is used, for instance,
         * to ensure that the Dashboard view in Plasma hides when such an event is
         * triggered by an item it is displaying.
         */
        void launchActivated();

        /**
         * Emitted when the containment requests zooming out one step.
         * Usually only used for desktop containments.
         */
        void zoomIn();

        /**
         * Emitted when the containment requests zooming out one step.
         * Usually only used for desktop containments.
         */
        void zoomOut();

        /**
         * Emitted when the containment requests an add widgets dialog is shown.
         * Usually only used for desktop containments.
         */
        void showAddWidgets();

    public Q_SLOTS:
        /**
         * Informs the Corona as to what position it is in. This is informational
         * only, as the Corona doesn't change it's actual location. This is,
         * however, passed on to Applets that may be managed by this Corona.
         *
         * @param location the new location of this Corona
         */
        void setLocation(Plasma::Location location);

        /**
         * Sets the form factor for this Corona. This may cause changes in both
         * the arrangement of Applets as well as the display choices of individual
         * Applets.
         */
        void setFormFactor(Plasma::FormFactor formFactor);

        /**
         * Returns the Corona (if any) that this Containment is hosted by
         */
        Corona* corona() const;

    protected:
        void contextMenuEvent(QGraphicsSceneContextMenuEvent * event);
        void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
        void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);
        bool sceneEventFilter(QGraphicsItem *watched, QEvent *event);

    protected Q_SLOTS:
        /**
         * @internal
         */
        void appletDestroyed(QObject*);
        void appletAnimationComplete(QGraphicsItem *item, Plasma::Phase::Animation anim);
        void dropEvent(QGraphicsSceneDragDropEvent* event);

    private Q_SLOTS:
        void handleDisappeared(AppletHandle *handle);
        void destroyApplet();

    private:
        QRectF geometryForApplet(Applet *applet) const;
        bool regionIsEmpty(const QRectF &region, Applet *ignoredApplet=0) const;
        void prepareApplet(Applet *applet, bool delayInit);

        Q_DISABLE_COPY(Containment)

        class Private;
        Private* const d;
};

} // Plasma namespace


#endif // multiple inclusion guard
