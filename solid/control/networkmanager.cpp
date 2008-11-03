/*  This file is part of the KDE project
    Copyright (C) 2006 Will Stephenson <wstephenson@kde.org>
    Copyright (C) 2006-2007 Kevin Ottens <ervin@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.

*/

#include "ifaces/networkmanager.h"
#include "ifaces/networkinterface.h"
#include "ifaces/wirednetworkinterface.h"
#include "ifaces/wirelessnetworkinterface.h"
#include "ifaces/networkgsminterface.h"
#include "ifaces/networkcdmainterface.h"

#include "soliddefs_p.h"
#include "networkmanager_p.h"
#include "networkinterface.h"
#include "wirednetworkinterface.h"
#include "wirelessnetworkinterface.h"

#include "networkmanager.h"

#include <kglobal.h>

#include <kdebug.h>

K_GLOBAL_STATIC(Solid::Control::NetworkManagerPrivate, globalNetworkManager)

Solid::Control::NetworkManagerPrivate::NetworkManagerPrivate() : m_invalidDevice(0)
{
    loadBackend("Network Management",
                "SolidNetworkManager",
                "Solid::Control::Ifaces::NetworkManager");

    if (managerBackend()!=0) {
        connect(managerBackend(), SIGNAL(networkInterfaceAdded(const QString &)),
                this, SLOT(_k_networkInterfaceAdded(const QString &)));
        connect(managerBackend(), SIGNAL(networkInterfaceRemoved(const QString &)),
                this, SLOT(_k_networkInterfaceRemoved(const QString &)));
        connect(managerBackend(), SIGNAL(statusChanged(Solid::Networking::Status)),
                this, SIGNAL(statusChanged(Solid::Networking::Status)));
        connect(managerBackend(), SIGNAL(wirelessEnabledChanged(bool)),
                this, SIGNAL(wirelessEnabledChanged(bool)));
        connect(managerBackend(), SIGNAL(wirelessHardwareEnabledChanged(bool)),
                this, SIGNAL(wirelessHardwareEnabledChanged(bool)));
        connect(managerBackend(), SIGNAL(activeConnectionsChanged()),
                this, SIGNAL(activeConnectionsChanged()));
    }
}

Solid::Control::NetworkManagerPrivate::~NetworkManagerPrivate()
{
    // Delete all the devices, they are now outdated
    typedef QPair<NetworkInterface *, QObject *> NetworkInterfaceIfacePair;

    foreach (const NetworkInterfaceIfacePair &pair, m_networkInterfaceMap) {
        delete pair.first;
        delete pair.second;
    }

    m_networkInterfaceMap.clear();
}

Solid::Control::NetworkInterfaceList Solid::Control::NetworkManagerPrivate::buildDeviceList(const QStringList &uniList)
{
    NetworkInterfaceList list;
    Ifaces::NetworkManager *backend = qobject_cast<Ifaces::NetworkManager *>(managerBackend());

    if (backend == 0) return list;

    foreach (const QString &uni, uniList)
    {
        QPair<NetworkInterface *, QObject *> pair = findRegisteredNetworkInterface(uni);

        if (pair.first!= 0)
        {
            list.append(pair.first);
        }
    }

    return list;
}

Solid::Control::NetworkInterfaceList Solid::Control::NetworkManagerPrivate::networkInterfaces()
{
    Ifaces::NetworkManager *backend = qobject_cast<Ifaces::NetworkManager *>(managerBackend());

    if (backend!= 0)
    {
        return buildDeviceList(backend->networkInterfaces());
    }
    else
    {
        return NetworkInterfaceList();
    }
}

Solid::Control::NetworkInterfaceList Solid::Control::NetworkManager::networkInterfaces()
{
    return globalNetworkManager->networkInterfaces();
}

bool Solid::Control::NetworkManager::isNetworkingEnabled()
{
    return_SOLID_CALL(Ifaces::NetworkManager *, globalNetworkManager->managerBackend(), false, isNetworkingEnabled());
}

bool Solid::Control::NetworkManager::isWirelessEnabled()
{
    return_SOLID_CALL(Ifaces::NetworkManager *, globalNetworkManager->managerBackend(), false, isWirelessEnabled());
}

bool Solid::Control::NetworkManager::isWirelessHardwareEnabled()
{
    return_SOLID_CALL(Ifaces::NetworkManager *, globalNetworkManager->managerBackend(), false, isWirelessHardwareEnabled());
}

void Solid::Control::NetworkManager::setNetworkingEnabled(bool enabled)
{
    SOLID_CALL(Ifaces::NetworkManager *, globalNetworkManager->managerBackend(), setNetworkingEnabled(enabled));
}

void Solid::Control::NetworkManager::setWirelessEnabled(bool enabled)
{
    SOLID_CALL(Ifaces::NetworkManager *, globalNetworkManager->managerBackend(), setWirelessEnabled(enabled));
}

Solid::Networking::Status Solid::Control::NetworkManager::status()
{
    return_SOLID_CALL(Ifaces::NetworkManager *, globalNetworkManager->managerBackend(), Solid::Networking::Unknown, status());
}

Solid::Control::NetworkInterface * Solid::Control::NetworkManagerPrivate::findNetworkInterface(const QString &uni)
{
    Ifaces::NetworkManager *backend = qobject_cast<Ifaces::NetworkManager *>(managerBackend());

    if (backend == 0) return 0;

    QPair<NetworkInterface *, QObject *> pair = findRegisteredNetworkInterface(uni);

    if (pair.first != 0)
    {
        return pair.first;
    }
    else
    {
        return 0;
    }
}

Solid::Control::NetworkInterface * Solid::Control::NetworkManager::findNetworkInterface(const QString &uni)
{
    return globalNetworkManager->findNetworkInterface(uni);
}

Solid::Control::NetworkManager::Notifier * Solid::Control::NetworkManager::notifier()
{
    return globalNetworkManager;
}

void Solid::Control::NetworkManagerPrivate::_k_networkInterfaceAdded(const QString &uni)
{
    QPair<NetworkInterface *, QObject*> pair = m_networkInterfaceMap.take(uni);

    if (pair.first!= 0)
    {
        // Oops, I'm not sure it should happen...
        // But well in this case we'd better kill the old device we got, it's probably outdated

        delete pair.first;
        delete pair.second;
    }

    emit networkInterfaceAdded(uni);
}

void Solid::Control::NetworkManagerPrivate::_k_networkInterfaceRemoved(const QString &uni)
{
    QPair<NetworkInterface *, QObject *> pair = m_networkInterfaceMap.take(uni);

    if (pair.first!= 0)
    {
        delete pair.first;
        delete pair.second;
    }

    emit networkInterfaceRemoved(uni);
}

void Solid::Control::NetworkManagerPrivate::_k_destroyed(QObject *object)
{
    Ifaces::NetworkInterface *device = qobject_cast<Ifaces::NetworkInterface *>(object);

    if (device!=0)
    {
        QString uni = device->uni();
        QPair<NetworkInterface *, QObject *> pair = m_networkInterfaceMap.take(uni);
        delete pair.first;
    }
}

/***************************************************************************/

QPair<Solid::Control::NetworkInterface *, QObject *>
Solid::Control::NetworkManagerPrivate::findRegisteredNetworkInterface(const QString &uni)
{
    if (m_networkInterfaceMap.contains(uni)) {
        return m_networkInterfaceMap[uni];
    } else {
        Ifaces::NetworkManager *backend = qobject_cast<Ifaces::NetworkManager *>(managerBackend());

        if (backend!=0)
        {
            QObject * iface = backend->createNetworkInterface(uni);
            NetworkInterface *device = 0;
            if (qobject_cast<Ifaces::WirelessNetworkInterface *>(iface) != 0) {
                device = new WirelessNetworkInterface(iface);
            } else if (qobject_cast<Ifaces::WiredNetworkInterface *>(iface) != 0) {
                device = new WiredNetworkInterface(iface);
            } else if (qobject_cast<Ifaces::GsmNetworkInterface *>(iface) != 0) {
                device = new GsmNetworkInterface(iface);
            } else if (qobject_cast<Ifaces::CdmaNetworkInterface *>(iface) != 0) {
                device = new CdmaNetworkInterface(iface);
            } else {
                kDebug() << "Unhandled network interface: " << uni;
            }
            if (device != 0) {
                QPair<NetworkInterface *, QObject *> pair(device, iface);
                connect(iface, SIGNAL(destroyed(QObject *)),
                        this, SLOT(_k_destroyed(QObject *)));
                m_networkInterfaceMap[uni] = pair;
                return pair;
            }
            else
            {
                return QPair<NetworkInterface *, QObject *>(0, 0);
            }
        }
        else
        {
            return QPair<NetworkInterface *, QObject *>(0, 0);
        }
    }
}

void Solid::Control::NetworkManager::activateConnection(const QString & interfaceUni, const QString & connectionUni,
                const QVariantMap & connectionParameters )
{
    SOLID_CALL(Ifaces::NetworkManager *, globalNetworkManager->managerBackend(), activateConnection(interfaceUni, connectionUni, connectionParameters));
}

void Solid::Control::NetworkManager::deactivateConnection(const QString & activeConnectionUni)
{
    SOLID_CALL(Ifaces::NetworkManager *, globalNetworkManager->managerBackend(), deactivateConnection(activeConnectionUni));
}

QStringList Solid::Control::NetworkManager::activeConnections()
{
    return_SOLID_CALL(Ifaces::NetworkManager *, globalNetworkManager->managerBackend(), QStringList(), activeConnections());
}

#include "networkmanager_p.moc"
#include "networkmanager.moc"
