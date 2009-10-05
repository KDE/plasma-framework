/*
 *   Copyright 2007 Aaron Seigo <aseigo@kde.org>
 *   Copyright 2007 Matt Broadstone <mbroadst@gmail.com>
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

#ifndef PLASMA_CORONA_H
#define PLASMA_CORONA_H

#include <QtGui/QGraphicsScene>

#include <plasma/applet.h>
#include <plasma/containment.h>
#include <plasma/plasma.h>
#include <plasma/plasma_export.h>

class QGraphicsGridLayout;
class QAction;

class KAction;

namespace Plasma
{

class CoronaPrivate;
class ContainmentActionsPluginsConfig;

/**
 * @class Corona plasma/corona.h <Plasma/Corona>
 *
 * @short A QGraphicsScene for Plasma::Applets
 */
class PLASMA_EXPORT Corona : public QGraphicsScene
{
    Q_OBJECT

//typedef QHash<QString, QList<Plasma::Applet*> > layouts;

public:
    explicit Corona(QObject * parent = 0);
    ~Corona();

    /**
     *  Sets the mimetype of Drag/Drop items. Default is
     *  text/x-plasmoidservicename
     */
    void setAppletMimeType(const QString &mimetype);

    /**
     * The current mime type of Drag/Drop items.
     */
    QString appletMimeType();

    /**
     * @return all containments on this Corona
     */
    QList<Containment*> containments() const;

    /**
     * Clear the Corona from all applets.
     */
    void clearContainments();

    /**
     * Returns the config file used to store the configuration for this Corona
     */
    KSharedConfig::Ptr config() const;

    /**
     * Adds a Containment to the Corona
     *
     * @param name the plugin name for the containment, as given by
     *        KPluginInfo::pluginName(). If an empty string is passed in, the defalt
     *        containment plugin will be used (usually DesktopContainment). If the
     *        string literal "null" is passed in, then no plugin will be loaded and
     *        a simple Containment object will be created instead.
     * @param args argument list to pass to the containment
     *
     * @return a pointer to the containment on success, or 0 on failure
     */
    Containment *addContainment(const QString &name, const QVariantList &args = QVariantList());

    /**
     * Returns the Containment, if any, for a given physical screen
     *
     * @param screen number of the physical screen to locate
     * @param desktop the virtual desktop) to locate; if < 0 then it will
     *        simply return the first Containment associated with screen
     */
    Containment *containmentForScreen(int screen, int desktop = -1) const;

    /**
     * Adds a widget in the topleft quadrant in the scene. Widgets in the topleft quadrant are
     * normally never shown unless you specifically aim a view at it, which makes it ideal for
     * toplevel views etc.
     * @param widget the widget to add.
     */
    void addOffscreenWidget(QGraphicsWidget *widget);

    /**
     * Removes a widget from the topleft quadrant in the scene.
     * @param widget the widget to remove.
     */
    void removeOffscreenWidget(QGraphicsWidget *widget);

    /**
     * @return the list of all offscreen widgets
     * @since 4.3
     */
    QList <QGraphicsWidget *> offscreenWidgets() const;

    /**
     * Returns the number of screens available to plasma.
     * Subclasses should override this method as the default
     * implementation returns a meaningless value.
     */
    virtual int numScreens() const;

    /**
     * Returns the geometry of a given screen.
     * Valid screen ids are 0 to numScreen()-1, or -1 for the full desktop geometry.
     * Subclasses should override this method as the default
     * implementation returns a meaningless value.
     */
    virtual QRect screenGeometry(int id) const;

    /**
     * Returns the available region for a given screen.
     * The available region excludes panels and similar windows.
     * Valid screen ids are 0 to numScreens()-1.
     * By default this method returns a rectangular region
     * equal to screenGeometry(id); subclasses that need another
     * behavior should override this method.
     */
    virtual QRegion availableScreenRegion(int id) const;

    /**
    * Recommended position for a popup window like a menu or a tooltip
    * given its size
    * @param item the item that the popup should appear adjacent to (an applet, say)
    * @param size size of the popup
    * @returns reccomended position
    */
    QPoint popupPosition(const QGraphicsItem *item, const QSize &size);

    /**
     * @since 4.4
    * Recommended position for a popup window like a menu or a tooltip
    * given its size
    * @param item the item that the popup should appear adjacent to (an applet, say)
    * @param size size of the popup
    * @param alignment alignment of the popup, valid flags are Qt::AlignLeft, Qt::AlignRight and Qt::AlignCenter
    * @returns reccomended position
    */
    QPoint popupPosition(const QGraphicsItem *item, const QSize &size, Qt::AlignmentFlag alignment);

    /**
     * This method is useful in order to retrieve the list of available
     * screen edges for panel type containments.
     * @param screen the id of the screen to look for free edges.
     * @returns a list of free edges not filled with panel type containments.
     */
    QList<Plasma::Location> freeEdges(int screen) const;

    /**
     * Returns the QAction with the given name from our collection
     */
    QAction *action(QString name) const;

    /**
     * Adds the action to our collection under the given name
     */
    void addAction(QString name, QAction *action);

    /**
     * Returns all the actions in our collection
     */
    QList<QAction*> actions() const;

    /**
     * convenience function - enables or disables an action by name
     *
     * @param name the name of the action in our collection
     * @param enable true to enable, false to disable
     */
    void enableAction(const QString &name, bool enable);

    /**
     * @since 4.3
     * Updates keyboard shortcuts for all the corona's actions.
     * If you've added actions to the corona you'll need to
     * call this for them to be configurable.
     */
    void updateShortcuts();

    /**
     * @since 4.3
     * Adds a set of actions to the shortcut config dialog.
     * don't use this on actions in the corona's own actioncollection,
     * those are handled automatically. this is for stuff outside of that.
     */
    void addShortcuts(KActionCollection *newShortcuts);

    /**
     * @since 4.3
     * Creates an action in our collection under the given name
     * @return the new action
     * FIXME I'm wrapping so much of kactioncollection API now, maybe I should just expose the
     * collection itself :P
     */
    KAction* addAction(QString name);

    /**
     * @since 4.4
     * Sets the default containmentactions plugins for the given containment type
     */
    void setContainmentActionsDefaults(Containment::Type containmentType, const ContainmentActionsPluginsConfig &config);

    /**
     * @since 4.4
     * Returns the default containmentactions plugins for the given containment type
     */
    ContainmentActionsPluginsConfig containmentActionsDefaults(Containment::Type containmentType);

public Q_SLOTS:
    /**
     * Initializes the layout from a config file. This will first clear any existing
     * Containments, load a layout from the requested configuration file, request the
     * default layout if needed and update immutability.
     *
     * @param config the name of the config file to load from,
     *               or the default config file if QString()
     */
    void initializeLayout(const QString &config = QString());

    /**
     * Load applet layout from a config file. The results will be added to the
     * current set of Containments.
     *
     * @param config the name of the config file to load from,
     *               or the default config file if QString()
     */
    void loadLayout(const QString &config = QString());

    /**
     * Save applets layout to file
     * @arg config the file to save to, or the default config file if QString()
     */
    void saveLayout(const QString &config = QString()) const;

    /**
     * @return The type of immutability of this Corona
     */
    ImmutabilityType immutability() const;

    /**
     * Sets the immutability type for this Corona (not immutable,
     * user immutable or system immutable)
     * @arg immutable the new immutability type of this applet
     */
    void setImmutability(const ImmutabilityType immutable);

    /**
     * Schedules a flush-to-disk synchronization of the configuration state
     * at the next convenient moment.
     */
    void requestConfigSync();

    /**
     * Schedules a time sensitive flush-to-disk synchronization of the
     * configuration state. Since this method does not provide any sort of
     * event compression, it should only be used when an *immediate* disk
     * sync is *absolutely* required. Otherwise, use @see requestConfigSync()
     * which does do event compression.
     */
    void requireConfigSync();

Q_SIGNALS:
    /**
     * This signal indicates a new containment has been added to
     * the Corona
     */
    void containmentAdded(Plasma::Containment *containment);

    /**
     * This signal indicates that a containment has been newly
     * associated (or dissociated) with a physical screen.
     *
     * @param wasScreen the screen it was associated with
     * @param isScreen the screen it is now associated with
     * @param containment the containment switching screens
     */
    void screenOwnerChanged(int wasScreen, int isScreen, Plasma::Containment *containment);

    /**
     * This signal indicates that an application launch, window
     * creation or window focus event was triggered. This is used, for instance,
     * to ensure that the Dashboard view in Plasma hides when such an event is
     * triggered by an item it is displaying.
     */
    void releaseVisualFocus();

    /**
     * This signal indicates that the configuration file was flushed to disc.
     */
    void configSynced();

    /**
     * This signal inicates that a change in available screen goemetry occurred.
     */
    void availableScreenRegionChanged();

    /**
     * emitted when immutability changes.
     * this is for use by things that don't get contraints events, like plasmaapp.
     * it's NOT for containments or applets or any of the other stuff on the scene.
     * if your code's not in shells/ it probably shouldn't be using it.
     */
    void immutabilityChanged(Plasma::ImmutabilityType immutability);

    /**
     * @since 4.3
     * emitted when the user changes keyboard shortcut settings
     * connect to this if you've put some extra shortcuts in your app
     * that are NOT in corona's actioncollection.
     * if your code's not in shells/ it probably shouldn't be using this function.
     * @see addShortcuts
     */
    void shortcutsChanged();

protected:
    /**
     * Loads the default (system wide) layout for this user
     **/
    virtual void loadDefaultLayout();

    /**
     * Loads a containment with delayed initialization, primarily useful
     * for implementations of loadDefaultLayout. The caller is responsible
     * for all initializating, saving and notification of a new containment.
     *
     * @param name the plugin name for the containment, as given by
     *        KPluginInfo::pluginName(). If an empty string is passed in, the defalt
     *        containment plugin will be used (usually DesktopContainment). If the
     *        string literal "null" is passed in, then no plugin will be loaded and
     *        a simple Containment object will be created instead.
     * @param args argument list to pass to the containment
     *
     * @return a pointer to the containment on success, or 0 on failure
     **/
    Containment *addContainmentDelayed(const QString &name,
                                       const QVariantList &args = QVariantList());

    //Reimplemented from QGraphicsScene
    void dragEnterEvent(QGraphicsSceneDragDropEvent *event);
    void dragLeaveEvent(QGraphicsSceneDragDropEvent *event);
    void dragMoveEvent(QGraphicsSceneDragDropEvent *event);

private:
    CoronaPrivate *const d;

    Q_PRIVATE_SLOT(d, void containmentDestroyed(QObject*))
    Q_PRIVATE_SLOT(d, void offscreenWidgetDestroyed(QObject *))
    Q_PRIVATE_SLOT(d, void syncConfig())
    Q_PRIVATE_SLOT(d, void toggleImmutability())
    Q_PRIVATE_SLOT(d, void showShortcutConfig())

    friend class CoronaPrivate;
};

} // namespace Plasma

#endif

