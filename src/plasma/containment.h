/*
    SPDX-FileCopyrightText: 2007 Aaron Seigo <aseigo@kde.org>
    SPDX-FileCopyrightText: 2008 Ménard Alexis <darktears31@gmail.com>
    SPDX-FileCopyrightText: 2009 Chani Armitage <chani@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef PLASMA_CONTAINMENT_H
#define PLASMA_CONTAINMENT_H

#include <KSharedConfig>
#include <plasma/applet.h>

namespace Plasma
{
class DataEngine;
class Package;
class Corona;
class ContainmentActions;
class ContainmentPrivate;

/**
 * @class Containment plasma/containment.h <Plasma/Containment>
 *
 * @short The base class for plugins that provide backgrounds and applet grouping containers
 *
 * Containment objects provide the means to group applets into functional sets.
 * They also provide the following:
 *
 * creation of focusing event
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
    Q_PROPERTY(QString wallpaper READ wallpaper WRITE setWallpaper NOTIFY wallpaperChanged)
    Q_PROPERTY(bool isUiReady READ isUiReady NOTIFY uiReadyChanged)

public:
    /**
     * This constructor can be used with the KCoreAddons plugin loading system.
     * The argument list is expected to have contain the KPackage of the applet,
     * the meta data file path (for compatibility) and an applet ID which must be a base 10 number.
     *
     * @param parent a QObject parent; you probably want to pass in 0
     * @param data, KPluginMetaData used to create this plugin
     * @param args a list of strings containing the applet id
     * @since 5.86
     */
    explicit Containment(QObject *parentObject, const KPluginMetaData &data, const QVariantList &args);

#if PLASMA_ENABLE_DEPRECATED_SINCE(5, 86)
    /**
     * @param parent the QObject this applet is parented to
     * @param serviceId the name of the .desktop file containing the
     *      information about the widget
     * @param containmentId a unique id used to differentiate between multiple
     *      instances of the same Applet type
     * @deprecated Since 5.86, use Containment(QObject *, KPluginMetaData, QVariantList) instead
     */
    PLASMA_DEPRECATED_VERSION(5, 86, "use Containment(QObject *, KPluginMetaData, QVariantList) instead")
    explicit Containment(QObject *parent = nullptr, const QString &serviceId = QString(), uint containmentId = 0);
#endif

#if PLASMA_ENABLE_DEPRECATED_SINCE(5, 86)
    /**
     * This constructor is to be used with the plugin loading systems
     * found in KPluginInfo and KService. The argument list is expected
     * to have two elements: the KService service ID for the desktop entry
     * and an applet ID which must be a base 10 number.
     *
     * @param parent a QObject parent; you probably want to pass in 0
     * @param args a list of strings containing two entries: the service id
     *      and the applet id
     * @deprecated Since 5.86, use Containment(QObject *, KPluginMetaData, QVariantList) instead
     */
    PLASMA_DEPRECATED_VERSION(5, 86, "use Containment(QObject *, KPluginMetaData, QVariantList) instead")
    Containment(QObject *parent, const QVariantList &args);
#endif

    ~Containment() override;

    /**
     * Reimplemented from Applet
     */
    void init() override;

    /**
     * Returns the type of containment
     */
    Plasma::Types::ContainmentType containmentType() const;

    /**
     * Returns the Corona (if any) that this Containment is hosted by
     */
    Corona *corona() const;

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
    Applet *createApplet(const QString &name, const QVariantList &args = QVariantList());

    /**
     * Add an existing applet to this Containment
     *
     * @param applet the applet that should be added
     * @param pos the containment-relative position
     */
    void addApplet(Applet *applet);

    /**
     * @return the applets currently in this Containment
     */
    QList<Applet *> applets() const;

    /**
     * @return the screen number this containment is serving as the desktop for
     *         or -1 if none
     */
    int screen() const;

    /**
     * @return the last screen number this containment had
     *         only returns -1 if it's never ever been on a screen
     * @since 4.5
     */
    int lastScreen() const;

    /**
     * @reimp
     * @sa Applet::save(KConfigGroup &)
     */
    void save(KConfigGroup &group) const override;

    /**
     * @reimp
     * @sa Applet::restore(KConfigGroup &)
     */
    void restore(KConfigGroup &group) override;

    /**
     * Sets wallpaper plugin.
     *
     * @param pluginName the name of the wallpaper to attempt to load
     */
    void setWallpaper(const QString &pluginName);

    /**
     * Return wallpaper plugin.
     */
    QString wallpaper() const;

    /**
     * Sets the current activity by id
     *
     * @param activity the id of the activity
     */
    void setActivity(const QString &activityId);

    /**
     * @return the current activity id associated with this containment
     */
    QString activity() const;

    /**
     * Sets a containmentactions plugin.
     *
     * @param trigger the mouse button (and optional modifier) to associate the plugin with
     * @param pluginName the name of the plugin to attempt to load. blank = set no plugin.
     * @since 4.4
     */
    void setContainmentActions(const QString &trigger, const QString &pluginName);

    /**
     * @return All the loaded containment action plugins, indexed by trigger name
     * @since 5.0
     */
    QHash<QString, ContainmentActions *> &containmentActions();

    /**
     * @returns true when the ui of this containment is fully loaded, as well the ui of every applet in it
     */
    bool isUiReady() const;

Q_SIGNALS:
    /**
     * This signal is emitted when a new applet is added in the containment
     * It may happen in the following situations:
     * * The user created the applet
     * * The applet was moved in from another containment
     * * The applet got restored at startup
     */
    void appletAdded(Plasma::Applet *applet);

    /**
     * This signal is emitted when an applet is destroyed
     */
    void appletRemoved(Plasma::Applet *applet);

    /**
     * This signal is emitted when a new applet is created by the containment.
     * Compared to appletAdded, this gets emitted only when the user explicitly
     * creates a new applet, either via the widget explorer or the scripting
     * environment.
     * @see appletAdded
     * @since 5.16
     */
    void appletCreated(Plasma::Applet *applet);

    /**
     * Emitted when the activity id has changed
     */
    void activityChanged(const QString &activity);

    /**
     * Emitted when the containment requests an add widgets dialog is shown.
     * Usually only used for desktop containments.
     *
     * @param pos where in the containment this request was made from, or
     *            an invalid position (QPointF()) is not location specific
     */
    void showAddWidgetsInterface(const QPointF &pos);

    /**
     * This signal indicates that a containment has been
     * associated (or dissociated) with a physical screen.
     *
     * @param newScreen the screen it is now associated with
     */
    void screenChanged(int newScreen);

    /**
     * Emitted when the user wants to configure/change the containment, or an applet inside it.
     */
    void configureRequested(Plasma::Applet *applet);

    /**
     * Emitted when the user wants to chose an alternative for this applet or containment.
     */
    void appletAlternativesRequested(Plasma::Applet *applet);

    /**
     * Emitted when the wallpaper plugin is changed
     */
    void wallpaperChanged();

    /**
     * Emitted when the location has changed
     * @since 5.0
     */
    void locationChanged(Plasma::Types::Location location);

    /**
     * Emitted when the formFactor has changed
     * @since 5.0
     */
    void formFactorChanged(Plasma::Types::FormFactor formFactor);

    /**
     * Emitted when the ui has been fully loaded and is fully working
     * @param uiReady true when the ui of the containment is ready, as well the ui of each applet in it
     */
    void uiReadyChanged(bool uiReady);

    /**
     * emitted when the containment type changed
     */
    void containmentTypeChanged();

public Q_SLOTS:
    /**
     * Informs the Corona as to what position it is in. This is informational
     * only, as the Corona doesn't change its actual location. This is,
     * however, passed on to Applets that may be managed by this Corona.
     *
     * @param location the new location of this Corona
     */
    void setLocation(Plasma::Types::Location location);

    /**
     * Sets the form factor for this Containment. This may cause changes in both
     * the arrangement of Applets as well as the display choices of individual
     * Applets.
     */
    void setFormFactor(Plasma::Types::FormFactor formFactor);

    /**
     * Set Display hints that come from the containment that suggest the applet how to look and behave.
     *
     * @param hints the new hints, as bitwise OR
     * @since 5.77
     */
    void setContainmentDisplayHints(Plasma::Types::ContainmentDisplayHints hints);

    /**
     * Sets the type of this containment.
     */
    void setContainmentType(Plasma::Types::ContainmentType type);

    void reactToScreenChange();

protected:
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

private:
    /**
     * @internal This constructor is to be used with the Package loading system.
     *
     * @param parent a QObject parent; you probably want to pass in 0
     * @since 4.3
     */
    Containment(const KPluginMetaData &md, uint appletId);

    Q_PRIVATE_SLOT(d, void appletDeleted(Plasma::Applet *))
    Q_PRIVATE_SLOT(d, void triggerShowAddWidgets())
    Q_PRIVATE_SLOT(d, void checkStatus(Plasma::Types::ItemStatus))

    friend class Applet;
    friend class AppletPrivate;
    friend class CoronaPrivate;
    friend class ContainmentPrivate;
    friend class ContainmentActions;
    ContainmentPrivate *const d;
};

} // Plasma namespace

#endif // multiple inclusion guard
