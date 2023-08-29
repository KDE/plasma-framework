/*
    SPDX-FileCopyrightText: 2023 David Edmundson <davidedmundson@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "waylandintegration_p.h"

#include <QGuiApplication>
#include <QWaylandClientExtensionTemplate>
#include <QWindow>
#include <qpa/qplatformwindow_p.h>

#include <KWindowSystem>

class PlasmaShellManager : public QWaylandClientExtensionTemplate<PlasmaShellManager>, public QtWayland::org_kde_plasma_shell
{
public:
    PlasmaShellManager()
        : QWaylandClientExtensionTemplate<PlasmaShellManager>(8)
    {
    }
};

class PlasmaShellSurface : public QtWayland::org_kde_plasma_surface
{
public:
    PlasmaShellSurface(struct ::org_kde_plasma_surface *impl)
        : QtWayland::org_kde_plasma_surface(impl)
    {
    }
    ~PlasmaShellSurface()
    {
        destroy();
    }
};

class WaylandIntegrationSingleton
{
public:
    WaylandIntegrationSingleton();
    std::unique_ptr<PlasmaShellManager> shellManager;
    QHash<QWindow *, PlasmaShellWaylandIntegration *> windows;
};

WaylandIntegrationSingleton::WaylandIntegrationSingleton()
{
    if (KWindowSystem::isPlatformWayland()) {
        shellManager = std::make_unique<PlasmaShellManager>();
    }
}

Q_GLOBAL_STATIC(WaylandIntegrationSingleton, s_waylandIntegration)

PlasmaShellWaylandIntegration *PlasmaShellWaylandIntegration::get(QWindow *window)
{
    PlasmaShellWaylandIntegration *&it = s_waylandIntegration->windows[window];
    if (!it) {
        it = new PlasmaShellWaylandIntegration(window);
    }
    return it;
}

PlasmaShellWaylandIntegration::~PlasmaShellWaylandIntegration()
{
    s_waylandIntegration->windows.remove(m_window);
}

PlasmaShellWaylandIntegration::PlasmaShellWaylandIntegration(QWindow *window)
    : QObject(window)
    , m_window(window)
{
    m_window->create();
    if (auto waylandWindow = window->nativeInterface<QNativeInterface::Private::QWaylandWindow>()) {
        connect(waylandWindow, &QNativeInterface::Private::QWaylandWindow::surfaceCreated, this, [this] {
            surfaceCreated();
        });
        connect(waylandWindow, &QNativeInterface::Private::QWaylandWindow::surfaceDestroyed, this, [this] {
            surfaceDestroyed();
        });
        if (waylandWindow->surface()) {
            surfaceCreated();
        }
    }
}

void PlasmaShellWaylandIntegration::setPosition(const QPoint &position)
{
    if (position == m_position) {
        return;
    }

    m_position = position;
    if (m_shellSurface) {
        m_shellSurface->set_position(m_position->x(), m_position->y());
    }
}

void PlasmaShellWaylandIntegration::setPanelBehavior(QtWayland::org_kde_plasma_surface::panel_behavior panelBehavior)
{
    if (panelBehavior == m_panelBehavior) {
        return;
    }
    m_panelBehavior = panelBehavior;
    if (m_shellSurface) {
        m_shellSurface->set_panel_behavior(panelBehavior);
    }
}

void PlasmaShellWaylandIntegration::setRole(QtWayland::org_kde_plasma_surface::role role)
{
    if (role == m_role) {
        return;
    }
    m_role = role;
    if (m_shellSurface) {
        m_shellSurface->set_role(role);
    }
}

void PlasmaShellWaylandIntegration::setTakesFocus(bool takesFocus)
{
    if (takesFocus == m_takesFocus) {
        return;
    }
    m_takesFocus = takesFocus;
    if (m_shellSurface) {
        m_shellSurface->set_panel_takes_focus(takesFocus);
    }
}

void PlasmaShellWaylandIntegration::surfaceCreated()
{
    struct wl_surface *surface = nullptr;

    ;
    if (!s_waylandIntegration->shellManager || !s_waylandIntegration->shellManager->isActive()) {
        return;
    }

    if (auto waylandWindow = m_window->nativeInterface<QNativeInterface::Private::QWaylandWindow>()) {
        surface = waylandWindow->surface();
    }

    if (!surface) {
        return;
    }

    m_shellSurface = std::make_unique<PlasmaShellSurface>(s_waylandIntegration->shellManager->get_surface(surface));
    if (m_shellSurface) {
        if (m_position) {
            m_shellSurface->set_position(m_position->x(), m_position->y());
        }
        m_shellSurface->set_panel_takes_focus(m_takesFocus);
        m_shellSurface->set_role(m_role);
        m_shellSurface->set_skip_switcher(true);
        m_shellSurface->set_skip_taskbar(true);
    }
}

void PlasmaShellWaylandIntegration::surfaceDestroyed()
{
    m_shellSurface.reset();
}

#include "moc_waylandintegration_p.cpp"
