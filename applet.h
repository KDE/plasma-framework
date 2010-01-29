/*
 *   Copyright 2006-2007 by Aaron Seigo <aseigo@kde.org>
 *   Copyright 2007 by Riccardo Iaconelli <riccardo@kde.org>
 *   Copyright 2008 by Ménard Alexis <darktears31@gmail.com>

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

#ifndef PLASMA_APPLET_H
#define PLASMA_APPLET_H

#include <QtGui/QGraphicsItem>
#include <QtGui/QGraphicsWidget>
#include <QtGui/QIcon>

#include <kconfiggroup.h>
#include <kgenericfactory.h>
#include <kplugininfo.h>
#include <kshortcut.h>

#include <plasma/configloader.h>
#include <plasma/packagestructure.h>
#include <plasma/plasma.h>
#include <plasma/animator.h>
#include <plasma/version.h>
#include <plasma/framesvg.h>

class QWidget;

class KConfigDialog;
class QGraphicsView;
class KActionCollection;

namespace Plasma
{

class AppletPrivate;
class Containment;
class Context;
class DataEngine;
class Extender;
class ExtenderItem;
class Package;


/**
 * @class Applet plasma/applet.h <Plasma/Applet>
 *
 * @short The base Applet class
 *
 * Applet provides several important roles for add-ons widgets in Plasma.
 *
 * First, it is the base class for the plugin system and therefore is the
 * interface to applets for host applications. It also handles the life time
 * management of data engines (e.g. all data engines accessed via
 * Applet::dataEngine(const QString&) are properly deref'd on Applet
 * destruction), background painting (allowing for consistent and complex
 * look and feel in just one line of code for applets), loading and starting
 * of scripting support for each applet, providing access to the associated
 * plasmoid package (if any) and access to configuration data.
 *
 * See techbase.kde.org for tutorials on writing Applets using this class.
 */
class PLASMA_EXPORT Applet : public QGraphicsWidget
{
    Q_OBJECT
    Q_PROPERTY(bool hasConfigurationInterface READ hasConfigurationInterface)
    Q_PROPERTY(QString name READ name)
    Q_PROPERTY(QString category READ category)
    Q_PROPERTY(ImmutabilityType immutability READ immutability WRITE setImmutability)
    Q_PROPERTY(bool hasFailedToLaunch READ hasFailedToLaunch WRITE setFailedToLaunch)
    Q_PROPERTY(bool isBusy READ isBusy WRITE setBusy) //KDE5: remove
    Q_PROPERTY(bool busy READ isBusy WRITE setBusy)
    Q_PROPERTY(bool configurationRequired READ configurationRequired WRITE setConfigurationRequired)
    Q_PROPERTY(QRectF geometry READ geometry WRITE setGeometry)
    Q_PROPERTY(bool shouldConserveResources READ shouldConserveResources)
    Q_PROPERTY(uint id READ id)

    public:
        typedef QList<Applet*> List;
        typedef QHash<QString, Applet*> Dict;

        /**
         * Description on how draw a background for the applet
         */
        enum BackgroundHint {
            NoBackground = 0,         /**< Not drawing a background under the
                                          applet, the applet has its own implementation */
            StandardBackground = 1,   /**< The standard background from the theme is drawn */
            TranslucentBackground = 2, /**< An alternate version of the background is drawn,
                                          usually more translucent */
            DefaultBackground = StandardBackground /**< Default settings:
                                          both standard background */
        };
        Q_DECLARE_FLAGS(BackgroundHints, BackgroundHint)

        ~Applet();

        /**
         * @return a package structure representing a Theme
         */
        static PackageStructure::Ptr packageStructure();

        /**
         * @return the id of this applet
         */
        uint id() const;

        /**
        * Returns the KConfigGroup to access the applets configuration.
        *
        * This config object will write to an instance
        * specific config file named \<appletname\>\<instanceid\>rc
        * in the Plasma appdata directory.
        **/
        KConfigGroup config() const;

        /**
         * Returns a config group with the name provided. This ensures
         * that the group name is properly namespaced to avoid collision
         * with other applets that may be sharing this config file
         *
         * @param group the name of the group to access
         **/
        KConfigGroup config(const QString &group) const;

        /**
         * Saves state information about this applet that will
         * be accessed when next instantiated in the restore(KConfigGroup&) method.
         *
         * This method does not need to be reimplmented by Applet
         * subclasses, but can be useful for Applet specializations
         * (such as Containment) to do so.
         *
         * Applet subclasses may instead want to reimplement saveState().
         **/
        virtual void save(KConfigGroup &group) const;

        /**
         * Restores state information about this applet saved previously
         * in save(KConfigGroup&).
         *
         * This method does not need to be reimplmented by Applet
         * subclasses, but can be useful for Applet specializations
         * (such as Containment) to do so.
         **/
        virtual void restore(KConfigGroup &group);

        /**
         * Returns a KConfigGroup object to be shared by all applets of this
         * type.
         *
         * This config object will write to an applet-specific config object
         * named plasma_\<appletname\>rc in the local config directory.
         */
        KConfigGroup globalConfig() const;

        /**
         * Returns the config skeleton object from this applet's package,
         * if any.
         *
         * @return config skeleton object, or 0 if none
         **/
        ConfigLoader *configScheme() const;

        /**
         * Loads the given DataEngine
         *
         * Tries to load the data engine given by @p name.  Each engine is
         * only loaded once, and that instance is re-used on all subsequent
         * requests.
         *
         * If the data engine was not found, an invalid data engine is returned
         * (see DataEngine::isValid()).
         *
         * Note that you should <em>not</em> delete the returned engine.
         *
         * @param name Name of the data engine to load
         * @return pointer to the data engine if it was loaded,
         *         or an invalid data engine if the requested engine
         *         could not be loaded
         */
        Q_INVOKABLE DataEngine *dataEngine(const QString &name) const;

        /**
         * Accessor for the associated Package object if any.
         * Generally, only Plasmoids come in a Package.
         *
         * @return the Package object, or 0 if none
         **/
        const Package *package() const;

        /**
         * Returns the view this widget is visible on, or 0 if none can be found.
         * @warning do NOT assume this will always return a view!
         * a null view probably means that either plasma isn't finished loading, or your applet is
         * on an activity that's not being shown anywhere.
         */
        QGraphicsView *view() const;

        /**
         * Maps a QRect from a view's coordinates to local coordinates.
         * @param view the view from which rect should be mapped
         * @param rect the rect to be mapped
         */
        QRectF mapFromView(const QGraphicsView *view, const QRect &rect) const;

        /**
         * Maps a QRectF from local coordinates to a view's coordinates.
         * @param view the view to which rect should be mapped
         * @param rect the rect to be mapped
         */
        QRect mapToView(const QGraphicsView *view, const QRectF &rect) const;

        /**
         * Reccomended position for a popup window like a menu or a tooltip
         * given its size
         * @param s size of the popup
         * @returns reccomended position
         */
        QPoint popupPosition(const QSize &s) const;

        /**
         * @since 4.4
         * Reccomended position for a popup window like a menu or a tooltip
         * given its size
         * @param s size of the popup
         * @param alignment alignment of the popup, valid flags are Qt::AlignLeft, Qt::AlignRight and Qt::AlignCenter
         * @returns reccomended position
         */
        QPoint popupPosition(const QSize &s, Qt::AlignmentFlag alignment) const;

        /**
         * Called when any of the geometry constraints have been updated.
         * This method calls constraintsEvent, which may be reimplemented,
         * once the Applet has been prepared for updating the constraints.
         *
         * @param constraints the type of constraints that were updated
         */
        void updateConstraints(Plasma::Constraints constraints = Plasma::AllConstraints);

        /**
         * Returns the current form factor the applet is being displayed in.
         *
         * @see Plasma::FormFactor
         */
        virtual FormFactor formFactor() const;

        /**
         * Returns the location of the scene which is displaying applet.
         *
         * @see Plasma::Location
         */
        virtual Location location() const;

        /**
         * Returns the workspace context which the applet is operating in
         */
        Context *context() const;

        /**
         * @return the preferred aspect ratio mode for placement and resizing
         */
        Plasma::AspectRatioMode aspectRatioMode() const;

        /**
         * Sets the preferred aspect ratio mode for placement and resizing
         */
        void setAspectRatioMode(Plasma::AspectRatioMode);

        /**
         * Returns a list of all known applets.
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
        static KPluginInfo::List listAppletInfo(const QString &category = QString(),
                                              const QString &parentApp = QString());

        /**
         * Returns a list of all known applets associated with a certain mimetype.
         *
         * @return list of applets
         **/
        static KPluginInfo::List listAppletInfoForMimetype(const QString &mimetype);

        /**
         * Returns a list of all known applets associated with a certain URL.
         *
         * @since 4.4
         * @return list of applets
         **/
        static KPluginInfo::List listAppletInfoForUrl(const QUrl &url);

        /**
         * Returns a list of all the categories used by installed applets.
         *
         * @param parentApp the application to filter applets on. Uses the
         *                  X-KDE-ParentApp entry (if any) in the plugin info.
         *                  The default value of QString() will result in a
         *                  list containing only applets not specifically
         *                  registered to an application.
         * @return list of categories
         * @param visibleOnly true if it should only return applets that are marked as visible
         */
        static QStringList listCategories(const QString &parentApp = QString(),
                                          bool visibleOnly = true);

        /**
         * Sets the list of custom categories that are used in addition to the default
         * set of categories known to libplasma for Applets.
         * @arg categories a list of categories
         * @since 4.3
         */
        void setCustomCategories(const QStringList &categories);

        /**
         * @return the list of custom categories known to libplasma
         * @since 4.3
         */
        QStringList customCategories();

        /**
         * Attempts to load an applet from a package
         *
         * Returns a pointer to the applet if successful.
         * The caller takes responsibility for the applet, including
         * deleting it when no longer needed.
         *
         * @param path the path to the package
         * @param appletId unique ID to assign the applet, or zero to have one
         *        assigned automatically.
         * @param args to send the applet extra arguments
         * @return a pointer to the loaded applet, or 0 on load failure
         * @since 4.3
         **/
        static Applet *loadPlasmoid(const QString &path, uint appletId = 0,
                                    const QVariantList &args = QVariantList());

        /**
         * Attempts to load an applet
         *
         * Returns a pointer to the applet if successful.
         * The caller takes responsibility for the applet, including
         * deleting it when no longer needed.
         *
         * @param name the plugin name, as returned by KPluginInfo::pluginName()
         * @param appletId unique ID to assign the applet, or zero to have one
         *        assigned automatically.
         * @param args to send the applet extra arguments
         * @return a pointer to the loaded applet, or 0 on load failure
         **/
        static Applet *load(const QString &name, uint appletId = 0,
                            const QVariantList &args = QVariantList());

        /**
         * Attempts to load an applet
         *
         * Returns a pointer to the applet if successful.
         * The caller takes responsibility for the applet, including
         * deleting it when no longer needed.
         *
         * @param info KPluginInfo object for the desired applet
         * @param appletId unique ID to assign the applet, or zero to have one
         *        assigned automatically.
         * @param args to send the applet extra arguments
         * @return a pointer to the loaded applet, or 0 on load failure
         **/
        static Applet *load(const KPluginInfo &info, uint appletId = 0,
                            const QVariantList &args = QVariantList());

        /**
         * Get the category of the given applet
         *
         * @param applet a KPluginInfo object for the applet
         */
        static QString category(const KPluginInfo &applet);

        /**
         * Get the category of the given applet
         *
         * @param appletName the name of the applet
         */
        static QString category(const QString &appletName);

        /**
         * This method is called when the interface should be painted.
         *
         * @param painter the QPainter to use to do the paintiner
         * @param option the style options object
         * @param contentsRect the rect to paint within; automatically adjusted for
         *                     the background, if any
         **/
        virtual void paintInterface(QPainter *painter,
                                    const QStyleOptionGraphicsItem *option,
                                    const QRect &contentsRect);

        /**
         * Returns the user-visible name for the applet, as specified in the
         * .desktop file.
         *
         * @return the user-visible name for the applet.
         **/
        QString name() const;

        /**
         * @return the font currently set for this widget
         **/
        QFont font() const;

        /**
         * Returns the plugin name for the applet
         */
        QString pluginName() const;

        /**
         * Whether the applet should conserve resources. If true, try to avoid doing stuff which
         * is computationally heavy. Try to conserve power and resources.
         *
         * @return true if it should conserve resources, false if it does not.
         */
        bool shouldConserveResources() const;

        /**
         * Returns the icon related to this applet
         **/
        QString icon() const;

        /**
         * Returns the category the applet is in, as specified in the
         * .desktop file.
         */
        QString category() const;

        /**
         * @return The type of immutability of this applet
         */
        ImmutabilityType immutability() const;

        void paintWindowFrame(QPainter *painter,
                              const QStyleOptionGraphicsItem *option, QWidget *widget);

        /**
         * If for some reason, the applet fails to get up on its feet (the
         * library couldn't be loaded, necessary hardware support wasn't found,
         * etc..) this method returns true
         **/
        bool hasFailedToLaunch() const;

        /**
         * @return true if the applet is busy and is showing an indicator widget for that
         */
        bool isBusy() const;

        /**
         * @return true if the applet currently needs to be configured,
         *         otherwise, false
         */
        bool configurationRequired() const;

        /**
         * @return true if this plasmoid provides a GUI configuration
         **/
        bool hasConfigurationInterface() const;

        /**
         * Returns a list of context-related QAction instances.
         *
         * This is used e.g. within the \a DesktopView to display a
         * contextmenu.
         *
         * @return A list of actions. The default implementation returns an
         *         empty list.
         **/
        virtual QList<QAction*> contextualActions();

        /**
         * Returns the QAction with the given name from our collection
         */
        QAction *action(QString name) const;

        /**
         * Adds the action to our collection under the given name
         */
        void addAction(QString name, QAction *action);

        /**
         * Sets the BackgroundHints for this applet @see BackgroundHint
         *
         * @param hints the BackgroundHint combination for this applet
         */
        void setBackgroundHints(const BackgroundHints hints);

        /**
         * @return BackgroundHints flags combination telling if the standard background is shown
         *         and if it has a drop shadow
         */
        BackgroundHints backgroundHints() const;

        /**
         * @return true if this Applet is currently being used as a Containment, false otherwise
         */
        bool isContainment() const;

        /**
         * This method returns screen coordinates for the widget; this method can be somewhat
         * expensive and should ONLY be called when screen coordinates are required. For
         * example when positioning top level widgets on top of the view to create the
         * appearance of unit. This should NOT be used for popups (@see popupPosition) or
         * for normal widget use (use Plasma:: widgets or QGraphicsProxyWidget instead).
         *
         * @return a rect of the applet in screen coordinates.
         */
        QRect screenRect() const;

        /**
         * Reimplemented from QGraphicsItem
         **/
        int type() const;
        enum {
            Type = Plasma::AppletType
        };

        /**
         * @return the Containment, if any, this applet belongs to
         **/
        Containment *containment() const;

        /**
         * Sets the global shorcut to associate with this widget.
         */
        void setGlobalShortcut(const KShortcut &shortcut);

        /**
         * @return the global shortcut associated with this wiget, or
         * an empty shortcut if no global shortcut is associated.
         */
        KShortcut globalShortcut() const;

        /**
         * @return true is there is a popup assoiated with this Applet
         * showing, such as the dialog of a PopupApplet. May be reimplemented
         * for custom popup implementations.
         */
        virtual bool isPopupShowing() const;

        /**
         * associate actions with this widget, including ones added after this call.
         * needed to make keyboard shortcuts work.
         */
        virtual void addAssociatedWidget(QWidget *widget);

        /**
         * un-associate actions from this widget, including ones added after this call.
         * needed to make keyboard shortcuts work.
         */
        virtual void removeAssociatedWidget(QWidget *widget);

        /**
         * Gets called when an extender item has to be initialized after a plasma restart. If you
         * create ExtenderItems in your applet, you should implement this function to again create
         * the widget that should be shown in this extender item. This function might look something
         * like this:
         *
         * @code
         * SuperCoolWidget *widget = new SuperCoolWidget();
         * dataEngine("engine")->connectSource(item->config("dataSourceName"), widget);
         * item->setWidget(widget);
         * @endcode
         *
         * You can also add one or more custom qactions to this extender item in this function.
         *
         * Note that by default, not all ExtenderItems are persistent. Only items that are detached,
         * will have their configuration stored when plasma exits.
         */
        virtual void initExtenderItem(ExtenderItem *item);

        /**
         * @param parent the QGraphicsItem this applet is parented to
         * @param serviceId the name of the .desktop file containing the
         *      information about the widget
         * @param appletId a unique id used to differentiate between multiple
         *      instances of the same Applet type
         */
        explicit Applet(QGraphicsItem *parent = 0,
                        const QString &serviceId = QString(),
                        uint appletId = 0);

        /**
         * @param parent the QGraphicsItem this applet is parented to
         * @param serviceId the name of the .desktop file containing the
         *      information about the widget
         * @param appletId a unique id used to differentiate between multiple
         *      instances of the same Applet type
         * @param args  a list of strings containing two entries: the service id
         *      and the applet id
         * @since 4.3
         */
        explicit Applet(QGraphicsItem *parent,
                        const QString &serviceId,
                        uint appletId,
                        const QVariantList &args);


        /**
         * @return true if destroy() was called; useful for Applets which should avoid
         * certain tasks if they are about to be deleted permanently
         */
        bool destroyed() const;

        /**
         * Reimplement this method so provide a configuration interface,
         * parented to the supplied widget. Ownership of the widgets is passed
         * to the parent widget.
         *
         * @param parent the dialog which is the parent of the configuration
         *               widgets
         */
        virtual void createConfigurationInterface(KConfigDialog *parent);

        /**
         * Returns true if the applet is allowed to perform functions covered by the given constraint
         * eg. hasAuthorization("FileDialog") returns true if applets are allowed to show filedialogs.
         * @since 4.3
         */
        bool hasAuthorization(const QString &constraint) const;

        /**
         * Sets an application associated to this applet, that will be
         * regarded as a full view of what is represented in the applet
         *
         * @param string the name of the application. it can be
         *      \li a name understood by KService::serviceByDesktopName
         *        (e.g. "konqueror")
         *      \li a command in $PATH
         *      \li or an absolute path to an executable
         * @since 4.4
         */
        void setAssociatedApplication(const QString &string);

        /**
         * Sets a list of urls associated to this application,
         * they will be used as parameters for the associated application
         * @see setAssociatedApplication()
         *
         * @param urls
         */
        void setAssociatedApplicationUrls(const KUrl::List &urls);

        /**
         * @return the application associated to this applet
         * @since 4.4
         */
        QString associatedApplication() const;

        /**
         * @return the urls associated to this applet
         * @since 4.4
         */
        KUrl::List associatedApplicationUrls() const;

        /**
         * @return true if the applet has a valid associated application or urls
         * @since 4.4
         */
        bool hasValidAssociatedApplication() const;

    Q_SIGNALS:
        /**
         * This signal indicates that an application launch, window
         * creation or window focus event was triggered. This is used, for instance,
         * to ensure that the Dashboard view in Plasma hides when such an event is
         * triggered by an item it is displaying.
         */
        void releaseVisualFocus();

        /**
         * Emitted whenever the applet makes a geometry change, so that views
         * can coordinate themselves with these changes if they desire.
         */
        void geometryChanged();

        /**
         * Emitted when the user completes a transformation of the applet.
         */
        void appletTransformedByUser();

        /**
         * Emitted when the applet changes its own geometry or transform.
         */
        void appletTransformedItself();

        /**
         * Emitted by Applet subclasses when they change a sizeHint and wants to announce the change
         */
        void sizeHintChanged(Qt::SizeHint which);

        /**
         * Emitted when an applet has changed values in its configuration
         * and wishes for them to be saved at the next save point. As this implies
         * disk activity, this signal should be used with care.
         *
         * @note This does not need to be emitted from saveState by individual
         * applets.
         */
        void configNeedsSaving();

        /**
         * Emitted when activation is requested due to, for example, a global
         * keyboard shortcut. By default the wiget is given focus.
         */
        void activate();

        /**
         * Emitted when the user clicked on a button of the message overlay
         *
         * Since the signal passes the type MessageButton without specifying
         * the Plasma namespace and since automoc is pedantic if you want to
         * use this signal outside the Plasma namespace you have to declare
         * "typedef Plasma::MessageButton MessageButton" and use the typedef
         * in your slot and in the connect.
         * @see showMessage
         * @see Plasma::MessageButton
         * @since 4.3
         */
        void messageButtonPressed(const MessageButton button);

        /**
         * Emitted when the applet is deleted
         */
        void appletDestroyed(Plasma::Applet *applet);

        /**
         * Emitted when the applet status changes
         * @since 4.4
         */
        void newStatus(Plasma::ItemStatus status);

        /**
         * Emitted when an ExtenderItem in a scripting applet needs to be initialized
         */
        void extenderItemRestored(Plasma::ExtenderItem *item);

        /**
         * Emitted when the immutability changes
         * @since 4.4
         */
        void immutabilityChanged(Plasma::ImmutabilityType immutable);

    public Q_SLOTS:
        /**
         * Sets the immutability type for this applet (not immutable,
         * user immutable or system immutable)
         * @arg immutable the new immutability type of this applet
         */
        void setImmutability(const ImmutabilityType immutable);

        /**
         * Destroys the applet; it will be removed nicely and deleted.
         * Its configuration will also be deleted.
         */
        virtual void destroy();

        /**
         * Lets the user interact with the plasmoid options.
         * Called when the user selects the configure entry
         * from the context menu.
         *
         * Unless there is good reason for overriding this method,
         * Applet subclasses should actually override createConfigurationInterface
         * instead. A good example of when this isn't plausible is
         * when using a dialog prepared by another library, such
         * as KPropertiesDialog from libkfile.
         */
        virtual void showConfigurationInterface();

        /**
         * Causes this applet to raise above all other applets.
         */
        void raise();

        /**
         * Causes this applet to lower below all the other applets.
         */
        void lower();

        /**
         * Sends all pending contraints updates to the applet. Will usually
         * be called automatically, but can also be called manually if needed.
         */
        void flushPendingConstraintsEvents();

        /**
         * This method is called once the applet is loaded and added to a Corona.
         * If the applet requires a QGraphicsScene or has an particularly intensive
         * set of initialization routines to go through, consider implementing it
         * in this method instead of the constructor.
         *
         * Note: paintInterface may get called before init() depending on initialization
         * order. Painting is managed by the canvas (QGraphisScene), and may schedule a
         * paint event prior to init() being called.
         **/
        virtual void init();

        /**
         * Called when applet configuration values have changed.
         */
        virtual void configChanged();

        /**
         * Shows a busy indicator that overlays the applet
         * @param busy show or hide the busy indicator
         */
        void setBusy(bool busy);

        /**
         * @return the list of arguments which the applet was called with
         * @since KDE4.3
         */
        QVariantList startupArguments() const;

        /**
         * @return the status of the applet
         * @since 4.4
         */
        ItemStatus status() const;

        /**
         * sets the status for this applet
         * @since 4.4
         */
        void setStatus(const ItemStatus stat);

        /**
         * Publishes and optionally announces this applet on the network for remote access.
         * @param methods the methods to use for announcing this applet.
         * @param resourceName the name under which this applet will be published (has to be unique
         * for each machine)
         */
        void publish(Plasma::AnnouncementMethods methods, const QString &resourceName);

        void unpublish();

        bool isPublished() const;

        /**
         * Open the application associated to this applet, if it's not set
         * but some urls are, open those urls with the proper application
         * for their mimetype
         * @see setAssociatedApplication()
         * @see setAssociatedApplicationUrls()
         * @since 4.4
         */
        void runAssociatedApplication();

    protected:
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
        Applet(QObject *parent, const QVariantList &args);

        /**
         * Call this method when the applet fails to launch properly. An
         * optional reason can be provided.
         *
         * Not that all children items will be deleted when this method is
         * called. If you have pointers to these items, you will need to
         * reset them after calling this method.
         *
         * @param failed true when the applet failed, false when it succeeded
         * @param reason an optional reason to show the user why the applet
         *               failed to launch
         **/
        void setFailedToLaunch(bool failed, const QString &reason = QString());

        /**
         * When called, the Applet should write any information needed as part
         * of the Applet's running state to the configuration object in config()
         * and/or globalConfig().
         *
         * Applets that always sync their settings/state with the config
         * objects when these settings/states change do not need to reimplement
         * this method.
         **/
        virtual void saveState(KConfigGroup &config) const;

        /**
         * Sets whether or not this applet provides a user interface for
         * configuring the applet.
         *
         * It defaults to false, and if true is passed in you should
         * also reimplement createConfigurationInterface()
         *
         * @param hasInterface whether or not there is a user interface available
         **/
        void setHasConfigurationInterface(bool hasInterface);

        /**
         * When the applet needs to be configured before being usable, this
         * method can be called to show a standard interface prompting the user
         * to configure the applet
         *
         * @param needsConfiguring true if the applet needs to be configured,
         *                         or false if it doesn't
         * @param reason a translated message for the user explaining that the
         *               applet needs configuring; this should note what needs
         *               to be configured
         */
        void setConfigurationRequired(bool needsConfiguring, const QString &reason = QString());

        /**
         * Shows a message as an overlay of the applet: the message has an
         * icon, text and (optional) buttons
         *
         * @param icon the icon that will be shown
         * @param message the message string that will be shown.
         *                If the message is empty nothng will be shown
         *                and if there was a message already it will be hidden
         * @param buttons an OR combination of all the buttons needed
         *
         * @see Plasma::MessageButtons
         * @see messageButtonPressed
         * @since 4.3
         */
        void showMessage(const QIcon &icon, const QString &message, const MessageButtons buttons);

        /**
         * Called when any of the geometry constraints have been updated.
         *
         * This is always called prior to painting and should be used as an
         * opportunity to layout the widget, calculate sizings, etc.
         *
         * Do not call update() from this method; an update() will be triggered
         * at the appropriate time for the applet.
         *
         * @param constraints the type of constraints that were updated
         * @property constraint
         */
        virtual void constraintsEvent(Plasma::Constraints constraints);

        /**
         * Register the widgets that manage mouse clicks but you still want
         * to be able to drag the applet around when holding the mouse pointer
         * on that widget.
         *
         * Calling this results in an eventFilter being places on the widget.
         *
         * @param item the item to watch for mouse move
         */
        void registerAsDragHandle(QGraphicsItem *item);

        /**
         * Unregister a widget registered with registerAsDragHandle.
         *
         * @param item the item to unregister
         */
        void unregisterAsDragHandle(QGraphicsItem *item);

        /**
         * @param item the item to look for if it is registered or not
         * @return true if it is registered, false otherwise
         */
        bool isRegisteredAsDragHandle(QGraphicsItem *item);

        /**
         * @return the extender of this applet.
         */
        Extender *extender() const;

        /**
         * @internal event filter; used for focus watching
         **/
        bool eventFilter(QObject *o, QEvent *e);

        /**
         * @internal scene event filter; used to manage applet dragging
         */
        bool sceneEventFilter (QGraphicsItem *watched, QEvent *event);

        /**
         * @internal manage the mouse movement to drag the applet around
         */
        void mouseMoveEvent(QGraphicsSceneMouseEvent *event);

        /**
         * Reimplemented from QGraphicsItem
         */
        void focusInEvent(QFocusEvent *event);

        /**
         * Reimplemented from QGraphicsItem
         */
        void resizeEvent(QGraphicsSceneResizeEvent *event);

        /**
         * Reimplemented from QGraphicsItem
         */
        QVariant itemChange(GraphicsItemChange change, const QVariant &value);

        /**
         * Reimplemented from QGraphicsItem
         */
        QPainterPath shape() const;

        /**
         * Reimplemented from QGraphicsLayoutItem
         */
        QSizeF sizeHint(Qt::SizeHint which, const QSizeF & constraint = QSizeF()) const;

        /**
         * Reimplemented from QGraphicsLayoutItem
         */
        void hoverEnterEvent(QGraphicsSceneHoverEvent *event);

        /**
         * Reimplemented from QGraphicsLayoutItem
         */
        void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);

        /**
         * Reimplemented from QObject
         */
        void timerEvent (QTimerEvent *event);

    private:
        /**
         * @internal This constructor is to be used with the Package loading system.
         *
         * @param parent a QObject parent; you probably want to pass in 0
         * @param args a list of strings containing two entries: the service id
         *      and the applet id
         * @since 4.3
         */
        Applet(const QString &packagePath, uint appletId, const QVariantList &args);

        Q_PRIVATE_SLOT(d, void setFocus())
        Q_PRIVATE_SLOT(d, void checkImmutability())
        Q_PRIVATE_SLOT(d, void themeChanged())
        Q_PRIVATE_SLOT(d, void appletAnimationComplete())
        Q_PRIVATE_SLOT(d, void selectItemToDestroy())
        Q_PRIVATE_SLOT(d, void updateRect(const QRectF& rect))
        Q_PRIVATE_SLOT(d, void destroyMessageOverlay())
        Q_PRIVATE_SLOT(d, void clearShortcutEditorPtr())
        Q_PRIVATE_SLOT(d, void configDialogFinished())
        Q_PRIVATE_SLOT(d, void updateShortcuts())
        Q_PRIVATE_SLOT(d, void publishCheckboxStateChanged(int state))

        /**
         * Reimplemented from QGraphicsItem
         **/
        void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0);

        AppletPrivate *const d;

        //Corona needs to access setFailedToLaunch and init
        friend class Corona;
        friend class CoronaPrivate;
        friend class Containment;
        friend class ContainmentPrivate;
        friend class AppletScript;
        friend class AppletHandle;
        friend class AppletPrivate;
        friend class AccessAppletJobPrivate;
        friend class PopupApplet;
        friend class PopupAppletPrivate;
        friend class AssociatedApplicationManager;

        friend class Extender;
        friend class ExtenderGroup;
        friend class ExtenderGroupPrivate;
        friend class ExtenderPrivate;
        friend class ExtenderItem;
};

} // Plasma namespace

Q_DECLARE_OPERATORS_FOR_FLAGS(Plasma::Applet::BackgroundHints)

/**
 * Register an applet when it is contained in a loadable module
 */
#define K_EXPORT_PLASMA_APPLET(libname, classname) \
K_PLUGIN_FACTORY(factory, registerPlugin<classname>();) \
K_EXPORT_PLUGIN(factory("plasma_applet_" #libname)) \
K_EXPORT_PLUGIN_VERSION(PLASMA_VERSION)

#endif // multiple inclusion guard
