/*
 *   Copyright 2007 by Aaron Seigo <aseigo@kde.org>
 *   Copyright 2008 by Ménard Alexis <darktears31@gmail.com>
 *   Copyright (c) 2009 Chani Armitage <chani@kde.org>
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
#include <QtGui/QStyleOptionGraphicsItem>

#include <kplugininfo.h>
#include <ksharedconfig.h>
#include <kgenericfactory.h>

#include <plasma/applet.h>
#include <plasma/animator.h>

namespace KIO
{
    class Job;
}

namespace Plasma
{

class AccessAppletJob;
class AppletHandle;
class DataEngine;
class Package;
class Corona;
class View;
class Wallpaper;
class ContainmentActions;
class ContainmentPrivate;
class AbstractToolBox;

/**
 * @class Containment plasma/containment.h <Plasma/Containment>
 *
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
        class PLASMA_EXPORT StyleOption : public QStyleOptionGraphicsItem
        {
            public:
                explicit StyleOption();
                explicit StyleOption(const StyleOption &other);
                explicit StyleOption(const QStyleOptionGraphicsItem &other);

                enum StyleOptionType {
                    Type = SO_CustomBase + 1
                };
                enum StyleOptionVersion {
                    Version = QStyleOptionGraphicsItem::Version + 1
                };

                /**
                 * The View, if any, that this containment is currently
                 * being rendered into. Note: this may be NULL, so be
                 * sure to check it before using it!
                 */
                Plasma::View *view;
        };

        enum Type {
            NoContainmentType = -1,  /**< @internal */
            DesktopContainment = 0,  /**< A desktop containment */
            PanelContainment,        /**< A desktop panel */
            CustomContainment = 127, /**< A containment that is neither a desktop nor a panel
                                        but something application specific */
            CustomPanelContainment = 128 /**< A customized desktop panel */
        };

        /**
         * @param parent the QGraphicsItem this applet is parented to
         * @param serviceId the name of the .desktop file containing the
         *      information about the widget
         * @param containmentId a unique id used to differentiate between multiple
         *      instances of the same Applet type
         */
        explicit Containment(QGraphicsItem *parent = 0,
                             const QString &serviceId = QString(),
                             uint containmentId = 0);

        /**
         * This constructor is to be used with the plugin loading systems
         * found in KPluginInfo and KService. The argument list is expected
         * to have two elements: the KService service ID for the desktop entry
         * and an applet ID which must be a base 10 number.
         *
         * @param parent a QObject parent; you probably want to pass in 0
         * @param args a list of strings containing two entries: the service id
         *      and the applet id
         */
        Containment(QObject *parent, const QVariantList &args);

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
         * Returns the Corona (if any) that this Containment is hosted by
         */
        Corona *corona() const;

        /**
         * Returns a list of all known containments.
         *
         * @param category Only containments matching this category will be returned.
         *                 Useful in conjunction with knownCategories.
         *                 If "Miscellaneous" is passed in, then applets without a
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
        static KPluginInfo::List listContainments(const QString &category = QString(),
                                                  const QString &parentApp = QString());

        /**
         * Returns a list of all known Containments that match the parameters.
         *
         * @param type Only Containments with this string in X-Plasma-ContainmentCategories
         *             in their .desktop files will be returned. Common values are panel and
         *             desktop
         * @param category Only applets matchin this category will be returned.
         *                 Useful in conjunction with knownCategories.
         *                 If "Miscellaneous" is passed in, then applets without a
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
        static KPluginInfo::List listContainmentsOfType(const QString &type,
                                                        const QString &category = QString(),
                                                        const QString &parentApp = QString());

        /**
         * @return a list of all known types of Containments on this system
         */
        static QStringList listContainmentTypes();

        /**
         * Returns a list of all known applets associated with a certain mimetype
         *
         * @return list of applets
         **/
        static KPluginInfo::List listContainmentsForMimetype(const QString &mimetype);

        /**
         * Adds an applet to this Containment
         *
         * @param name the plugin name for the applet, as given by
         *        KPluginInfo::pluginName()
         * @param args argument list to pass to the plasmoid
         * @param geometry where to place the applet, or to auto-place it if an invalid
         *                 is provided
         *
         * @return a pointer to the applet on success, or 0 on failure
         */
        Applet *addApplet(const QString &name, const QVariantList &args = QVariantList(),
                          const QRectF &geometry = QRectF(-1, -1, -1, -1));

        /**
         * Add an existing applet to this Containment
         *
         * If dontInit is true, the pending constraints are not flushed either.
         * So it is your responsibility to call both init() and
         * flushPendingConstraints() on the applet.
         *
         * @param applet the applet that should be added
         * @param pos the containment-relative position
         * @param dontInit if true, init() will not be called on the applet
         */
        void addApplet(Applet *applet, const QPointF &pos = QPointF(-1, -1), bool dontInit = true);

        /**
         * @return the applets currently in this Containment
         */
        Applet::List applets() const;

        /**
         * Removes all applets from this Containment
         */
        void clearApplets();

        /**
         * Sets the physical screen this Containment is associated with.
         *
         * @param screen the screen number this containment is the desktop for, or -1
         *               if it is not serving as the desktop for any screen
         * @param desktop the virtual desktop to also associate this this screen with
         */
        void setScreen(int screen, int desktop = -1);

        /**
         * @return the screen number this containment is serving as the desktop for
         *         or -1 if none
         */
        int screen() const;

        /**
         * @return the viewport (e.g. virtual desktop) this Containment is associated with.
         */
        int desktop() const;

        /**
         * @reimp
         * @sa Applet::save(KConfigGroup &)
         */
        void save(KConfigGroup &group) const;

        /**
         * @reimp
         * @sa Applet::restore(KConfigGroup &)
         */
        void restore(KConfigGroup &group);

        /**
         * convenience function - enables or disables an action by name
         *
         * @param name the name of the action in our collection
         * @param enable true to enable, false to disable
         */
        void enableAction(const QString &name, bool enable);

        /**
         * Add an action to the toolbox
         */
        void addToolBoxAction(QAction *action);

        /**
         * Remove an action from the toolbox
         */
        void removeToolBoxAction(QAction *action);

        /**
         * Sets the open or closed state of the Containment's toolbox
         *
         * @arg open true to open the ToolBox, false to close it
         */
        void setToolBoxOpen(bool open);

        /**
         * Open the Containment's toolbox
         */
        void openToolBox();

        /**
         * Closes Containment's toolbox
         */
        void closeToolBox();

        /**
         * associate actions with this widget, including ones added after this call.
         * needed to make keyboard shortcuts work.
         */
        void addAssociatedWidget(QWidget *widget);

        /**
         * un-associate actions from this widget, including ones added after this call.
         * needed to make keyboard shortcuts work.
         */
        void removeAssociatedWidget(QWidget *widget);

        /**
         * Return whether wallpaper is painted or not.
         */
        bool drawWallpaper();

        /**
         * Sets wallpaper plugin.
         *
         * @param pluginName the name of the wallpaper to attempt to load
         * @param mode optional mode or the wallpaper plugin (e.g. "Slideshow").
         *        These values are pugin specific and enumerated in the plugin's
         *        .desktop file.
         */
        void setWallpaper(const QString &pluginName, const QString &mode = QString());

        /**
         * Return wallpaper plugin.
         */
        Plasma::Wallpaper *wallpaper() const;

        /**
         * Sets the current activity by name
         *
         * @param activity the name of the activity; if it doesn't exist in the
         *        semantic store, it will be created.
         */
        void setActivity(const QString &activity);

        /**
         * @return the current activity associated with this activity
         */
        QString activity() const;

        /**
         * Shows the context menu for the containment directly, bypassing Applets
         * altogether.
         */
        void showContextMenu(const QPointF &containmentPos, const QPoint &screenPos);

        /**
         * Shows a visual clue for drag and drop
         * The default implementation does nothing,
         * reimplement in containments that need it
         *
         * @param pos point where to show the drop target; if an invalid point is passed in
         *        the drop zone should not be shown
         */
        virtual void showDropZone(const QPoint pos);

        /**
         * Sets a containmentactions plugin.
         *
         * @param trigger the mouse button (and optional modifier) to associate the plugin with
         * @param pluginName the name of the plugin to attempt to load. blank = set no plugin.
         * @since 4.4
         */
        void setContainmentActions(const QString &trigger, const QString &pluginName);

        /**
         * @return a list of all triggers that have a containmentactions plugin associated
         * @since 4.4
         */
        QStringList containmentActionsTriggers();

        /**
         * @return the plugin name for the given trigger
         * @since 4.4
         */
        QString containmentActions(const QString &trigger);

    Q_SIGNALS:
        /**
         * This signal is emitted when a new applet is created by the containment
         */
        void appletAdded(Plasma::Applet *applet, const QPointF &pos);

        /**
         * This signal is emitted when an applet is destroyed
         */
        void appletRemoved(Plasma::Applet *applet);

        /**
         * Emitted when the containment requests zooming in or out one step.
         */
        void zoomRequested(Plasma::Containment *containment, Plasma::ZoomDirection direction);

        /**
         * Emitted when the user clicks on the toolbox
         */
        void toolBoxToggled();

        /**
         * Emitted when the toolbox is hidden or shown
         * @since 4.3
         */
        void toolBoxVisibilityChanged(bool);

        /**
         * Emitted when the containment wants a new containment to be created.
         * Usually only used for desktop containments.
         */
        void addSiblingContainment(Plasma::Containment *);

        /**
         * Emitted when the containment requests an add widgets dialog is shown.
         * Usually only used for desktop containments.
         *
         * @param pos where in the containment this request was made from, or
         *            an invalid position (QPointF()) is not location specific
         */
        void showAddWidgetsInterface(const QPointF &pos);

        /**
         * This signal indicates that a containment has been newly
         * associated (or dissociated) with a physical screen.
         *
         * @param wasScreen the screen it was associated with
         * @param isScreen the screen it is now associated with
         * @param containment the containment switching screens
         */
        void screenChanged(int wasScreen, int isScreen, Plasma::Containment *containment);

        /**
         * Emitted when the user wants to configure/change containment.
         */
        void configureRequested(Plasma::Containment *containment);

        /**
         * The activity associated to this containemnt has changed
         */
        void contextChanged(Plasma::Context *context);

    public Q_SLOTS:
        /**
         * Informs the Corona as to what position it is in. This is informational
         * only, as the Corona doesn't change its actual location. This is,
         * however, passed on to Applets that may be managed by this Corona.
         *
         * @param location the new location of this Corona
         */
        void setLocation(Plasma::Location location);

        /**
         * Sets the form factor for this Containment. This may cause changes in both
         * the arrangement of Applets as well as the display choices of individual
         * Applets.
         */
        void setFormFactor(Plasma::FormFactor formFactor);

        /**
         * Tells the corona to create a new desktop containment
         */
        void addSiblingContainment();

        /**
         * switch keyboard focus to the next of our applets
         */
        void focusNextApplet();

        /**
         * switch keyboard focus to the previous one of our applets
         */
        void focusPreviousApplet();

        /**
         * Destroys this containment and all its applets (after a confirmation dialog);
         * it will be removed nicely and deleted.
         * Its configuration will also be deleted.
         */
        void destroy();

        /**
         * Destroys this containment and all its applets (after a confirmation dialog);
         * it will be removed nicely and deleted.
         * Its configuration will also be deleted.
         *
         * @arg confirm whether or not confirmation from the user should be requested
         */
        void destroy(bool confirm);

        /**
         * @reimp
         * @sa Applet::showConfigurationInterface()
         */
        void showConfigurationInterface();

    protected:
        /**
         * Sets the type of this containment.
         */
        void setContainmentType(Containment::Type type);

        /**
         * Sets whether wallpaper is painted or not.
         */
        void setDrawWallpaper(bool drawWallpaper);

        /**
         * Called when the contents of the containment should be saved. By default this saves
         * all loaded Applets
         *
         * @param group the KConfigGroup to save settings under
         */
        virtual void saveContents(KConfigGroup &group) const;

        /**
         * Called when the contents of the containment should be loaded. By default this loads
         * all previously saved Applets
         *
         * @param group the KConfigGroup to save settings under
         */
        virtual void restoreContents(KConfigGroup &group);

        void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
        void mousePressEvent(QGraphicsSceneMouseEvent *event);
        void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
        void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);
        void keyPressEvent(QKeyEvent *event);
        void wheelEvent(QGraphicsSceneWheelEvent *event);
        bool sceneEventFilter(QGraphicsItem *watched, QEvent *event);
        QVariant itemChange(GraphicsItemChange change, const QVariant &value);

        /**
         * @reimp
         * @sa QGraphicsItem::dragEnterEvent()
         */
        void dragEnterEvent(QGraphicsSceneDragDropEvent *event);

        /**
         * @reimp
         * @sa QGraphicsItem::dragLeaveEvent()
         */
        void dragLeaveEvent(QGraphicsSceneDragDropEvent *event);

        /**
         * @reimp
         * @sa QGraphicsItem::dragMoveEvent()
         */
        void dragMoveEvent(QGraphicsSceneDragDropEvent *event);

        /**
         * @reimp
         * @sa QGraphicsItem::dropEvent()
         */
        void dropEvent(QGraphicsSceneDragDropEvent *event);

        /**
         * @reimp
         * @sa QGraphicsItem::resizeEvent()
         */
        void resizeEvent(QGraphicsSceneResizeEvent *event);

        /**
         * @returns the toolbox associated with this containment, or a null pointer if none
         */
        KDE_DEPRECATED const QGraphicsItem *toolBoxItem() const;

        /**
         * Sets a custom ToolBox
         * if there was an old one it will be deleted
         * and the new one won't have any actions in it
         *
         * @param item the new toolbox item
         * @since 4.4
         */
        void setToolBox(AbstractToolBox *toolBox);

        /**
         * @return the ToolBox
         * @since 4.4
         */
        AbstractToolBox *toolBox() const;

    private:
        /**
         * @internal This constructor is to be used with the Package loading system.
         *
         * @param parent a QObject parent; you probably want to pass in 0
         * @param args a list of strings containing two entries: the service id
         *      and the applet id
         * @since 4.3
         */
        Containment(const QString &packagePath, uint appletId, const QVariantList &args);

        Q_PRIVATE_SLOT(d, void appletDestroyed(Plasma::Applet*))
        Q_PRIVATE_SLOT(d, void appletAppearAnimationComplete())
        Q_PRIVATE_SLOT(d, void triggerShowAddWidgets())
        Q_PRIVATE_SLOT(d, void handleDisappeared(AppletHandle *handle))
        Q_PRIVATE_SLOT(d, void positionToolBox())
        Q_PRIVATE_SLOT(d, void zoomIn())
        Q_PRIVATE_SLOT(d, void zoomOut())
        Q_PRIVATE_SLOT(d, void requestConfiguration())
        Q_PRIVATE_SLOT(d, void updateToolBoxVisibility())
        Q_PRIVATE_SLOT(d, void showDropZoneDelayed())
        Q_PRIVATE_SLOT(d, void remoteAppletReady(Plasma::AccessAppletJob *))
        /**
        * This slot is called when the 'stat' after a job event has finished.
        */
        Q_PRIVATE_SLOT(d, void mimeTypeRetrieved(KIO::Job *, const QString &))
        Q_PRIVATE_SLOT(d, void dropJobResult(KJob *))

        friend class Applet;
        friend class AppletPrivate;
        friend class CoronaPrivate;
        friend class ContainmentPrivate;
        friend class ContainmentActions;
        friend class PopupApplet;
        ContainmentPrivate *const d;
};

} // Plasma namespace

#endif // multiple inclusion guard
