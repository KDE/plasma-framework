/*
 *   Copyright 2008-2013 Aaron Seigo <aseigo@kde.org>
 *   Copyright 2010-2013 Marco Martin <mart@kde.org>
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

#ifndef CONTAINMENTINTERFACE_H
#define CONTAINMENTINTERFACE_H

#include <QMenu>

#include <Plasma/Containment>

#include "appletinterface.h"

class QmlObject;
class WallpaperInterface;

class ContainmentInterface : public AppletInterface
{
    Q_OBJECT
    Q_PROPERTY(QList <QObject *> applets READ applets NOTIFY appletsChanged)
    Q_PROPERTY(bool drawWallpaper READ drawWallpaper WRITE setDrawWallpaper)
    Q_PROPERTY(Type containmentType READ containmentType WRITE setContainmentType)
    Q_PROPERTY(int screen READ screen NOTIFY screenChanged)
    Q_PROPERTY(QString activity READ activity NOTIFY activityChanged)
    Q_ENUMS(Type)

public:
    enum Type {
        NoContainmentType = -1,  /**< @internal */
        DesktopContainment = 0,  /**< A desktop containment */
        PanelContainment,        /**< A desktop panel */
        CustomContainment = 127, /**< A containment that is neither a desktop nor a panel
                                    but something application specific */
        CustomPanelContainment = 128 /**< A customized desktop panel */
    };
    ContainmentInterface(DeclarativeAppletScript *parent);
//Not for QML
    inline Plasma::Containment *containment() const { return static_cast<Plasma::Containment *>(m_appletScriptEngine->applet()->containment()); }

    inline WallpaperInterface *wallpaperInterface() const { return m_wallpaperInterface;}

//For QML use
    QList<QObject *> applets();

    void setDrawWallpaper(bool drawWallpaper);
    bool drawWallpaper();
    Type containmentType() const;
    void setContainmentType(Type type);
    int screen() const;

    QString activity() const;

    Q_INVOKABLE QRectF screenGeometry(int id) const;
    Q_INVOKABLE QVariantList availableScreenRegion(int id) const;

protected:
    void init();
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);

    void addAppletActions(QMenu &desktopMenu, Plasma::Applet *applet, QEvent *event);
    void addContainmentActions(QMenu &desktopMenu, QEvent *event);

Q_SIGNALS:
    void appletAdded(QObject *applet);
    void appletRemoved(QObject *applet);
    void screenChanged();
    void activityChanged();
    void availableScreenRegionChanged();
    void appletsChanged();

protected Q_SLOTS:
    void appletAddedForward(Plasma::Applet *applet);
    void appletRemovedForward(Plasma::Applet *applet);
    void loadWallpaper();

private:
    WallpaperInterface *m_wallpaperInterface;
    QList<QObject *> m_appletInterfaces;
};

#endif
