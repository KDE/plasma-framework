/*
    SPDX-FileCopyrightText: 2020 Vlad Zahorodnii <vlad.zahorodnii@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef WAYLANDINTEGRATION_P_H
#define WAYLANDINTEGRATION_P_H

#include <QObject>
#include <QPointer>

namespace KWayland
{
namespace Client
{
class PlasmaShell;
class Registry;
}
}

class WaylandIntegration : public QObject
{
    Q_OBJECT

public:
    explicit WaylandIntegration(QObject *parent = nullptr);
    ~WaylandIntegration() override;

    KWayland::Client::PlasmaShell *waylandPlasmaShell();

    static WaylandIntegration *self();

private:
    void setupKWaylandIntegration();

    QPointer<KWayland::Client::Registry> m_registry;
    QPointer<KWayland::Client::PlasmaShell> m_waylandPlasmaShell;

    Q_DISABLE_COPY(WaylandIntegration)
};

#endif // WAYLANDINTEGRATION_P_H
