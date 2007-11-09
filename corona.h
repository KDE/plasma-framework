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
     * Adds an applet to the default Containment
     *
     * @param name the plugin name for the applet, as given by 
     *        KPluginInfo::pluginName()
     * @param args argument list to pass to the plasmoid
     * @param id to assign to this applet, or 0 to auto-assign it a new id
     * @param geometry where to place the applet, or to auto-place it if an invalid
     *                 is provided
     *
     * @return a pointer to the applet on success, or 0 on failure
     */
    Applet* addApplet(const QString& name, const QVariantList& args = QVariantList(),
                      uint id = 0, const QRectF &geometry = QRectF(-1, -1, -1, -1));

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

protected:
    void dragEnterEvent(QGraphicsSceneDragDropEvent* event);
    void dragLeaveEvent(QGraphicsSceneDragDropEvent* event);
    void dragMoveEvent(QGraphicsSceneDragDropEvent* event);
    //void contextMenuEvent(QGraphicsSceneContextMenuEvent *contextMenuEvent);

protected Q_SLOTS:
    void containmentDestroyed(QObject*);
    void screenResized(int);

private:
    class Private;
    Private * const d;
};

} // namespace Plasma

#endif


