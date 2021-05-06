/*
    SPDX-FileCopyrightText: 2007 Aaron Seigo <aseigo@kde.org>
    SPDX-FileCopyrightText: 2007 Matt Broadstone <mbroadst@gmail.com>
    SPDX-FileCopyrightText: 2012 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef PLASMA_CORONA_H
#define PLASMA_CORONA_H

#include <plasma/containment.h>
#include <plasma/plasma.h>
#include <plasma/plasma_export.h>

class QAction;

namespace Plasma
{
class CoronaPrivate;

/**
 * @class Corona plasma/Corona.h <Plasma/Corona>
 *
 * @short A bookkeeping Scene for Plasma::Applets
 */
class PLASMA_EXPORT Corona : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool isStartupCompleted READ isStartupCompleted NOTIFY startupCompleted)
    Q_PROPERTY(bool editMode READ isEditMode WRITE setEditMode NOTIFY editModeChanged)
#if PLASMA_ENABLE_DEPRECATED_SINCE(5, 83)
    Q_PROPERTY(Package package READ package NOTIFY packageChanged)
#endif
    Q_PROPERTY(KPackage::Package kPackage READ kPackage NOTIFY kPackageChanged)

public:
    explicit Corona(QObject *parent = nullptr);
    ~Corona();

#if PLASMA_ENABLE_DEPRECATED_SINCE(5, 6)
    /**
     * Accessor for the associated Package object if any.
     * A Corona package defines how Containments are laid out in a View,
     * ToolBoxes, default layout, error messages
     * and in general all the furniture specific of a particular
     * device form factor.
     *
     * @return the Package object, or an invalid one if none
     * @since 5.0
     * @deprecated Since 5.6, use kPackage instead
     **/
    PLASMA_DEPRECATED_VERSION(5, 6, "Use Corona::kPackage()")
    Plasma::Package package() const;
#endif

#if PLASMA_ENABLE_DEPRECATED_SINCE(5, 6)
    /**
     * Setting the package name
     * @deprecated Since 5.6, use setKPackage instead
     */
    PLASMA_DEPRECATED_VERSION(5, 6, "Use Corona::setKPackage(const KPackage::Package &)")
    void setPackage(const Plasma::Package &package);
#endif

    /**
     * Accessor for the associated Package object if any.
     * A Corona package defines how Containments are laid out in a View,
     * ToolBoxes, default layout, error messages
     * and in genelal all the furniture specific of a particular
     * device form factor.
     *
     * @return the Package object, or an invalid one if none
     * @since 5.5
     **/
    KPackage::Package kPackage() const;

    /**
     * Setting the package for the corona
     * @since 5.5
     */
    void setKPackage(const KPackage::Package &package);

    /**
     * @return all containments on this Corona
     */
    QList<Containment *> containments() const;

    /**
     * @returns true when the startup is over, and
     * all the ui graphics has been instantiated
     */
    bool isStartupCompleted() const;

    /**
     * Returns the config file used to store the configuration for this Corona
     */
    KSharedConfig::Ptr config() const;

    /**
     * Adds a Containment to the Corona
     *
     * @param name the plugin name for the containment, as given by
     *        KPluginInfo::pluginName(). If an empty string is passed in, the default
     *        containment plugin will be used (usually DesktopContainment). If the
     *        string literal "null" is passed in, then no plugin will be loaded and
     *        a simple Containment object will be created instead.
     * @param args argument list to pass to the containment
     *
     * @return a pointer to the containment on success, or 0 on failure. Failure can be
     * caused by too restrictive of an Immutability type, as containments cannot be added
     * when widgets are locked.
     * If the requested containment plugin can not be located or successfully loaded, the Containment will have an invalid pluginInfo().
     */
    Containment *createContainment(const QString &name, const QVariantList &args = QVariantList());

    /**
     * Returns the Containment for a given physical screen and desktop, creating one
     * if none exists
     *
     * @param screen number of the physical screen to locate
     * @param activity the activity id of the containment we want,
     *                 and empty string if the activity is not important
     * @param defaultPluginIfNonExistent the plugin to load by default; "null" won't
     * create it and "default" creates the default plugin
     * @param defaultArgs optional arguments to pass in when creating a Containment if needed
     * @since 5.45
     */
    Containment *
    containmentForScreen(int screen, const QString &activity, const QString &defaultPluginIfNonExistent, const QVariantList &defaultArgs = QVariantList());

    // TODO KF6: add activity here, can't be done now as the overload would get confused
#if PLASMA_ENABLE_DEPRECATED_SINCE(5, 46)
    /**
     * Returns the Containment, if any, for a given physical screen
     *
     * @param screen number of the physical screen to locate
     * @deprecated Since 5.46, use containmentForScreen(int, const QString &, const QString &, const QVariantList &)
     */
    PLASMA_DEPRECATED_VERSION(5, 46, "Use Corona::containmentForScreen(int, const QString &, const QString &, const QVariantList &)")
    Containment *containmentForScreen(int screen) const;
#endif

#if PLASMA_ENABLE_DEPRECATED_SINCE(5, 46)
    /**
     * Returns the Containment for a given physical screen and desktop, creating one
     * if none exists
     *
     * @param screen number of the physical screen to locate
     * @param defaultPluginIfNonExistent the plugin to load by default; "null" is an empty
     * Containment and "default" creates the default plugin
     * @param defaultArgs optional arguments to pass in when creating a Containment if needed
     * @deprecated Since 5.46, use containmentForScreen(int, const QString &, const QString &, const QVariantList &)
     */
    PLASMA_DEPRECATED_VERSION(5, 46, "Use Corona::containmentForScreen(int, const QString &, const QString &, const QVariantList &)")
    Containment *containmentForScreen(int screen, const QString &defaultPluginIfNonExistent, const QVariantList &defaultArgs = QVariantList());
#endif

    /**
     * Returns all containments which match a particular activity, for any screen
     * @param activity the activity id we want
     * @returns the list of matching containments if any, empty if activity is an empty string
     * @since 5.45
     */
    QList<Containment *> containmentsForActivity(const QString &activity);

    /**
     * Returns all containments which match a particular screen, for any activity
     * @param screen the screen number we want
     * @returns the list of matching containments if any, empty if screen is < 0
     * @since 5.45
     */
    QList<Containment *> containmentsForScreen(int screen);

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
    virtual QRect screenGeometry(int id) const = 0;

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
     * Returns the available rect for a given screen.
     * The difference between this and availableScreenRegion()
     * is that this method returns only a rectangular
     * available space (it doesn't care if your panel is not 100% width).
     * The available rect excludes panels and similar windows.
     * Valid screen ids are 0 to numScreens()-1.
     * By default this method returns a rectangular region
     * equal to screenGeometry(id); subclasses that need another
     * behavior should override this method.
     */
    virtual QRect availableScreenRect(int id) const;

    /**
     * This method is useful in order to retrieve the list of available
     * screen edges for panel type containments.
     * @param screen the id of the screen to look for free edges.
     * @returns a list of free edges not filled with panel type containments.
     */
    QList<Plasma::Types::Location> freeEdges(int screen) const;

    /**
     * The actions associated with this Corona
     */
    KActionCollection *actions() const;

    /**
     * Imports an applet layout from a config file. The results will be added to the
     * current set of Containments.
     *
     * @param config the name of the config file to load from,
     *               or the default config file if QString()
     * @return the list of containments that were loaded
     * @since 4.6
     */
    QList<Plasma::Containment *> importLayout(const KConfigGroup &config);

    /**
     * Exports a set of containments to a config file.
     *
     * @param config the config group to save to
     * @param containments the list of containments to save
     * @since 4.6
     */
    void exportLayout(KConfigGroup &config, QList<Containment *> containments);

    /**
     * @returns the id of the screen which is showing @p containment
     * -1 is returned if the containment is not associated with a screen.
     */
    virtual int screenForContainment(const Containment *containment) const;

    /**
     * @return The type of immutability of this Corona
     */
    Types::ImmutabilityType immutability() const;

    /**
     * Set the Corona globally into "edit mode"
     * Only when the corona is of mutable type can be set of edit mode.
     * This indicates the UI to make easy for the user to manipulate applets.
     * @param edit
     * @since 5.63
     */
    void setEditMode(bool edit);

    /**
     * @returns true if the corona is in edit mode
     * @since 5.63
     */
    bool isEditMode() const;

public Q_SLOTS:
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
     * @param config the file to save to, or the default config file if QString()
     */
    void saveLayout(const QString &config = QString()) const;

    /**
     * Sets the immutability type for this Corona (not immutable,
     * user immutable or system immutable)
     * @param immutable the new immutability type of this applet
     */
    void setImmutability(const Types::ImmutabilityType immutable);

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
     * the Corona: it may occur after creation or restore from config
     */
    void containmentAdded(Plasma::Containment *containment);

    /**
     * This signal indicates a new containment has been created
     * in the Corona. Compared to containmentAdded it can only happen
     * after the creation of a new containment.
     *
     * @see containmentAdded
     * @since 5.16
     */
    void containmentCreated(Plasma::Containment *containment);

    /**
     * This signal indicates that a containment has been newly
     * associated (or dissociated) with a physical screen.
     *
     * @param isScreen the screen it is now associated with
     */
    void screenOwnerChanged(int isScreen);

    /**
     * This signal indicates that the configuration file was flushed to disk.
     */
    void configSynced();

    /**
     * This signal indicates that a change in available screen geometry occurred.
     */
    void availableScreenRegionChanged();

    /**
     * This signal indicates that a change in available screen geometry occurred.
     */
    void availableScreenRectChanged();

    /**
     * This signal indicates that a change in geometry for the screen occurred.
     */
    void screenGeometryChanged(int id);

    /**
     * emitted when immutability changes.
     * this is for use by things that don't get constraints events, like plasmaapp.
     * it's NOT for containments or applets or any of the other stuff on the scene.
     * if your code's not in shells/ it probably shouldn't be using it.
     */
    void immutabilityChanged(Plasma::Types::ImmutabilityType immutability);

    /** This signal indicates the screen with the specified id was removed.
     * @since 5.40
     */
    void screenRemoved(int id);

    /** This signal indicates a new screen with the specified id was added.
     * @since 5.40
     */
    void screenAdded(int id);

    /**
     * emitted when the editMode state changes
     * @see isEditMode()
     * @since 5.63
     */
    void editModeChanged(bool edit);

#if PLASMA_ENABLE_DEPRECATED_SINCE(5, 6)
    /**
     * Emitted when the package for this corona has been changed.
     * Shells must support changing the shell package on the fly (for instance due to device form factor changing)
     *
     * @param package the new package that defines the Corona furniture and behavior
     * @deprecated Since 5.6, use kPackageChanged instead
     */
    PLASMA_DEPRECATED_VERSION(5, 6, "Use Corona::kPackageChanged(const KPackage::Package &)")
    void packageChanged(const Plasma::Package &package);
#endif

    /**
     * Emitted when the package for this corona has been changed.
     * Shells must support changing the shell package on the fly (for instance due to device form factor changing)
     *
     * @param package the new package that defines the Corona furniture and behavior
     */
    void kPackageChanged(const KPackage::Package &package);

    /**
     * Emitted when the startup phase has been completed
     */
    void startupCompleted();

protected:
    /**
     * Loads the default (system wide) layout for this user
     **/
    virtual void loadDefaultLayout();

    /**
     * Loads a containment with delayed initialization, primarily useful
     * for implementations of loadDefaultLayout. The caller is responsible
     * for all initialization, saving and notification of a new containment.
     *
     * @param name the plugin name for the containment, as given by
     *        KPluginInfo::pluginName(). If an empty string is passed in, the default
     *        containment plugin will be used (usually DesktopContainment). If the
     *        string literal "null" is passed in, then no plugin will be loaded and
     *        a simple Containment object will be created instead.
     * @param args argument list to pass to the containment
     *
     * @return a pointer to the containment on success, or 0 on failure. Failure can
     * be caused by the Immutability type being too restrictive, as containments can't be added
     * when widgets are locked, or if the requested containment plugin can not be located
     * or successfully loaded.
     * @see addContainment
     **/
    Containment *createContainmentDelayed(const QString &name, const QVariantList &args = QVariantList());

private:
    CoronaPrivate *const d;

    Q_PRIVATE_SLOT(d, void containmentDestroyed(QObject *))
    Q_PRIVATE_SLOT(d, void syncConfig())
    Q_PRIVATE_SLOT(d, void toggleImmutability())
    Q_PRIVATE_SLOT(d, void containmentReady(bool))

    friend class CoronaPrivate;
    friend class View;
};

} // namespace Plasma

#endif
