/*
 *   Copyright 2006-2007 by Aaron Seigo <aseigo@kde.org>
 *   Copyright 2007 by Riccardo Iaconelli <riccardo@kde.org>
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

#ifndef PLASMA_APPLET_H
#define PLASMA_APPLET_H

#include <QtGui/QGraphicsItem>
#include <QtGui/QWidget>

#include <kplugininfo.h>
#include <ksharedconfig.h>
#include <kgenericfactory.h>

#include <plasma/configxml.h>
#include <plasma/plasma.h>
#include <plasma/widgets/widget.h>

namespace Plasma
{

class Containment;
class DataEngine;
class Package;

/**
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
 * See techbase.kde.org for tutorial on writing Applets using this class.
 */
class PLASMA_EXPORT Applet : public Widget
{
    Q_OBJECT
//    Q_PROPERTY( QRectF maxSizeHint READ maxSizeHint )
    Q_PROPERTY( bool hasConfigurationInterface READ hasConfigurationInterface WRITE setHasConfigurationInterface )
    Q_PROPERTY( QString name READ name )
    Q_PROPERTY( QString category READ category )
    Q_PROPERTY( bool immutable READ isImmutable WRITE setImmutable )
    Q_PROPERTY( bool drawStandardBackground READ drawStandardBackground WRITE setDrawStandardBackground )
    Q_PROPERTY( bool failedToLaunch READ failedToLaunch WRITE setFailedToLaunch )
    Q_PROPERTY( bool needsConfiguring READ needsConfiguring WRITE setNeedsConfiguring )
    Q_PROPERTY( QRectF boundingRect READ boundingRect )
    Q_PROPERTY( QRectF geometry READ geometry WRITE setGeometry )
    Q_PROPERTY( bool shouldConserveResources READ shouldConserveResources )

    public:
        typedef QList<Applet*> List;
        typedef QHash<QString, Applet*> Dict;

        /**
         * @arg parent the QGraphicsItem this applet is parented to
         * @arg servideId the name of the .desktop file containing the
         *      information about the widget
         * @arg appletId a unique id used to differentiate between multiple
         *      instances of the same Applet type
         */
        explicit Applet(QGraphicsItem* parent = 0,
                        const QString& serviceId = QString(),
                        uint appletId = 0);

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
        Applet(QObject* parent, const QVariantList& args);

        ~Applet();

        /**
         * This method is called once the applet is loaded and added to a Corona.
         * If the applet requires a QGraphicsScene or has an particularly intensive
         * set of initialization routines to go through, consider implementing it
         * in this method instead of the constructor.
         **/
        virtual void init();

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
         * Saves state information about this applet.
         **/
        void save(KConfigGroup* group) const;

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
        ConfigXml* configXml() const;

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
        Q_INVOKABLE DataEngine* dataEngine(const QString& name) const;

        /**
         * Accessor for the associated Package object if any.
         * Generally, only Plasmoids come in a Package.
         *
         * @return the Package object, or 0 if none
         **/
        const Package* package() const;

        /**
         * Called when any of the geometry constraints have been updated.
         * This method calls constraintsUpdated, which may be reimplemented,
         * once the Applet has been prepared for updating the constraints.
         *
         * @param constraints the type of constraints that were updated
         */
        void updateConstraints(Plasma::Constraints constraints = Plasma::AllConstraints);

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
        virtual void constraintsUpdated(Plasma::Constraints constraints);

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
         * Returns the rect that the contents are positioned within in local coordinates
         */
        QRectF contentRect() const;

        /**
         * Returns the area within which contents can be painted.
         **/
        QSizeF contentSize() const;

        /**
         * Sets the content size.
         *
         * @note Normally an applet should never
         * call this directly except in the constructor to provide a default
         * size
         *
         * @arg size the new size of the contents area
         */
        void setContentSize(const QSizeF &size);

        /**
         * Sets the content size.
         *
         * @note Normally an applet should never
         * call this directly except in the constructor to provide a default
         * size
         *
         * @arg width the new width of the contents area
         * @arg height the new height of the contents area
         */
        void setContentSize(int width, int height);

        /**
         * Returns an ideal size for the applet's content.
         * Applets can re-implement this to provide a suitable size based
         * on their contents.
         *
         * Unlike sizeHint() , contentSizeHint() does not include the
         * size of any borders surrounding the content area.
         *
         * The default implementation returns the sizeHint() of the applet's
         * layout if it has one, or a null size otherwise.
         */
        virtual QSizeF contentSizeHint() const;

        /**
         * Sets the minimum size for the content of this applet
         */
        void setMinimumContentSize(const QSizeF &minSize);

        /**
         * Sets the minimum size for the content of this applet
         *
         * @arg minWidth the new minimum width of the contents area
         * @arg minHeight the new minimum height of the contents area
         */
        void setMinimumContentSize(int minWidth, int minHeight);

        /**
         * Get the minimum size for the content of this applet
         */
        QSizeF minimumContentSize() const;

        /**
         * Sets the maximum size for the content of this applet.
         */
        void setMaximumContentSize(const QSizeF &maxSize);

        /**
         * Sets the maximum size for the content of this applet
         *
         * @arg maxWidth the new maximum width of the contents area
         * @arg maxHeight the new maximum height of the contents area
         */
        void setMaximumContentSize(int maxWidth, int maxHeight);

        /**
         * Get the minimum size for the content of this applet
         */
        QSizeF maximumContentSize() const;

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
        static KPluginInfo::List knownApplets(const QString &category = QString(),
                                              const QString &parentApp = QString());

        /**
         * Returns a list of all known applets associated with a certain mimetype.
         *
         * @return list of applets
         **/
        static KPluginInfo::List knownAppletsForMimetype(const QString &mimetype);

        /**
         * Returns a list of all the categories used by
         * installed applets.
         *
         * @param parentApp the application to filter applets on. Uses the
         *                  X-KDE-ParentApp entry (if any) in the plugin info.
         *                  The default value of QString() will result in a
         *                  list containing only applets not specifically
         *                  registered to an application.
         * @return list of categories
         * @param visibleOnly true if it should only return applets that are marked as visible
         */
        static QStringList knownCategories(const QString &parentApp = QString(), bool visibleOnly = true);

        /**
         * @return true if this plasmoid provides a GUI configuration
         **/
        bool hasConfigurationInterface();

        /**
         * Attempts to load an applet
         *
         * Returns a pointer to the applet if successful.
         * The caller takes responsibility for the applet, including
         * deleting it when no longer needed.
         *
         * @param name the plugin name, as returned by KPluginInfo::pluginName()
         * @param applet unique ID to assign the applet, or zero to have one
         *        assigned automatically.
         * @param args to send the applet extra arguments
         * @return a pointer to the loaded applet, or 0 on load failure
         **/
        static Applet* loadApplet(const QString &name, uint appletId = 0,
                                  const QVariantList& args = QVariantList());

        /**
         * Attempts to load an applet
         *
         * Returns a pointer to the applet if successful.
         * The caller takes responsibility for the applet, including
         * deleting it when no longer needed.
         *
         * @param info KPluginInfo object for the desired applet
         * @param applet unique ID to assign the applet, or zero to have one
         *        assigned automatically.
         * @return a pointer to the loaded applet, or 0 on load failure
         **/
        static Applet* loadApplet(const KPluginInfo& info, uint appletId = 0,
                                  const QVariantList& args = QVariantList());

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
                                    const QRect& contentsRect);

        /**
         * Returns the user-visible name for the applet, as specified in the
         * .desktop file.
         *
         * @return the user-visible name for the applet.
         **/
        QString name() const;

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
         * Returns the color corresponding to the applet's category.
         */
        QColor color() const;

        /**
         * Get the category of the given applet
         *
         * @param a KPluginInfo object for the applet
         */
        static QString category(const KPluginInfo& applet);

        /**
         * Get the category of the given applet
         *
         * @param the name of the applet
         */
        static QString category(const QString& appletName);

        /**
         * @return true if this applet is immutable
         **/
        bool isImmutable() const;

        /**
         * @return true if this applet is immutable due to Kiosk settings
         */
        bool isKioskImmutable() const;

        /**
         * @return returns whether or not the applet is using the standard
         *         background
         **/
        bool drawStandardBackground();

        /**
         * Sets whether the applet should automatically draw the standard
         * background.
         *
         * Defaults to true
         **/
        void setDrawStandardBackground(bool drawBackground);

        /**
         * If for some reason, the applet fails to get up on its feet (the
         * library couldn't be loaded, necessary hardware support wasn't found,
         * etc..) this method returns true
         **/
        bool failedToLaunch() const;

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
        void setFailedToLaunch(bool failed, const QString& reason = QString());

        /**
         * @return true if the applet currently needs to be configured,
         *         otherwise, false
         */
        bool needsConfiguring() const;

        /**
         * When the applet needs to be configured before being usable, this
         * method can be called to show a standard interface prompting the user
         * to configure the applet
         *
         * Not that all children items will be deleted when this method is
         * called. If you have pointers to these items, you will need to
         * reset them after calling this method.
         *
         * @param needsConfiguring true if the applet needs to be configured,
         *                         or false if it doesn't
         */
        void setNeedsConfiguring(bool needsConfiguring);
        enum { Type = Plasma::AppletType };

        /**
         * Reimplemented from QGraphicsItem
         **/
        int type() const;

        /**
         * Reimplemented from QGraphicsItem
         **/
        QRectF boundingRect () const;

        /**
         * Returns a list of context-related QAction instances.
         *
         * This is used e.g. within the \a DesktopView to display a
         * contextmenu.
         *
         * @return A list of actions. The default implementation returns an
         *         empty list.
         **/
        virtual QList<QAction*> contextActions();

        /**
         * Sets shadow for the given applet.
         */
        void setShadowShown(bool);
        /**
         * Returns true if the given item has a shadow shown.
         */
        bool isShadowShown() const;

        // reimplemented from LayoutItem
        // value is the same as contentSizeHint() if drawStandardBackground() is false
        // or contentSizeHint() plus the size of the border otherwise.
        virtual QSizeF sizeHint() const;

        /**
         * Sets the geometry of this Plasma::Applet
         * @param geometry the geometry to apply to this Plasma::Applet.
         */
        void setGeometry(const QRectF &geometry);

        /**
         * Sets whether or not this Applet is acting as a Containment
         */
        void setIsContainment(bool isContainment);

        /**
         * @return true if this Applet is currently being used as a Containment, false otherwise
         */
        bool isContainment() const;

        /**
         * @internal
         */
        void resetConfigurationObject();

    Q_SIGNALS:
        /**
         * Emitted when the applet needs to take (or lose) keyboard focus.
         *
         * An applet should emit this signal to ensure that autohiding
         * elements stay unhidden and other bits of bookkeeping are
         * performed to ensure proper function.
         *
         * If you call watchForFocus on your applet, then this is handled for
         * the applet and it is not necessary to emit the signal directly.
         *
         * @param focus true if the applet is taking keyboard focus, false if
         *        it is giving it up
         **/
        void requestFocus( bool focus );

        /**
         * Emitted whenever the applet makes a geometry change, so that views
         * can coordinate themselves with these changes if they desire.
         */
        void geometryChanged();

    public Q_SLOTS:
        /**
         * Sets whether or not this applet is immutable or not.
         *
         * @arg immutable true if this applet should not be changeable
         **/
        void setImmutable(bool immutable);

        /**
         * Destroys the applet; it will be deleted and configurations reset.
         */
        void destroy();

        /**
         * Reimplement this slot to show a configuration dialog.
         *
         * Let the user play with the plasmoid options.
         * Called when the user selects the configure entry
         * from the context menu.
         */
        virtual void showConfigurationInterface();

        /**
         * Sends all pending contraints updates to the applet. Will usually
         * be called automatically, but can also be called manually if needed.
         */
        void flushUpdatedConstraints();

        /**
         * @return the Containment, if any, this applet belongs to
         **/
        Containment* containment() const;

    protected:
        /**
         * Called when a request to save the state of the applet is made
         * during runtime
         **/
        virtual void saveState(KConfigGroup* config) const;

        /**
         * Returns the name of the applet.
         *
         * This will be the same for all instances of this applet.
         **/
        QString globalName() const;

        /**
         * Returns a name unique to the instance of this applet.
         *
         * Useful for being able to refer directly to a particular
         * applet. Combines the global name with the applet id
         **/
        QString instanceName() const;

        /**
        * Register widgets that can receive keyboard focus.
        *
        * Calling this results in an eventFilter being places on the widget.
        *
        * @param widget the widget to watch for keyboard focus
        * @param watch whether to start watching the widget, or to stop doing so
        */
        void watchForFocus( QObject *widget, bool watch = true );

        /**
        * Call this whenever focus is needed or not needed.
        *
        * You do not have to call this method for widgets that have
        * been registered with watchForFocus
        *
        * @see watchForFocus
        * @param focus whether to or not to request focus
        */
        void needsFocus(bool focus);

        /**
         * Sets whether or not this applet provides a user interface for
         * configuring the applet.
         *
         * It defaults to false, and if true is passed in you should
         * also reimplement showConfigurationInterface()
         *
         * @arg hasInterface whether or not there is a user interface available
         **/
        void setHasConfigurationInterface(bool hasInterface);

        /**
         * @internal event filter; used for focus watching
         **/
        bool eventFilter( QObject *o, QEvent *e );

        /**
         * Reimplemented from QGraphicsItem
         */
        QVariant itemChange(GraphicsItemChange change, const QVariant &value);

    protected Q_SLOTS:
        /**
         * @internal used to show the configuration of an applet on first show
         */
        void performSetupConfig();

        /**
         * @internal used to check the immutability of the item in the config file
         */
        void checkImmutability();

    private:
        Q_DISABLE_COPY(Applet)

        /**
         * Reimplemented from QGraphicsItem
         **/
        void paintWidget(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0);

        class Private;
        Private* const d;
};

} // Plasma namespace

/**
 * Register an applet when it is contained in a loadable module
 */
#define K_EXPORT_PLASMA_APPLET(libname, classname) \
K_PLUGIN_FACTORY(factory, registerPlugin<classname>();) \
K_EXPORT_PLUGIN(factory("plasma_applet_" #libname))

#endif // multiple inclusion guard
