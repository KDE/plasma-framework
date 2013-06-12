/*
 *   Copyright 2007 Aaron Seigo <aseigo@kde.org>
 *   Copyright 2007 Matt Broadstone <mbroadst@gmail.com>
 *   Copyright 2012 Marco MArtin <mart@kde.org>
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

#include <plasma/containment.h>
#include <plasma/plasma.h>
#include <plasma/plasma_export.h>

class QAction;

class KAction;

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

public:
    explicit Corona(QObject * parent = 0);
    ~Corona();

    /**
     * Accessor for the associated Package object if any.
     * A Corona package defines how Containments are laid out in a View,
     * ToolBoxes, default layout, error messages
     * and in genelal all the furniture specific of a particular
     * device form factor.
     *
     * @return the Package object, or an invalid one if none
     * @since 5.0
     **/
    Plasma::Package package() const;

    /**
     * @return all containments on this Corona
     */
    QList<Containment*> containments() const;

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
     * when widgets are locked, or if the requested containment plugin can not be located
     * or successfully loaded.
     */
    Containment *createContainment(const QString &name, const QVariantList &args = QVariantList());

    /**
     * Returns the Containment, if any, for a given physical screen and desktop
     *
     * @param screen number of the physical screen to locate
     * @param desktop the virtual desktop) to locate; if < 0 then it will
     *        simply return the first Containment associated with screen
     */
    Containment *containmentForScreen(int screen) const;

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
     * This method is useful in order to retrieve the list of available
     * screen edges for panel type containments.
     * @param screen the id of the screen to look for free edges.
     * @returns a list of free edges not filled with panel type containments.
     */
    QList<Plasma::Types::Location> freeEdges(int screen) const;

    /**
     * The actions assocated with this Corona
     */
    KActionCollection* actions() const;

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
    void exportLayout(KConfigGroup &config, QList<Containment*> containments);

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
     * @return The type of immutability of this Corona
     */
    Types::ImmutabilityType immutability() const;

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
     * This signal indicates that the configuration file was flushed to disk.
     */
    void configSynced();

    /**
     * This signal indicates that a change in available screen gemetry occurred.
     */
    void availableScreenRegionChanged();

    /**
     * emitted when immutability changes.
     * this is for use by things that don't get constraints events, like plasmaapp.
     * it's NOT for containments or applets or any of the other stuff on the scene.
     * if your code's not in shells/ it probably shouldn't be using it.
     */
    void immutabilityChanged(Plasma::Types::ImmutabilityType immutability);

protected:
    /**
     * Loads the default (system wide) layout for this user
     **/
    virtual void loadDefaultLayout();

private:
    CoronaPrivate *const d;

    Q_PRIVATE_SLOT(d, void containmentDestroyed(QObject*))
    Q_PRIVATE_SLOT(d, void syncConfig())
    Q_PRIVATE_SLOT(d, void toggleImmutability())

    friend class CoronaPrivate;
    friend class View;
};

} // namespace Plasma

#endif

