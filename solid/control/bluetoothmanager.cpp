/*  This file is part of the KDE project
    Copyright (C) 2006 Will Stephenson <wstephenson@kde.org>
    Copyright (C) 2006 Kévin Ottens <ervin@kde.org>
    Copyright (C) 2007 Daniel Gollub <dgollub@suse.de>
    Copyright (C) 2008 Tom Patzig <tpatzig@suse.de>


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

#include <QMap>
#include <QPair>
#include <QStringList>

#include "ifaces/bluetoothmanager.h"
#include "ifaces/bluetoothinterface.h"
#include "ifaces/bluetoothinputdevice.h"
#include "ifaces/bluetoothsecurity.h"

#include "soliddefs_p.h"
#include "managerbase_p.h"

#include "bluetoothinterface.h"
#include "bluetoothmanager.h"
#include "bluetoothsecurity.h"

#include <kdebug.h>

namespace Solid
{
namespace Control
{
class BluetoothManagerPrivate : public ManagerBasePrivate
{
public:
    BluetoothManagerPrivate(BluetoothManager *parent)
        : q(parent) { }

    BluetoothManager * const q;

    QPair<BluetoothInterface *, Ifaces::BluetoothInterface *> findRegisteredBluetoothInterface(const QString &ubi) const;
    QPair<BluetoothInputDevice *, Ifaces::BluetoothInputDevice *> findRegisteredBluetoothInputDevice(const QString &ubi) const;

    void connectBackend(QObject *newBackend);

    void _k_interfaceAdded(const QString &ubi);
    void _k_interfaceRemoved(const QString &ubi);
    void _k_interfaceDestroyed(QObject *object);
/*
    void _k_inputDeviceCreated(const QString &ubi);
    void _k_inputDeviceRemoved(const QString &ubi);
    void _k_inputDeviceDestroyed(QObject *object);
*/

    mutable QMap<QString, QPair<BluetoothInterface *, Ifaces::BluetoothInterface *> > bluetoothInterfaceMap;
    mutable QMap<QString, QPair<BluetoothInputDevice *, Ifaces::BluetoothInputDevice *> > bluetoothInputDeviceMap;

    BluetoothInterface invalidInterface;
    BluetoothInputDevice invalidInputDevice;
};
}
}

SOLID_SINGLETON_IMPLEMENTATION(Solid::Control::BluetoothManager, BluetoothManager)


Solid::Control::BluetoothManager::BluetoothManager()
        : QObject(), d(new BluetoothManagerPrivate(this))
{
    d->loadBackend("Bluetooth Management",
                   "SolidBluetoothManager",
                   "Solid::Control::Ifaces::BluetoothManager");

    if (d->managerBackend() != 0) {
        d->connectBackend(d->managerBackend());
    }
}

Solid::Control::BluetoothManager::~BluetoothManager()
{
    // Delete all the interfaces, they are now outdated
    typedef QPair<BluetoothInterface *, Ifaces::BluetoothInterface *> BluetoothInterfaceIfacePair;

    // Delete all the devices, they are now outdated
    foreach (const BluetoothInterfaceIfacePair &pair, d->bluetoothInterfaceMap) {
        delete pair.first;
        delete pair.second;
    }

    d->bluetoothInterfaceMap.clear();
}
/*
Solid::Control::BluetoothInterfaceList Solid::Control::BluetoothManager::buildDeviceList(const QStringList  & ubiList) const
{
    BluetoothInterfaceList list;
    Ifaces::BluetoothManager *backend = qobject_cast<Ifaces::BluetoothManager *>(d->managerBackend());

    if (backend == 0) return list;

    foreach (const QString &ubi, ubiList) {
        QPair<BluetoothInterface *, Ifaces::BluetoothInterface *> pair = d->findRegisteredBluetoothInterface(ubi);

        if (pair.first != 0) {
            list.append(*pair.first);
        }
    }

    return list;
}
*/
/*
Solid::Control::BluetoothInterfaceList Solid::Control::BluetoothManager::bluetoothInterfaces() const
{
    Ifaces::BluetoothManager *backend = qobject_cast<Ifaces::BluetoothManager *>(d->managerBackend());

    if (backend != 0) {
        return buildDeviceList(backend->bluetoothInterfaces());
    } else {
        return BluetoothInterfaceList();
    }
}
*/
QStringList Solid::Control::BluetoothManager::bluetoothInterfaces() const
{
    return_SOLID_CALL(Ifaces::BluetoothManager *, d->managerBackend(), QStringList(), bluetoothInterfaces());
}

QString Solid::Control::BluetoothManager::defaultInterface() const
{
    return_SOLID_CALL(Ifaces::BluetoothManager *, d->managerBackend(), QString(), defaultInterface());
}

Solid::Control::BluetoothInterface Solid::Control::BluetoothManager::findBluetoothInterface(const QString &ubi) const
{
    Ifaces::BluetoothManager *backend = qobject_cast<Ifaces::BluetoothManager *>(d->managerBackend());

    if (backend == 0) return d->invalidInterface;

    QPair<BluetoothInterface *, Ifaces::BluetoothInterface *> pair = d->findRegisteredBluetoothInterface(ubi);

    if (pair.first != 0) {
        return *pair.first;
    } else {
        return d->invalidInterface;
    }
}

QString Solid::Control::BluetoothManager::findInterface(const QString &dev) const
{
    return_SOLID_CALL(Ifaces::BluetoothManager *, d->managerBackend(), QString(), findInterface(dev));
}

/*
Solid::Control::BluetoothInputDevice Solid::Control::BluetoothManager::findBluetoothInputDevice(const QString &ubi) const
{
    Ifaces::BluetoothManager *backend = qobject_cast<Ifaces::BluetoothManager *>(d->managerBackend());

    if (backend == 0) return d->invalidInputDevice;

    QPair<BluetoothInputDevice *, Ifaces::BluetoothInputDevice *> pair = d->findRegisteredBluetoothInputDevice(ubi);

    if (pair.first != 0) {
        return *pair.first;
    } else {
        return d->invalidInputDevice;
    }
}
*/

/*
Solid::Control::BluetoothInputDevice* Solid::Control::BluetoothManager::createBluetoothInputDevice(const QString &ubi)
{
        Ifaces::BluetoothManager *backend = qobject_cast<Ifaces::BluetoothManager *>(d->managerBackend());
        Ifaces::BluetoothInputDevice *iface = 0;
	if (backend != 0) {
	    iface = qobject_cast<Ifaces::BluetoothInputDevice *>(backend->createBluetoothInputDevice(ubi));
        }
	if (iface != 0) {
            BluetoothInputDevice *device = new BluetoothInputDevice(iface);
	    return device;

	} else {
	    return &d->invalidInputDevice;
	}

}

KJob *Solid::Control::BluetoothManager::setupInputDevice(const QString &ubi)
{
    return_SOLID_CALL(Ifaces::BluetoothManager *, d->managerBackend(), 0, setupInputDevice(ubi));
}

Solid::Control::BluetoothInputDeviceList Solid::Control::BluetoothManager::bluetoothInputDevices() const
{
    BluetoothInputDeviceList list;
    Ifaces::BluetoothManager *backend = qobject_cast<Ifaces::BluetoothManager *>(d->managerBackend());

    if (backend == 0) return list;

    QStringList ubis = backend->bluetoothInputDevices();

    foreach (const QString &ubi, ubis) {
        QPair<BluetoothInputDevice *, Ifaces::BluetoothInputDevice *> pair = d->findRegisteredBluetoothInputDevice(ubi);
		
        if (pair.first != 0) {
            list.append(*pair.first);
        }
    }
    return list;
}

Solid::Control::BluetoothSecurity *Solid::Control::BluetoothManager::security(const QString &interface)
{
    Ifaces::BluetoothManager *backendManager = qobject_cast<Ifaces::BluetoothManager*>(d->managerBackend());
    if (backendManager!=0) {
        Ifaces::BluetoothSecurity *backendSecurity = backendManager->security(interface);
        return new Solid::Control::BluetoothSecurity(backendSecurity);
    }
    return 0;
}

void Solid::Control::BluetoothManager::removeInputDevice(const QString &ubi)
{
    SOLID_CALL(Ifaces::BluetoothManager *, d->managerBackend(), removeInputDevice(ubi));
}
*/
void Solid::Control::BluetoothManagerPrivate::_k_interfaceAdded(const QString &ubi)
{
    QPair<BluetoothInterface *, Ifaces::BluetoothInterface *> pair = bluetoothInterfaceMap.take(ubi);

    if (pair.first != 0) {
        // Oops, I'm not sure it should happen...
        // But well in this case we'd better kill the old device we got, it's probably outdated

        delete pair.first;
        delete pair.second;
    }

    emit q->interfaceAdded(ubi);
}

void Solid::Control::BluetoothManagerPrivate::_k_interfaceRemoved(const QString &ubi)
{
    QPair<BluetoothInterface *, Ifaces::BluetoothInterface *> pair = bluetoothInterfaceMap.take(ubi);

    if (pair.first != 0) {
        delete pair.first;
        delete pair.second;
    }

    emit q->interfaceRemoved(ubi);
}

void Solid::Control::BluetoothManagerPrivate::_k_interfaceDestroyed(QObject *object)
{
    Ifaces::BluetoothInterface *device = qobject_cast<Ifaces::BluetoothInterface *>(object);

    if (device != 0) {
        QString ubi = device->ubi();
        QPair<BluetoothInterface *, Ifaces::BluetoothInterface *> pair = bluetoothInterfaceMap.take(ubi);
        delete pair.first;
    }
}

/*
void Solid::Control::BluetoothManagerPrivate::_k_inputDeviceCreated(const QString &ubi)
{
    QPair<BluetoothInputDevice *, Ifaces::BluetoothInputDevice *> pair = bluetoothInputDeviceMap.take(ubi);

    if (pair.first != 0) {
        // Oops, I'm not sure it should happen...
        // But well in this case we'd better kill the old device we got, it's probably outdated

        delete pair.first;
        delete pair.second;
    }

    emit q->inputDeviceCreated(ubi);
}

void Solid::Control::BluetoothManagerPrivate::_k_inputDeviceRemoved(const QString &ubi)
{
    QPair<BluetoothInputDevice *, Ifaces::BluetoothInputDevice *> pair = bluetoothInputDeviceMap.take(ubi);

    if (pair.first != 0) {
        delete pair.first;
        delete pair.second;
    }

    emit q->inputDeviceRemoved(ubi);
}

void Solid::Control::BluetoothManagerPrivate::_k_inputDeviceDestroyed(QObject *object)
{
    Ifaces::BluetoothInputDevice *device = qobject_cast<Ifaces::BluetoothInputDevice *>(object);

    if (device != 0) {
        QString ubi = device->ubi();
        QPair<BluetoothInputDevice *, Ifaces::BluetoothInputDevice *> pair = bluetoothInputDeviceMap.take(ubi);
        delete pair.first;
    }
}
*/

/***************************************************************************/

void Solid::Control::BluetoothManagerPrivate::connectBackend(QObject *newBackend)
{
    QObject::connect(newBackend, SIGNAL(interfaceAdded(const QString &)),
                     q, SLOT(_k_interfaceAdded(const QString &)));
    QObject::connect(newBackend, SIGNAL(interfaceRemoved(const QString &)),
                     q, SLOT(_k_interfaceRemoved(const QString &)));
/*
    QObject::connect(newBackend, SIGNAL(inputDeviceCreated(const QString &)),
                     q, SLOT(_k_inputDeviceCreated(const QString &)));
    QObject::connect(newBackend, SIGNAL(inputDeviceRemoved(const QString &)),
                     q, SLOT(_k_inputDeviceRemoved(const QString &)));
*/

}

QPair<Solid::Control::BluetoothInterface *, Solid::Control::Ifaces::BluetoothInterface *> Solid::Control::BluetoothManagerPrivate::findRegisteredBluetoothInterface(const QString &ubi) const
{
    if (bluetoothInterfaceMap.contains(ubi)) {
        return bluetoothInterfaceMap[ubi];
    } else {
        Ifaces::BluetoothManager *backend = qobject_cast<Ifaces::BluetoothManager *>(managerBackend());
        Ifaces::BluetoothInterface *iface = 0;

        if (backend != 0) {
            iface = qobject_cast<Ifaces::BluetoothInterface *>(backend->createInterface(ubi));
        }

        if (iface != 0) {
            BluetoothInterface *device = new BluetoothInterface(iface);
            QPair<BluetoothInterface *, Ifaces::BluetoothInterface *> pair(device, iface);
            QObject::connect(iface, SIGNAL(destroyed(QObject *)),
                             q, SLOT(_k_interfaceDestroyed(QObject *)));
            bluetoothInterfaceMap[ubi] = pair;
            return pair;
        } else {
            return QPair<BluetoothInterface *, Ifaces::BluetoothInterface *>(0, 0);
        }
    }
}

/*
QPair<Solid::Control::BluetoothInputDevice *, Solid::Control::Ifaces::BluetoothInputDevice *> Solid::Control::BluetoothManagerPrivate::findRegisteredBluetoothInputDevice(const QString &ubi) const
{

    if (bluetoothInputDeviceMap.contains(ubi)) {
        return bluetoothInputDeviceMap[ubi];
    } else {
        Ifaces::BluetoothManager *backend = qobject_cast<Ifaces::BluetoothManager *>(managerBackend());
        Ifaces::BluetoothInputDevice *iface = 0;

        if (backend != 0) {
	    iface = qobject_cast<Ifaces::BluetoothInputDevice *>(backend->createBluetoothInputDevice(ubi));
        }

        if (iface != 0) {
            BluetoothInputDevice *device = new BluetoothInputDevice(iface);
            QPair<BluetoothInputDevice *, Ifaces::BluetoothInputDevice *> pair(device, iface);
            bluetoothInputDeviceMap[ubi] = pair;
            QObject::connect(iface, SIGNAL(destroyed(QObject *)),
                             q, SLOT(_k_inputDeviceDestroyed(QObject *)));
            return pair;
        } else {
            return QPair<BluetoothInputDevice *, Ifaces::BluetoothInputDevice *>(0, 0);
        }
    }
}
*/




#include "bluetoothmanager.moc"
