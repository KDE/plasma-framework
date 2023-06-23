/*
    SPDX-FileCopyrightText: 2008-2013 Aaron Seigo <aseigo@kde.org>
    SPDX-FileCopyrightText: 2010-2013 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef CONTAINMENTINTERFACE_H
#define CONTAINMENTINTERFACE_H

#include <QMenu>

#include <Plasma/Containment>

#include "appletinterface.h"

class AppletQuickItem;
class WallpaperInterface;
class DropMenu;
class KJob;

namespace KIO
{
class Job;
class DropJob;
}

namespace KActivities
{
class Info;
}

/**
 * @brief This class is exposed to containments QML as the attached property Plasmoid
 *
 * <b>Import Statement</b>
 * @code import org.kde.plasma.plasmoid @endcode
 * @version 2.0
 */
class ContainmentInterface : public AppletInterface
{
    Q_OBJECT

    Q_PROPERTY(WallpaperInterface *wallpaper READ wallpaperInterface NOTIFY wallpaperInterfaceChanged)

    /**
     * True if the UI is still loading, for instance a desktop which doesn't have its wallpaper yet
     */
    Q_PROPERTY(bool loading READ isLoading NOTIFY isLoadingChanged)

public:
    ContainmentInterface(QQuickItem *parent = nullptr);

    void classBegin() override;

    // Not for QML
    Plasma::Containment *containment() const
    {
        return m_containment.data();
    }

    inline WallpaperInterface *wallpaperInterface() const
    {
        return m_wallpaperInterface;
    }

    // For QML use
    /**
     * Returns the corresponding AppletInterface of one of its applets
     */
    Q_INVOKABLE AppletQuickItem *itemFor(Plasma::Applet *applet) const;

    /**
     * Process the mime data arrived to a particular coordinate, either with a drag and drop or paste with middle mouse button
     */
    Q_INVOKABLE void processMimeData(QMimeData *data, int x, int y, KIO::DropJob *dropJob = nullptr);

    /**
     * Process the mime data arrived to a particular coordinate, either with a drag and drop or paste with middle mouse button
     */
    Q_INVOKABLE void processMimeData(QObject *data, int x, int y, KIO::DropJob *dropJob = nullptr);

    /**
     * Search for a containment at those coordinates.
     * the coordinates are passed as local coordinates of *this* containment
     */
    Q_INVOKABLE QObject *containmentItemAt(int x, int y);

    /**
     * Map coordinates from relative to the given applet to relative to this containment
     */
    Q_INVOKABLE QPointF mapFromApplet(Plasma::Applet *applet, int x, int y);

    /**
     *Map coordinates from relative to this containment to relative to the given applet
     */
    Q_INVOKABLE QPointF mapToApplet(Plasma::Applet *applet, int x, int y);

    /**
     * Given a geometry, it adjusts it moving it completely inside of the boundaries
     * of availableScreenRegion
     * @return the toLeft point of the rectangle
     */
    Q_INVOKABLE QPointF adjustToAvailableScreenRegion(int x, int y, int w, int h) const;

    /**
     * Opens the context menu of the Corona
     *
     * @param globalPos menu position in the global coordinate system
     * @since 5.102
     */
    Q_INVOKABLE void openContextMenu(const QPointF &globalPos);

protected:
    void init() override;
    void loadWallpaper();
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

    void addAppletActions(QMenu *desktopMenu, Plasma::Applet *applet, QEvent *event);
    void addContainmentActions(QMenu *desktopMenu, QEvent *event);

    bool isLoading() const;
    void itemChange(ItemChange change, const ItemChangeData &value) override;

Q_SIGNALS:
    // Property notifiers
    void activityChanged();
    void activityNameChanged();
    void appletsChanged();
    void drawWallpaperChanged();
    void actionsChanged();
    void editModeChanged();
    void wallpaperInterfaceChanged();
    void isLoadingChanged();

private Q_SLOTS:
    // Used only internally by a metaObject()->invokeMethod
    Plasma::Applet *createApplet(const QString &plugin, const QVariantList &args, const QRectF &geom);

private:
    void dropJobResult(KJob *job);
    void mimeTypeRetrieved(KIO::Job *job, const QString &mimetype);
    void appletAddedForward(Plasma::Applet *applet, const QRectF &geometryHint);
    void appletRemovedForward(Plasma::Applet *applet);
    void clearDataForMimeJob(KIO::Job *job);
    void setAppletArgs(Plasma::Applet *applet, const QString &mimetype, const QVariant &data);
    void deleteWallpaperInterface();

    WallpaperInterface *m_wallpaperInterface;
    QList<QObject *> m_appletInterfaces;
    KActivities::Info *m_activityInfo;
    QPointer<Plasma::Containment> m_containment;
    QPointer<QMenu> m_contextMenu;
    QPointer<DropMenu> m_dropMenu;
    int m_wheelDelta;
    friend class AppletInterface;
};

#endif
