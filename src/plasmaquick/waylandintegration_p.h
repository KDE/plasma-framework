/*
 *  Copyright 2020 Vlad Zahorodnii <vlad.zahorodnii@kde.org>
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
