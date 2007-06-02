/*
 *   Copyright (C) 2007 Matt Broadstone <mbroadst@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License version 2 as
 *   published by the Free Software Foundation
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

#include <QGraphicsScene>

#include "applet.h"
#include "plasma.h"
#include "plasma/plasma_export.h"

namespace Plasma
{

class Layout;
class Svg;

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
    explicit Corona(qreal x, qreal y, qreal width, qreal height, QObject * parent = 0);
    ~Corona();

    /**
     * The location of the Corona. @see Plasma::Location
     */
    Plasma::Location location() const;

    /**
     * The current form factor for this Corona. @see Plasma::FormFactor
     **/
    Plasma::FormFactor formFactor() const;

    /**
     * A rect containing the maximum size a plasmoid on this corona should
     * consider being.
     **/
    QRectF maxSizeHint() const;

public Q_SLOTS:
    /**
     * Informs the Corona as to what position it is in. This is informational
     * only, as the Corona doesn't change it's actual location. This is,
     * however, passed on to Applets that may be managed by this Corona.
     *
     * @param location the new location of this Corona
     */
    void setLocation(Plasma::Location location);

    /**
     * Sets the form factor for this Corona. This may cause changes in both
     * the arrangement of Applets as well as the display choices of individual
     * Applets.
     */
    void setFormFactor(Plasma::FormFactor formFactor);

    /**
     * Adds a plasmoid applet to the scene
     *
     * @param name the plugin name for the applet, as given by 
     *        KPluginInfo::pluginName()
     */
    void addPlasmoid(const QString& name);

    /**
     * Adds a SuperKaramba theme to the scene
     *
     * @param path the path to the theme file
     */
    void addKaramba(const KUrl& path);

protected:
    void dragEnterEvent(QGraphicsSceneDragDropEvent* event);
    void dragLeaveEvent(QGraphicsSceneDragDropEvent* event);
    void dragMoveEvent(QGraphicsSceneDragDropEvent* event);
    void dropEvent(QGraphicsSceneDragDropEvent* event);
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *contextMenuEvent);

protected Q_SLOTS:
    void launchExplorer();
    void appletDestroyed(QObject*);

private:
    void init();

    class Private;
    Private * const d;
};

} // namespace Plasma

#endif


