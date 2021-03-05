/*
    SPDX-FileCopyrightText: 2020 Vlad Zahorodnii <vlad.zahorodnii@kde.org>

    Based on WaylandIntegration from kwayland-integration

    SPDX-FileCopyrightText: 2014 Martin Gräßlin <mgraesslin@kde.org>
    SPDX-FileCopyrightText: 2015 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "debug_p.h"
#include "waylandintegration_p.h"

#include <KWayland/Client/connection_thread.h>
#include <KWayland/Client/plasmashell.h>
#include <KWayland/Client/registry.h>

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
        const KWayland::Client::Registry::AnnouncedInterface interface = m_registry->interface(KWayland::Client::Registry::Interface::PlasmaShell);

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
