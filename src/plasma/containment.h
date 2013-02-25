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

#include <QObject>
#include <QWidget>

#include <kplugininfo.h>
#include <ksharedconfig.h>

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
    Q_PROPERTY(QString wallpaper READ wallpaper WRITE setWallpaper NOTIFY wallpaperChanged)

    public:
        /**
         * @param parent the QObject this applet is parented to
         * @param serviceId the name of the .desktop file containing the
         *      information about the widget
         * @param containmentId a unique id used to differentiate between multiple
         *      instances of the same Applet type
         */
        explicit Containment(QObject *parent = 0,
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
        Plasma::ContainmentType containmentType() const;

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
        Applet *addApplet(const QString &name, const QVariantList &args = QVariantList());

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
         * Sets the physical screen this Containment is associated with.
         *
         * @param screen the screen number this containment is the desktop for, or -1
         *               if it is not serving as the desktop for any screen
         * @param desktop the virtual desktop to also associate this this screen with
         */
        void setScreen(int screen);

        /**
         * @return the screen number this containment is serving as the desktop for
         *         or -1 if none
         */
        int screen() const;

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
         * Return whether wallpaper is painted or not.
         */
        bool drawWallpaper();

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
        void addContainmentActions(const QString &trigger, const QString &pluginName);

        /**
         * @return All the loaded containment action plugins, indexed by trigger name
         * @since 5.0
         */
        QHash<QString, ContainmentActions*> &containmentActions();


Q_SIGNALS:
        /**
         * This signal is emitted when a new applet is created by the containment
         */
        void appletAdded(Plasma::Applet *applet);

        /**
         * This signal is emitted when an applet is destroyed
         */
        void appletRemoved(Plasma::Applet *applet);

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
         * Emitted when the wallpaper plugin is changed
         */
        void wallpaperChanged();

        /**
         * Emitted when the location has changed
         * @since 5.0
         */
        void locationChanged(Plasma::Location location);

        /**
         * Emitted when the formFactor has changed
         * @since 5.0
         */
        void formFactorChanged(Plasma::FormFactor formFactor);

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
         * @reimp
         * @sa Applet::showConfigurationInterface()
         */
        void showConfigurationInterface();

        /**
         * Sets the type of this containment.
         */
        void setContainmentType(Plasma::ContainmentType type);

        /**
         * Sets whether wallpaper is painted or not.
         */
        void setDrawWallpaper(bool drawWallpaper);

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
         * @param args a list of strings containing two entries: the service id
         *      and the applet id
         * @since 4.3
         */
        Containment(const QString &packagePath, uint appletId, const QVariantList &args);

        Q_PRIVATE_SLOT(d, void appletDeleted(Plasma::Applet*))
        Q_PRIVATE_SLOT(d, void triggerShowAddWidgets())
        Q_PRIVATE_SLOT(d, void requestConfiguration())
        Q_PRIVATE_SLOT(d, void checkStatus(Plasma::ItemStatus))

        friend class Applet;
        friend class AppletPrivate;
        friend class CoronaPrivate;
        friend class ContainmentPrivate;
        friend class ContainmentActions;
        ContainmentPrivate *const d;
};

} // Plasma namespace

#endif // multiple inclusion guard
