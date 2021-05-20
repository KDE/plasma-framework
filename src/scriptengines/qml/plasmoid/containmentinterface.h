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

class WallpaperInterface;
class DropMenu;

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
 * @class ContainmentInterface
 *
 * @brief This class is exposed to containments QML as the attached property Plasmoid
 *
 *
 */
class ContainmentInterface : public AppletInterface
{
    Q_OBJECT

    /**
     * List of applets this containment has: the containments
     * KF6: this should be AppletQuickItem *
     */
    Q_PROPERTY(QList<QObject *> applets READ applets NOTIFY appletsChanged)

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
    Q_PROPERTY(QList<QObject *> actions READ actions NOTIFY actionsChanged)

    /**
     * True when the Plasma Shell is in an edit mode that allows to move
     * things around: it's different from userConfiguring as it's about
     * editing plasmoids inside the containment, rather than the containment
     * settings dialog itself.
     * This is global for the whole Plasma process, all containments will have the same value for editMode
     */
    Q_PROPERTY(bool editMode READ isEditMode WRITE setEditMode NOTIFY editModeChanged)

    Q_PROPERTY(WallpaperInterface* wallpaper READ wallpaperInterface NOTIFY wallpaperInterfaceChanged)

    Q_PROPERTY(Plasma::Types::ContainmentDisplayHints containmentDisplayHints READ containmentDisplayHints WRITE setContainmentDisplayHints NOTIFY
                   containmentDisplayHintsChanged)

public:
    ContainmentInterface(DeclarativeAppletScript *parent, const QVariantList &args = QVariantList());

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
    QList<QObject *> applets();

    Plasma::Types::ContainmentType containmentType() const;
    void setContainmentType(Plasma::Types::ContainmentType type);

    QString activity() const;
    QString activityName() const;

    QList<QObject *> actions() const;

    void setContainmentDisplayHints(Plasma::Types::ContainmentDisplayHints hints);

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
    Q_INVOKABLE QObject *containmentAt(int x, int y);

    /**
     * Add an existing applet to this containment.
     * The coordinates are passed as local coordinates of this containment
     */
    Q_INVOKABLE void addApplet(AppletInterface *applet, int x, int y);

    /**
     * Map coordinates from relative to the given applet to relative to this containment
     */
    Q_INVOKABLE QPointF mapFromApplet(AppletInterface *applet, int x, int y);

    /**
     *Map coordinates from relative to this containment to relative to the given applet
     */
    Q_INVOKABLE QPointF mapToApplet(AppletInterface *applet, int x, int y);

    /**
     * Given a geometry, it adjusts it moving it completely inside of the boundaries
     * of availableScreenRegion
     * @return the toLeft point of the rectangle
     */
    Q_INVOKABLE QPointF adjustToAvailableScreenRegion(int x, int y, int w, int h) const;

    /**
     * @returns a named action from global Corona's actions
     */
    Q_INVOKABLE QAction *globalAction(QString name) const;

    bool isEditMode() const;
    void setEditMode(bool edit);

    static ContainmentInterface *qmlAttachedProperties(QObject *object)
    {
        return qobject_cast<ContainmentInterface *>(AppletQuickItem::qmlAttachedProperties(object));
    }

protected:
    void init() override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

    void addAppletActions(QMenu *desktopMenu, Plasma::Applet *applet, QEvent *event);
    void addContainmentActions(QMenu *desktopMenu, QEvent *event);

    virtual bool isLoading() const override;

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

    // Property notifiers
    void activityChanged();
    void activityNameChanged();
    void appletsChanged();
    void drawWallpaperChanged();
    void containmentTypeChanged();
    void actionsChanged();
    void editModeChanged();
    void wallpaperInterfaceChanged();

protected Q_SLOTS:
    void appletAddedForward(Plasma::Applet *applet);
    void appletRemovedForward(Plasma::Applet *applet);
    void loadWallpaper();
    void dropJobResult(KJob *job);
    void mimeTypeRetrieved(KIO::Job *job, const QString &mimetype);

private Q_SLOTS:
    Plasma::Applet *createApplet(const QString &plugin, const QVariantList &args, const QPoint &pos);
    Plasma::Applet *createApplet(const QString &plugin, const QVariantList &args, const QRectF &geom);

private:
    void clearDataForMimeJob(KIO::Job *job);
    void setAppletArgs(Plasma::Applet *applet, const QString &mimetype, const QString &data);

    WallpaperInterface *m_wallpaperInterface;
    QList<QObject *> m_appletInterfaces;
    KActivities::Info *m_activityInfo;
    QPointer<Plasma::Containment> m_containment;
    QPointer<QMenu> m_contextMenu;
    QPointer<DropMenu> m_dropMenu;
    int m_wheelDelta;
    friend class AppletInterface;
};

QML_DECLARE_TYPEINFO(ContainmentInterface, QML_HAS_ATTACHED_PROPERTIES)

#endif
