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

#ifndef CORONA_H
#define CORONA_H

#include <QtGui/QGraphicsScene>

#include <plasma/applet.h>
#include <plasma/plasma.h>
#include <plasma/plasma_export.h>

namespace Plasma
{

class Containment;

/**
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
     * The applets and Corona surface are changeable or not
     **/
    bool isImmutable() const;

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
     * Returns the the config file used to store the configuration for this Corona
     */
    KSharedConfig::Ptr config() const;

public Q_SLOTS:
    /**
     * Load applet layout from a config file
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
     * Called when there have been changes made to configuration that should be saved
     * to disk at the next convenient moment
     */
    void scheduleConfigSync() const;

    /**
     * Adds a Containment to the Corona
     *
     * @param name the plugin name for the containment, as given by
     *        KPluginInfo::pluginName(). If an empty string is passed in, the defalt
     *        containment plugin will be used (usually DesktopContainment). If the
     *        string literal "null" is passed in, then no plugin will be loaded and
     *        a simple Containment object will be created instead.
     * @param args argument list to pass to the containment
     * @param id to assign to this containment, or 0 to auto-assign it a new id
     * @param geometry where to place the containment, or to auto-place it if an invalid
     *                 is provided
     *
     * @return a pointer to the containment on success, or 0 on failure
     */
    Containment* addContainment(const QString& name, const QVariantList& args = QVariantList(),
                                uint id = 0, bool delayInit = false);

    /**
     * Removes a given containment from the corona
     */
    void destroyContainment(Containment *containment);

    /**
     * Returns the Containment, if any, for a given physical screen
     *
     * @param screen number of the physical screen to locate
     */
    Containment* containmentForScreen(int screen) const;

    /**
     * @return The type of immutability of this applet
     */
    ImmutabilityType immutability() const;

    /**
     * Sets the immutability type for this applet (not immutable, user immutable or system immutable)
     * @arg immutable the new immutability type of this applet
     */
    void setImmutability(const ImmutabilityType immutable);

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

protected:
    /**
     * Loads the default (system wide) layout for this user
     **/
    virtual void loadDefaultLayout();

    //Reimplemented from QGraphicsScene
    void dragEnterEvent(QGraphicsSceneDragDropEvent* event);
    void dragLeaveEvent(QGraphicsSceneDragDropEvent* event);
    void dragMoveEvent(QGraphicsSceneDragDropEvent* event);

protected Q_SLOTS:
    void containmentDestroyed(QObject*);
    void syncConfig();

private:
    class Private;
    Private * const d;
};

} // namespace Plasma

#endif


