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
    explicit Corona(const QRectF & sceneRect, QObject * parent = 0);
    Corona(qreal x, qreal y, qreal width, qreal height, QObject * parent = 0);
    ~Corona();

    /**
     * The applets and Corona surface are changeable or not
     **/
    bool isImmutable() const;

    /**
     * @return true if this Corona is immutable due to Kiosk settings
     */
    bool isKioskImmutable() const;

    /**
     * A rect containing the maximum size a plasmoid on this corona should
     * consider being.
     **/
    QRectF maxSizeHint() const;

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
     * Save applets to a config file
     *
     * @param config the name of the config file to save to
     */
    void saveApplets(const QString &config) const;

    /**
     * Load applet layout from a config file
     *
     * @param config the name of the config file to load from
     */
    void loadApplets(const QString &config);

    /**
     * Loads the default (system wide) layout for this user
     **/
    virtual void loadDefaultSetup();

    /**
     * Clear the Corona from all applets.
     */
    void clearApplets();

    /**
     * Returns the the config file used to store the configuration for this Corona
     */
    KSharedConfig::Ptr config() const;

public Q_SLOTS:
    /**
     * Load applets from the default config file
     */
    void loadApplets();

    /**
     * Save applets to the default config file
     */
    void saveApplets() const;

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
     * Returns all containments on this Corona
     */
    QList<Containment*> containments() const;

    /**
     * Sets if the applets are Immutable
     */
    void setImmutable(bool immutable_);

Q_SIGNALS:
    /**
     * This signal indicates that a application launch was triggered
     */
    void launchActivated();

    /**
     * This signal indicates that a new screen is being handled
     * at runtime (e.g. after first creation)
     *
     * @param screen the xinerama number of the new screen
     */
    void newScreen(int screen);

protected:
    void dragEnterEvent(QGraphicsSceneDragDropEvent* event);
    void dragLeaveEvent(QGraphicsSceneDragDropEvent* event);
    void dragMoveEvent(QGraphicsSceneDragDropEvent* event);
    //void contextMenuEvent(QGraphicsSceneContextMenuEvent *contextMenuEvent);

protected Q_SLOTS:
    void containmentDestroyed(QObject*);
    void screenResized(int);
    void syncConfig();

private:
    class Private;
    Private * const d;
};

} // namespace Plasma

#endif


