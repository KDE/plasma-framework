/*
    SPDX-FileCopyrightText: 2006-2007 Aaron Seigo <aseigo@kde.org>
    SPDX-FileCopyrightText: 2007 Riccardo Iaconelli <riccardo@kde.org>
    SPDX-FileCopyrightText: 2008 Ménard Alexis <darktears31@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef PLASMA_APPLET_H
#define PLASMA_APPLET_H

#include <QKeySequence>
#include <QObject>
#include <QUrl>

#include <KConfigGroup>
#include <plasma/plasma_export.h>

#include <plasma/framesvg.h>
#include <plasma/plasma.h>

namespace KPackage
{
class Package;
}
namespace PlasmaQuick
{
class AppletQuickItem;
class ConfigViewPrivate;
class ConfigModelPrivate;
class ConfigModel;
};
class DeclarativeAppletScript;
#include <KPluginFactory>

class KActionCollection;
class KConfigLoader;

namespace Plasma
{
class AppletPrivate;
class Containment;
class DataEngine;
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
class PLASMA_EXPORT Applet : public QObject
{
    Q_OBJECT
    /**
     * Applet id: is unique in the whole Plasma session and will never change across restarts
     */
    Q_PROPERTY(uint id READ id CONSTANT FINAL)

    /**
     * User friendly title for the plasmoid: it's the localized applet name by default
     */
    Q_PROPERTY(QString title READ title WRITE setTitle NOTIFY titleChanged FINAL)

    /**
     * Icon to represent the plasmoid
     */
    Q_PROPERTY(QString icon READ icon WRITE setIcon NOTIFY iconChanged FINAL)

    // TODO KF6 toolTipMainText toolTipSubText toolTipTextFormat toolTipItem: need to either be here or some other kind of attached property

    /**
     * The current form factor the applet is being displayed in.
     *
     * @see Plasma::FormFactor
     */
    Q_PROPERTY(Plasma::Types::FormFactor formFactor READ formFactor NOTIFY formFactorChanged)

    /**
     * The location of the scene which is displaying applet.
     *
     * @see Plasma::Types::Location
     */
    Q_PROPERTY(Plasma::Types::Location location READ location NOTIFY locationChanged)

    /**
     * Status of the plasmoid: useful to instruct the shell if this plasmoid is requesting attention, if is accepting input, or if is in an idle, inactive state
     */
    Q_PROPERTY(Plasma::Types::ItemStatus status READ status WRITE setStatus NOTIFY statusChanged)

    /**
     * The immutability of the Corona.
     * Tells the applet whether it should allow for any modification by the user.
     */
    Q_PROPERTY(Plasma::Types::ImmutabilityType immutability READ immutability WRITE setImmutability NOTIFY immutabilityChanged)

    /**
     * Display hints that come from the containment that suggest the applet how to look and behave.
     */
    Q_PROPERTY(Plasma::Types::ContainmentDisplayHints containmentDisplayHints READ containmentDisplayHints NOTIFY containmentDisplayHintsChanged)

    /**
     * True if the applet should show a busy status, for instance doing
     * some network operation
     */
    Q_PROPERTY(bool busy READ isBusy WRITE setBusy NOTIFY busyChanged FINAL)

    /**
     * True when the user is configuring, for instance when the configuration dialog is open.
     */
    Q_PROPERTY(bool userConfiguring READ isUserConfiguring NOTIFY userConfiguringChanged)

    /**
     * How the applet wants its background to be drawn. The containment may chose to ignore this hint.
     */
    Q_PROPERTY(Plasma::Types::BackgroundHints backgroundHints WRITE setBackgroundHints READ backgroundHints NOTIFY backgroundHintsChanged FINAL)

    /**
     * The containment (and/or the user) may decide to use another kind of background instead (if supported by the applet)
     */
    Q_PROPERTY(Plasma::Types::BackgroundHints userBackgroundHints WRITE setUserBackgroundHints READ userBackgroundHints NOTIFY userBackgroundHintsChanged FINAL)

    /**
     * The effective background hints the applet has, internally decided how to mix with userBackgroundHints
     */
    Q_PROPERTY(Plasma::Types::BackgroundHints effectiveBackgroundHints READ effectiveBackgroundHints NOTIFY effectiveBackgroundHintsChanged FINAL)

    /**
     * Sets the associated application of this plasmoid, if the plasmoid is representing the "compact" view for some application or for some document type.
     */
    Q_PROPERTY(QString associatedApplication WRITE setAssociatedApplication READ associatedApplication NOTIFY associatedApplicationChanged)

    /**
     * Sets the associated application of this plasmoid, if the plasmoid is representing the "compact" view for some application or for some document type.
     */
    Q_PROPERTY(QList<QUrl> associatedApplicationUrls WRITE setAssociatedApplicationUrls READ associatedApplicationUrls NOTIFY associatedApplicationUrlsChanged)

    // TODO KF6: activity, screen, screenGeometry, availableScreenRect, availableScreenRegion: should we instead make the containment accessible from qml
    // plasmoids and ask from there?

    // TODO KF6: configuration object

    // TODO KF6: hideOnWindowDeactivate

    /**
     * The global shortcut to activate the plasmoid
     *
     * This is typically only used by the default configuration module
     *
     */
    Q_PROPERTY(QKeySequence globalShortcut READ globalShortcut WRITE setGlobalShortcut RESET setGlobalShortcut NOTIFY globalShortcutChanged)

    /**
     * If true the applet requires manual configuration from the user
     * TODO KF6: having just a reson property and required would be string not empty? Uglier from c++ pov but more straight forward from qml pov
     */
    Q_PROPERTY(bool configurationRequired READ configurationRequired WRITE setConfigurationRequired NOTIFY configurationRequiredChanged)

    // TODO KF6: constraintHints

    /**
     * The metadata of the applet.
     */
    Q_PROPERTY(KPluginMetaData metaData READ pluginMetaData CONSTANT)

    // TODO KF6: contextualActions along with AppletInterface::setAction etc, alsongside a declarative way?

    // TODO KF6: editMode

public:
    // CONSTRUCTORS

    /**
     * This constructor can be used with the KCoreAddons plugin loading system.
     * The argument list is expected to have contain the KPackage of the applet,
     * the meta data file path (for compatibility) and an applet ID which must be a base 10 number.
     *
     * @param parent a QObject parent; you probably want to pass in 0
     * @param data, KPluginMetaData used to create this plugin
     * @param args a list of strings containing the applet id
     * @Since 5.86
     */
    Applet(QObject *parentObject, const KPluginMetaData &data, const QVariantList &args);

    ~Applet() override;

    // BOOKKEEPING
    /**
     * @return the id of this applet
     */
    uint id() const;

    /**
     * @return The type of immutability of this applet
     */
    Types::ImmutabilityType immutability() const;

    /**
     * If for some reason, the applet fails to get up on its feet (the
     * library couldn't be loaded, necessary hardware support wasn't found,
     * etc..) this method returns the reason why, in an user-readable way.
     * @since 5.0
     **/
    QString launchErrorMessage() const;

    /**
     * If for some reason, the applet fails to get up on its feet (the
     * library couldn't be loaded, necessary hardware support wasn't found,
     * etc..) this method returns true.
     **/
    bool failedToLaunch() const;

    /**
     * @return true if destroy() was called; useful for Applets which should avoid
     * certain tasks if they are about to be deleted permanently
     */
    bool destroyed() const;

    /**
     * @return the Containment, if any, this applet belongs to
     **/
    Containment *containment() const;

    /**
     * @return true if this Applet is currently being used as a Containment, false otherwise
     */
    bool isContainment() const;

    /**
     * @return the status of the applet
     * @since 4.4
     */
    Types::ItemStatus status() const;

    /**
     * Returns the current form factor the applet is being displayed in.
     *
     * @see Plasma::FormFactor
     */
    Types::FormFactor formFactor() const;

    /**
     * Returns the location of the scene which is displaying applet.
     *
     * @see Plasma::Types::Location
     */
    Types::Location location() const;

    /**
     * @return Display hints that come from the containment that suggest the applet how to look and behave.
     * @since 5.77
     */
    Types::ContainmentDisplayHints containmentDisplayHints() const;

    // CONFIGURATION
    /**
     * Returns the KConfigGroup to access the applets configuration.
     *
     * This config object will write to an instance
     * specific config file named \<appletname\>\<instanceid\>rc
     * in the Plasma appdata directory.
     **/
    KConfigGroup config() const;

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
    KConfigLoader *configScheme() const;

    /**
     * Saves state information about this applet that will
     * be accessed when next instantiated in the restore(KConfigGroup&) method.
     *
     * This method does not need to be reimplemented by Applet
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
     * This method does not need to be reimplemented by Applet
     * subclasses, but can be useful for Applet specializations
     * (such as Containment) to do so.
     **/
    virtual void restore(KConfigGroup &group);

    /**
     * @return true if the applet currently needs to be configured,
     *         otherwise, false
     */
    bool configurationRequired() const;

    /**
     * @return A translated message for the user explaining that the
     *           applet needs configuring; this should note what needs
     *           to be configured
     *
     * @see setConfigurationRequired
     * @since 5.20
     */
    QString configurationRequiredReason() const;

    /**
     * @return true when the configuration interface is being shown
     * @since 4.5
     */
    bool isUserConfiguring() const;

    /**
     * Tells the applet the user is configuring
     * @param configuring true if the configuration ui is showing
     */
    void setUserConfiguring(bool configuring);

    // UTILS
    /**
     * Called when any of the geometry constraints have been updated.
     * This method calls constraintsEvent, which may be reimplemented,
     * once the Applet has been prepared for updating the constraints.
     *
     * @param constraints the type of constraints that were updated
     */
    void updateConstraints(Plasma::Types::Constraints constraints = Plasma::Types::AllConstraints);

    // METADATA

    /**
     * @return metadata information about this plugin
     *
     * @since 5.27
     */
    KPluginMetaData pluginMetaData() const;

    /**
     * Returns the user-visible title for the applet, as specified in the
     * Name field of the .desktop file. Can be changed with @see setTitle
     *
     * @since 5.0
     * @return the user-visible title for the applet.
     **/
    QString title() const;

    /**
     * Sets a custom title for this instance of the applet. E.g. a clock might
     * use the timezone as its name rather than the .desktop file
     *
     * @since 5.0
     * @param title the user-visible title for the applet.
     */
    void setTitle(const QString &title);

    /**
     * @returns The icon name related to this applet
     * By default is the one in the plasmoid desktop file
     **/
    QString icon() const;

    /**
     * Sets an icon name for this applet
     * @param icon Freedesktop compatible icon name
     */
    void setIcon(const QString &icon);

    /**
     * @returns true if the applet should show a busy status, for instance doing
     * some network operation
     * @since 5.21
     */
    bool isBusy() const;

    /**
     * Sets the Applet to have a busy status hint, for instance the applet doing
     * some network operation.
     * The graphical representation of the busy status depends completely from
     * the visualization.
     * @param busy true if the applet is busy
     * @since 5.21
     */
    void setBusy(bool busy);

    /**
     * How the applet wants its background to be drawn. The containment may chose to ignore this hint.
     * @since 5.65
     */
    Plasma::Types::BackgroundHints backgroundHints() const;

    /**
     * Sets the applet background hints. Only Applet implementations should write this property
     * @since 5.65
     */
    void setBackgroundHints(Plasma::Types::BackgroundHints hint);

    /**
     * The containment (and/or the user) may decide to use another kind of background instead if supported by the applet.
     * In order for an applet to support user configuration of the
     * background, it needs to have the Plasma::Types::ConfigurableBackground flag set in its backgroundHints
     * @since 5.65
     */
    Plasma::Types::BackgroundHints userBackgroundHints() const;

    /**
     * Sets the hints the user wished the background style for the applet to be.
     * @since 5.65
     */
    void setUserBackgroundHints(Plasma::Types::BackgroundHints hint);

    /**
     * The effective background hints the applet will have: it will follow userBackgroundHints only if backgroundHints has the
     * Plasma::Types::ConfigurableBackground flag set
     * @since 5.65
     */
    Plasma::Types::BackgroundHints effectiveBackgroundHints() const;

    // ACTIONS
    /**
     * Returns a list of context-related QAction instances.
     *
     * This is used e.g. within the \a DesktopView to display a
     * contextmenu.
     *
     * @return A list of actions. The default implementation returns an
     *         empty list.
     **/
    virtual QList<QAction *> contextualActions();

    /**
     * Returns the collection of actions for this Applet
     */
    KActionCollection *actions() const;

    /**
     * Sets the global shortcut to associate with this widget.
     */
    void setGlobalShortcut(const QKeySequence &shortcut = QKeySequence());

    /**
     * @return the global shortcut associated with this widget, or
     * an empty shortcut if no global shortcut is associated.
     */
    QKeySequence globalShortcut() const;

    // ASSOCIATED APPLICATION
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
    void setAssociatedApplicationUrls(const QList<QUrl> &urls);

    /**
     * @return the application associated to this applet
     * @since 4.4
     */
    QString associatedApplication() const;

    /**
     * @return the urls associated to this applet
     * @since 4.4
     */
    QList<QUrl> associatedApplicationUrls() const;

    /**
     * @return true if the applet has a valid associated application or urls
     * @since 4.4
     */
    bool hasValidAssociatedApplication() const;

    // Completely UI-specific, remove or move to scriptengine
    /**
     * @return true if this plasmoid provides a GUI configuration
     **/
    bool hasConfigurationInterface() const;

Q_SIGNALS:
    // BOOKKEEPING
    /**
     * Emitted when the immutability changes
     * @since 4.4
     */
    void immutabilityChanged(Plasma::Types::ImmutabilityType immutable);

    /**
     * Emitted when the applet status changes
     * @since 4.4
     */
    void statusChanged(Plasma::Types::ItemStatus status);

    /**
     * Emitted when the applet has been scheduled for destruction
     * or the destruction has been undone
     * @since 5.4
     */
    void destroyedChanged(bool destroyed);

    /**
     * Emitted when the title has changed
     * @since 5.20
     */
    void titleChanged(const QString &title);

    /**
     * Emitted when the icon name for the applet has changed
     * @since 5.20
     */
    void iconChanged(const QString &icon);

    /**
     * Emitted when the busy status has changed
     * @since 5.21
     */
    void busyChanged(bool busy);

    /**
     * Emitted when the background hints have changed
     * @since 5.65
     */
    void backgroundHintsChanged();

    /**
     * Emitted when the user background hints have changed
     * @since 5.65
     */
    void userBackgroundHintsChanged();

    /**
     * Emitted when the effective background hints have changed
     * @since 5.65
     */
    void effectiveBackgroundHintsChanged();

    /**
     * Emitted when the associated application changes
     */
    void associatedApplicationChanged(const QString &application);

    /**
     * Emitted when the associated application urls changes
     */
    void associatedApplicationUrlsChanged(const QList<QUrl> &urls);

    /**
     * Emitted when the global shortcut to activate this applet has chanaged
     */
    void globalShortcutChanged(const QKeySequence &sequence);

    // CONFIGURATION
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
     * emitted when the config ui appears or disappears
     */
    void userConfiguringChanged(bool configuring);

    // ACTIONS
    /**
     * Emitted just before the contextual actions are about to show
     * For instance just before the context menu containing the actions
     * added with setAction() is shown
     */
    void contextualActionsAboutToShow();

    /**
     * Emitted when activation is requested due to, for example, a global
     * keyboard shortcut. By default the widget is given focus.
     */
    void activated();

    // TODO: fix usage in containment, port to QObject::destroyed
    /**
     * Emitted when the applet is deleted
     */
    void appletDeleted(Plasma::Applet *applet);

    /**
     * Emitted when the formfactor changes
     */
    void formFactorChanged(Plasma::Types::FormFactor formFactor);

    /**
     * Emitted when the location changes
     */
    void locationChanged(Plasma::Types::Location location);

    void containmentDisplayHintsChanged(Plasma::Types::ContainmentDisplayHints hints);

    /**
     * Emitted when setConfigurationRequired was called
     * @see setConfigurationRequired
     * @since 5.20
     */
    void configurationRequiredChanged(bool needsConfig, const QString &reason);

    // TODO KF6 keep as Q_SLOT only stuff that needsto be manually invokable from qml
public Q_SLOTS:
    // BOOKKEEPING
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
     * @since 5.0
     **/
    void setLaunchErrorMessage(const QString &reason = QString());

    /**
     * Sets the immutability type for this applet (not immutable,
     * user immutable or system immutable)
     * @param immutable the new immutability type of this applet
     */
    void setImmutability(const Types::ImmutabilityType immutable);

    /**
     * Destroys the applet; it will be removed nicely and deleted.
     * Its configuration will also be deleted.
     * If you want to remove the Applet configuration, use this, don't just delete the Applet *
     */
    void destroy();

    /**
     * sets the status for this applet
     * @since 4.4
     */
    void setStatus(const Types::ItemStatus stat);

    // CONFIGURATION
    /**
     * Called when applet configuration values have changed.
     */
    // TODO KF6: make it not a slot anymore and protected
    virtual void configChanged();

    // UTILS
    /**
     * Sends all pending constraints updates to the applet. Will usually
     * be called automatically, but can also be called manually if needed.
     */
    void flushPendingConstraintsEvents();

    /**
     * This method is called once the applet is loaded and added to a Corona.
     * If the applet requires a Scene or has an particularly intensive
     * set of initialization routines to go through, consider implementing it
     * in this method instead of the constructor.
     *
     * Note: paintInterface may get called before init() depending on initialization
     * order. Painting is managed by the canvas (QGraphisScene), and may schedule a
     * paint event prior to init() being called.
     **/
    virtual void init();

    // ASSOCIATED APPLICATION
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
    // CONFIGURATION
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

    // UTILS
    /**
     * Called when any of the constraints for the applet have been updated. These constraints
     * range from notifying when the applet has officially "started up" to when geometry changes
     * to when the form factor changes.
     *
     * Each constraint that has been changed is passed in the constraints flag.
     * All of the constraints and how they work is documented in the @see Plasma::Constraints
     * enumeration.
     *
     * On applet creation, this is always called prior to painting and can be used as an
     * opportunity to layout the widget, calculate sizings, etc.
     *
     * Do not call update() from this method; an update() will be triggered
     * at the appropriate time for the applet.
     *
     * @param constraints the type of constraints that were updated
     * @property constraint
     */
    virtual void constraintsEvent(Plasma::Types::Constraints constraints);

    // TODO: timerEvent should go into AppletPrivate
    /**
     * Reimplemented from QObject
     */
    void timerEvent(QTimerEvent *event) override;

private:
    QString filePath(const QByteArray &key, const QString &filename = QString()) const;
    KPackage::Package kPackage() const;
    /**
     * @internal This constructor is to be used with the Package loading system.
     *
     * @param parent a QObject parent; you probably want to pass in 0
     * @param args a list of strings containing two entries: the service id
     *      and the applet id
     * @since 4.3
     */
    Applet(const QString &packagePath, uint appletId);

    // TODO KF6: drop Q_PRIVATE_SLOT
    Q_PRIVATE_SLOT(d, void cleanUpAndDelete())
    Q_PRIVATE_SLOT(d, void askDestroy())
    Q_PRIVATE_SLOT(d, void updateShortcuts())
    Q_PRIVATE_SLOT(d, void globalShortcutChanged())
    Q_PRIVATE_SLOT(d, void propagateConfigChanged())
    Q_PRIVATE_SLOT(d, void requestConfiguration())

    AppletPrivate *const d;

    // Corona needs to access setLaunchErrorMessage and init
    friend class Corona;
    friend class CoronaPrivate;
    friend class Containment;
    friend class ContainmentPrivate;
    friend class AppletScript;
    friend class AppletPrivate;
    friend class AccessAppletJobPrivate;
    friend class GraphicsViewAppletPrivate;
    friend class PluginLoader;
    friend class AssociatedApplicationManager;
    friend class SvgPrivate;
    friend class PlasmaQuick::AppletQuickItem;
    friend class PlasmaQuick::ConfigModel;
    friend class PlasmaQuick::ConfigModelPrivate;
    friend class PlasmaQuick::ConfigViewPrivate;
    friend DeclarativeAppletScript;
};

} // Plasma namespace

#endif // multiple inclusion guard
