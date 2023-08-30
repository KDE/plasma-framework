/*
    SPDX-FileCopyrightText: 2023 David Edmundson <davidedmundson@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef WAYLANDINTEGRATION_P_H
#define WAYLANDINTEGRATION_P_H

#include <QObject>
#include <QPoint>
#include <memory>

#include "qwayland-plasma-shell.h"

class QWindow;

class PlasmaShellSurface;

/**
 * @brief The PlasmaWaylandShellIntegration class exposes Plasma specific
 * specific wayland extensions for
 *
 * The class abstracts the wayland protocol tasks, automatically sending
 * cached metadata when the underlying platform surfaces are created.
 */
class PlasmaShellWaylandIntegration : public QObject
{
    Q_OBJECT
public:
    /**
     * Returns the relevant PlasmaWaylandShellIntegration instance for this window
     * creating one if needed.
     *
     * A valid instance will always returned, it will no-op on unsupported platforms
     */
    static PlasmaShellWaylandIntegration *get(QWindow *window);
    ~PlasmaShellWaylandIntegration();
    void setPosition(const QPoint &position);
    void setPanelBehavior(QtWayland::org_kde_plasma_surface::panel_behavior panelBehavior);
    void setRole(QtWayland::org_kde_plasma_surface::role role);
    void setTakesFocus(bool takesFocus);

    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    PlasmaShellWaylandIntegration(QWindow *window);
    void platformWindowCreated();
    void surfaceCreated();
    void surfaceDestroyed();

    QWindow *m_window = nullptr;
    std::optional<QPoint> m_position;
    QtWayland::org_kde_plasma_surface::panel_behavior m_panelBehavior = QtWayland::org_kde_plasma_surface::panel_behavior_always_visible;
    QtWayland::org_kde_plasma_surface::role m_role = QtWayland::org_kde_plasma_surface::role_normal;
    bool m_takesFocus = false;
    std::unique_ptr<PlasmaShellSurface> m_shellSurface;
};

#endif // WAYLANDINTEGRATION_P_H
