/*
 *  Copyright 2020 Vlad Zahorodnii <vlad.zahorodnii@kde.org>
 *
 *  Based on WaylandIntegration from kwayland-integration
 *
 *  Copyright 2014 Martin Gräßlin <mgraesslin@kde.org>
 *  Copyright 2015 Marco Martin <mart@kde.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "waylandintegration_p.h"
#include "debug_p.h"

#include <KWayland/Client/connection_thread.h>
#include <KWayland/Client/plasmashell.h>
#include <KWayland/Client/registry.h>
#include <KWayland/Client/shadow.h>
#include <KWayland/Client/shm_pool.h>

#include <QCoreApplication>

class WaylandIntegrationSingleton
{
public:
    WaylandIntegration self;
};

Q_GLOBAL_STATIC(WaylandIntegrationSingleton, privateWaylandIntegrationSelf)

WaylandIntegration::WaylandIntegration(QObject *parent)
    : QObject(parent)
{
    setupKWaylandIntegration();
}

WaylandIntegration::~WaylandIntegration()
{
}

KWayland::Client::PlasmaShell *WaylandIntegration::waylandPlasmaShell()
{
    if (!m_waylandPlasmaShell && m_registry) {
        const KWayland::Client::Registry::AnnouncedInterface interface =
                m_registry->interface(KWayland::Client::Registry::Interface::PlasmaShell);

        if (interface.name == 0) {
            qCWarning(LOG_PLASMAQUICK) << "The compositor does not support the plasma shell protocol";
            return nullptr;
        }

        m_waylandPlasmaShell = m_registry->createPlasmaShell(interface.name, interface.version, qApp);

        connect(m_waylandPlasmaShell, &KWayland::Client::PlasmaShell::removed, this, [this]() {
            m_waylandPlasmaShell->deleteLater();
        });
    }

    return m_waylandPlasmaShell;
}

KWayland::Client::ShadowManager *WaylandIntegration::waylandShadowManager()
{
    if (!m_waylandShadowManager && m_registry) {
        const KWayland::Client::Registry::AnnouncedInterface interface =
                m_registry->interface(KWayland::Client::Registry::Interface::Shadow);

        if (interface.name == 0) {
            qCWarning(LOG_PLASMAQUICK) << "The compositor does not support the shadow protocol";
            return nullptr;
        }

        m_waylandShadowManager = m_registry->createShadowManager(interface.name, interface.version, qApp);

        connect(m_waylandShadowManager, &KWayland::Client::ShadowManager::removed, this, [this]() {
            m_waylandShadowManager->deleteLater();
        });
    }

    return m_waylandShadowManager;
}

KWayland::Client::ShmPool *WaylandIntegration::waylandShmPool()
{
    if (!m_waylandShmPool && m_registry) {
        const KWayland::Client::Registry::AnnouncedInterface interface =
                m_registry->interface(KWayland::Client::Registry::Interface::Shm);

        if (interface.name == 0) {
            return nullptr;
        }

        m_waylandShmPool = m_registry->createShmPool(interface.name, interface.version, qApp);

        connect(m_waylandShmPool, &KWayland::Client::ShmPool::removed, this, [this]() {
            m_waylandShmPool->deleteLater();
        });
    }

    return m_waylandShmPool;
}

WaylandIntegration *WaylandIntegration::self()
{
    return &privateWaylandIntegrationSelf()->self;
}

void WaylandIntegration::setupKWaylandIntegration()
{
    KWayland::Client::ConnectionThread *connection = KWayland::Client::ConnectionThread::fromApplication(this);
    if (!connection) {
        return;
    }

    m_registry = new KWayland::Client::Registry(qApp);
    m_registry->create(connection);
    m_registry->setup();

    connection->roundtrip();
}
