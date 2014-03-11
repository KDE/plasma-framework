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

namespace KIO {
    class Job;
}

namespace KActivities {
    class Info;
}

class ContainmentInterface : public AppletInterface
{
    Q_OBJECT

    /**
     * List of applets this containment has: the containments
     */
    Q_PROPERTY(QList <QObject *> applets READ applets NOTIFY appletsChanged)
    
    /**
     * Type of this containment TODO: notify
     */
    Q_PROPERTY(Plasma::Types::ContainmentType containmentType READ containmentType WRITE setContainmentType NOTIFY containmentTypeChanged)

    /**
     * Activity name of this containment
     */
    Q_PROPERTY(QString activity READ activity NOTIFY activityChanged)

    /**
     * Activity name of this containment
     */
    Q_PROPERTY(QString activityName READ activityName NOTIFY activityNameChanged)

    /**
     * Actions associated to this containment or corona
     */
    Q_PROPERTY(QList<QObject*> actions READ actions NOTIFY actionsChanged)

public:
    ContainmentInterface(DeclarativeAppletScript *parent);
//Not for QML
    Plasma::Containment *containment() const { return static_cast<Plasma::Containment *>(appletScript()->applet()->containment()); }

    inline WallpaperInterface *wallpaperInterface() const { return m_wallpaperInterface;}

//For QML use
    QList<QObject *> applets();

    Plasma::Types::ContainmentType containmentType() const;
    void setContainmentType(Plasma::Types::ContainmentType type);

    QString activity() const;
    QString activityName() const;

    QList<QObject*> actions() const;

    /**
     * FIXME: either a property or not accessible at all. Lock or unlock widgets
     */
    Q_INVOKABLE void lockWidgets(bool locked);

    /**
     * Geometry of this screen
     */
    Q_INVOKABLE QRectF screenGeometry(int id) const;

    /**
     * The available region of this screen, panels excluded. It's a list of rectangles
     */
    Q_INVOKABLE QVariantList availableScreenRegion(int id) const;

    /**
     * The available rect of this screen, panels excluded. A simple rect area
     * For more precise available geometry use availableScreenRegion()
     */
    Q_INVOKABLE QRect availableScreenRect(int id) const;

    /**
     * Process the mime data arrived to a particular coordinate, either with a drag and drop or paste with middle mouse button
     */
    Q_INVOKABLE void processMimeData(QMimeData *data, int x, int y);

    static ContainmentInterface *qmlAttachedProperties(QObject *object)
    {
        return qobject_cast<ContainmentInterface *>(AppletQuickItem::qmlAttachedProperties(object));
    }

protected:
    void init();
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void wheelEvent(QWheelEvent *event);

    void addAppletActions(QMenu &desktopMenu, Plasma::Applet *applet, QEvent *event);
    void addContainmentActions(QMenu &desktopMenu, QEvent *event);

Q_SIGNALS:
    /**
     * Emitted when an applet is added
     * @param applet the applet object: it's a qml graphical object and an instance of AppletInterface
     * @param x coordinate containment relative
     * @param y coordinate containment relative
     */
    void appletAdded(QObject *applet, int x, int y);

    /**
     * Emitted when an applet is removed
     * @param applet the applet object: it's a qml graphical object and an instance of AppletInterface.
     *               It's still valid, even if it will be deleted shortly
     */
    void appletRemoved(QObject *applet);

    //Property notifiers
    void activityChanged();
    void activityNameChanged();
    void availableScreenRegionChanged();
    void availableScreenRectChanged();
    void appletsChanged();
    void drawWallpaperChanged();
    void containmentTypeChanged();
    ///void immutableChanged();
    void actionsChanged();

protected Q_SLOTS:
    void appletAddedForward(Plasma::Applet *applet);
    void appletRemovedForward(Plasma::Applet *applet);
    void loadWallpaper();
    void dropJobResult(KJob *job);
    void mimeTypeRetrieved(KIO::Job *job, const QString &mimetype);

private Q_SLOTS:
    Plasma::Applet *addApplet(const QString &plugin, const QVariantList &args, const QPoint &pos);

private:
    void clearDataForMimeJob(KIO::Job *job);
    void setAppletArgs(Plasma::Applet *applet, const QString &mimetype, const QString &data);

    WallpaperInterface *m_wallpaperInterface;
    QList<QObject *> m_appletInterfaces;
    QHash<KJob*, QPoint> m_dropPoints;
    QHash<KJob*, QMenu*> m_dropMenus;
    KActivities::Info *m_activityInfo;
};

QML_DECLARE_TYPEINFO(ContainmentInterface, QML_HAS_ATTACHED_PROPERTIES)

#endif
