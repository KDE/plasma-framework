/*
    SPDX-FileCopyrightText: 2007 Aaron Seigo <aseigo@kde.org>
    SPDX-FileCopyrightText: 2008 Ménard Alexis <darktears31@gmail.com>
    SPDX-FileCopyrightText: 2009 Chani Armitage <chani@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef PLASMA_CONTAINMENT_H
#define PLASMA_CONTAINMENT_H

#include <KPluginMetaData>
#include <KSharedConfig>
#include <plasma/applet.h>
#include <qtmetamacros.h>

class QQuickItem;

namespace Plasma
{
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

    /**
     * List of applets this containment has: the containments
     * KF6: this should be AppletQuickItem *
     */
    Q_PROPERTY(QList<Plasma::Applet *> applets READ applets NOTIFY appletsChanged)

    /**
     * The corona for this contaiment
     */
    Q_PROPERTY(Plasma::Corona *corona READ corona CONSTANT)

    /**
     * Type of this containment
     */
    Q_PROPERTY(Plasma::Containment::Type containmentType READ containmentType NOTIFY containmentTypeChanged)

    /**
     * Activity UID of this containment
     */
    Q_PROPERTY(QString activity READ activity NOTIFY activityChanged)

    /**
     * Activity name of this containment
     */
    Q_PROPERTY(QString activityName READ activityName NOTIFY activityNameChanged)

    Q_PROPERTY(Plasma::Types::ContainmentDisplayHints containmentDisplayHints READ containmentDisplayHints WRITE setContainmentDisplayHints NOTIFY
                   containmentDisplayHintsChanged)

    Q_PROPERTY(QString wallpaper READ wallpaper WRITE setWallpaper NOTIFY wallpaperChanged)
    Q_PROPERTY(QObject *wallpaperGraphicsObject READ wallpaperGraphicsObject WRITE setWallpaperGraphicsObject NOTIFY wallpaperGraphicsObjectChanged)

    Q_PROPERTY(bool isUiReady READ isUiReady NOTIFY uiReadyChanged)

    /**
     * The screen number this containment is serving as the desktop for, or -1 if none
     */
    Q_PROPERTY(int screen READ screen NOTIFY screenChanged)

    /**
     * screen area free of panels: the coordinates are relative to the containment,
     * it's independent from the screen position
     * For more precise available geometry use availableScreenRegion()
     */
    Q_PROPERTY(QRectF availableScreenRect READ availableRelativeScreenRect NOTIFY availableRelativeScreenRectChanged)

    /**
     * The available region of this screen, panels excluded. It's a list of rectangles
     */
    Q_PROPERTY(QList<QRectF> availableScreenRegion READ availableRelativeScreenRegion NOTIFY availableRelativeScreenRegionChanged)

    /**
     * Provides access to the geometry of the applet is in.
     * Can be useful to figure out what's the absolute position of the applet.
     */
    Q_PROPERTY(QRectF screenGeometry READ screenGeometry NOTIFY screenGeometryChanged)

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

    ~Containment() override;

    /**
     * Reimplemented from Applet
     */
    void init() override;

    /**
     * This enumeration describes the type of the Containment.
     * DesktopContainments represent main containments that will own a screen in a mutually exclusive fashion,
     * while PanelContainments are accessories which can be present multiple per screen.
     *
     * This value is specified in the "X-Plasma-ContainmentType" JSON-metadata value of containments.
     */
    enum Type {
        NoContainment = -1, /**< @internal */
        Desktop = 0, /**< A desktop containment */
        Panel, /**< A desktop panel */
        Custom = 127, /**< A containment that is neither a desktop nor a panel but something application specific */
        CustomPanel = 128, /**< A customized desktop panel. "CustomPanel" in metadata */
        CustomEmbedded = 129, /**< A customized containment embedded in another applet */
    };
    Q_ENUM(Type)

    /**
     * Returns the type of containment
     */
    Type containmentType() const;

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
     * @param geometryHint an hint to pass to the GUI on the location
     *           and size we prefer for the newly created applet;
     *           the gui might choose whether to respect or not this hint
     *
     * @return a pointer to the applet on success, or 0 on failure
     */
    Applet *createApplet(const QString &name, const QVariantList &args = QVariantList(), const QRectF &geometryHint = QRectF());

    /**
     * Add an existing applet to this Containment
     *
     * @param applet the applet that should be added
     * @param geometryHint an hint to pass to the GUI on the location
     *           and size we prefer for the newly created applet;
     *           the gui might choose whether to respect or not this hint
     */
    Q_INVOKABLE void addApplet(Applet *applet, const QRectF &geometryHint = QRectF());

    /**
     * @return the applets currently in this Containment
     */
    QList<Applet *> applets() const;

    /**
     * @return the screen number this containment is serving as the desktop for
     *         or -1 if none
     * TODO KF6 virtual? this shouldbe available to applet as well
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
     * TODO KF6: this should be available to Appelt as well as a property... virtual?
     */
    QString activity() const;

    /**
     * @return Activity name corresponding to the activity UID
     * @see activity
     */
    QString activityName() const;

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

    /**
     * @returns The available screen rect (excluding panels) for the screen this containment is associated to,
     * empty rectangle if the containment is not active in a screen
     */
    QRectF availableRelativeScreenRect() const;

    /**
     * @returns The available region of this screen, panels excluded. It's a list of rectangles
     */
    QList<QRectF> availableRelativeScreenRegion() const;

    /**
     * @returns The geometry of the screen this containment is associated to
     */
    QRectF screenGeometry() const;

Q_SIGNALS:
    /**
     * This signal is emitted when a new applet is added in the containment
     * It may happen in the following situations:
     * * The user created the applet
     * * The applet was moved in from another containment
     * * The applet got restored at startup
     * @param applet the applet that has been added
     * @param geometryHint an hint to pass to the GUI on the location
     *           and size we prefer for the newly created applet;
     *           the gui might choose whether to respect or not this hint
     */
    void appletAdded(Plasma::Applet *applet, const QRectF &geometryHint);

    /**
     * This signal is emitted right before appletAdded, it can be used
     * to do a preliminary setup on the applet before the handlers of appletAdded are executed.
     * Useful for instance to prepare the GUI for the applet
     * @param applet the applet that is about to be added
     * @param geometryHint an hint to pass to the GUI on the location
     *           and size we prefer for the newly created applet;
     *           the gui might choose whether to respect or not this hint
     */
    void appletAboutToBeAdded(Plasma::Applet *applet, const QRectF &geometryHint);

    /**
     * This signal is emitted when an applet is destroyed
     */
    void appletRemoved(Plasma::Applet *applet);

    /**
     * This signal is emitted right before appletRemoved, it can be used
     * to do a preliminary setup on the applet before the handlers of appletRemoved are executed.
     * Useful for instance to prepare or teardown the GUI for the applet
     */
    void appletAboutToBeRemoved(Plasma::Applet *applet);

    /**
     * This signal is emitted when a new applet is created by the containment.
     * Compared to appletAdded, this gets emitted only when the user explicitly
     * creates a new applet, either via the widget explorer or the scripting
     * environment.
     * @see appletAdded
     * @since 5.16
     */
    void appletCreated(Plasma::Applet *applet, const QRectF &geometryHint);

    /**
     * Emitted when the list of applets has changed, either added or removed
     */
    void appletsChanged();

    /**
     * Emitted when the activity id has changed
     */
    void activityChanged(const QString &activity);

    /**
     * Emitted when the activity name has changed
     */
    void activityNameChanged(const QString &name);

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
     * Emitted when the containment disaplay hints change
     */
    void containmentDisplayHintsChanged(Plasma::Types::ContainmentDisplayHints hints);

    /**
     * Emitted when the ui has been fully loaded and is fully working
     * @param uiReady true when the ui of the containment is ready, as well the ui of each applet in it
     */
    void uiReadyChanged(bool uiReady);

    /**
     * emitted when the containment type changed
     */
    void containmentTypeChanged();

    /**
     * Emitted when the available screen rectangle has changed
     */
    void availableRelativeScreenRectChanged(const QRectF &rect);

    /**
     * Emitted when the available screen rectangle has changed
     */
    void availableRelativeScreenRegionChanged(const QList<QRectF> &region);

    /**
     * Emitted when the screen geometry has changed
     */
    void screenGeometryChanged(const QRectF &rect);

    /**
     * Emitted when the root wallpaper item has changed
     */
    void wallpaperGraphicsObjectChanged();

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

    /**
     * @internal Return root wallpaper item
     */
    QObject *wallpaperGraphicsObject() const;
    void setWallpaperGraphicsObject(QObject *object);

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
