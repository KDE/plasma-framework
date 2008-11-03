/*
Copyright 2008 Will Stephenson <wstephenson@kde.org>

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) version 3, or any
later version accepted by the membership of KDE e.V. (or its
successor approved by the membership of KDE e.V.), which shall
act as a proxy defined in Section 6 of version 3 of the license.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public 
License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "wirelessaccesspoint.h"
#include "frontendobject_p.h"
#include "soliddefs_p.h"
#include "ifaces/wirelessaccesspoint.h"

namespace Solid
{
namespace Control
{
    class AccessPointPrivate : public FrontendObjectPrivate
    {
    public:
        AccessPointPrivate(QObject *parent)
            : FrontendObjectPrivate(parent) { }

        void setBackendObject(QObject *object);
    };
}
}

Solid::Control::AccessPoint::AccessPoint(QObject *backendObject)
    : QObject(), d_ptr(new AccessPointPrivate(this))
{
    Q_D(AccessPoint);
    d->setBackendObject(backendObject);
}

Solid::Control::AccessPoint::AccessPoint(const AccessPoint &network)
    : QObject(), d_ptr(new AccessPointPrivate(this))
{
    Q_D(AccessPoint);
    d->setBackendObject(network.d_ptr->backendObject());
}

Solid::Control::AccessPoint::AccessPoint(AccessPointPrivate &dd, QObject *backendObject)
    : QObject(), d_ptr(&dd)
{
    Q_D(AccessPoint);
    d->setBackendObject(backendObject);
}

Solid::Control::AccessPoint::AccessPoint(AccessPointPrivate &dd, const AccessPoint &ap)
    : d_ptr(&dd)
{
    Q_D(AccessPoint);
    d->setBackendObject(ap.d_ptr->backendObject());
}

Solid::Control::AccessPoint::~AccessPoint()
{
    delete d_ptr;
}

QString Solid::Control::AccessPoint::uni() const
{
    Q_D(const AccessPoint);
    return_SOLID_CALL(Ifaces::AccessPoint *, d->backendObject(), QString(), uni());
}

Solid::Control::AccessPoint::Capabilities Solid::Control::AccessPoint::capabilities() const
{
    Q_D(const AccessPoint);
    return_SOLID_CALL(Ifaces::AccessPoint *, d->backendObject(), 0, capabilities());

}

Solid::Control::AccessPoint::WpaFlags Solid::Control::AccessPoint::wpaFlags() const
{
    Q_D(const AccessPoint);
    return_SOLID_CALL(Ifaces::AccessPoint *, d->backendObject(), 0, wpaFlags());

}

Solid::Control::AccessPoint::WpaFlags Solid::Control::AccessPoint::rsnFlags() const
{
    Q_D(const AccessPoint);
    return_SOLID_CALL(Ifaces::AccessPoint *, d->backendObject(), 0, rsnFlags());

}

QString Solid::Control::AccessPoint::ssid() const
{
    Q_D(const AccessPoint);
    return_SOLID_CALL(Ifaces::AccessPoint *, d->backendObject(), QString(), ssid());
}

uint Solid::Control::AccessPoint::frequency() const
{
    Q_D(const AccessPoint);
    return_SOLID_CALL(Ifaces::AccessPoint *, d->backendObject(), 0, frequency());

}

QString Solid::Control::AccessPoint::hardwareAddress() const
{
    Q_D(const AccessPoint);
    return_SOLID_CALL(Ifaces::AccessPoint *, d->backendObject(), QString(), hardwareAddress());

}

uint Solid::Control::AccessPoint::maxBitRate() const
{
    Q_D(const AccessPoint);
    return_SOLID_CALL(Ifaces::AccessPoint *, d->backendObject(), 0, maxBitRate());
}

Solid::Control::WirelessNetworkInterface::OperationMode Solid::Control::AccessPoint::mode() const
{
    Q_D(const AccessPoint);
    return_SOLID_CALL(Ifaces::AccessPoint *, d->backendObject(), (Solid::Control::WirelessNetworkInterface::OperationMode)0, mode());
}

int Solid::Control::AccessPoint::signalStrength() const
{
    Q_D(const AccessPoint);
    return_SOLID_CALL(Ifaces::AccessPoint *, d->backendObject(), 0, signalStrength());
}

void Solid::Control::AccessPointPrivate::setBackendObject(QObject *object)
{
    FrontendObjectPrivate::setBackendObject(object);

    if (object) {
        QObject::connect(object, SIGNAL(signalStrengthChanged(int)),
                parent(), SIGNAL(signalStrengthChanged(int)));
        QObject::connect(object, SIGNAL(bitRateChanged(int)),
                parent(), SIGNAL(bitRateChanged(int)));
        QObject::connect(object, SIGNAL(wpaFlagsChanged(Solid::Control::AccessPoint::WpaFlags)),
                parent(), SIGNAL(wpaFlagsChanged(Solid::Control::AccessPoint::WpaFlags)));
        QObject::connect(object, SIGNAL(rsnFlagsChanged(Solid::Control::AccessPoint::WpaFlags)),
                parent(), SIGNAL(rsnFlagsChanged(Solid::Control::AccessPoint::WpaFlags)));
        QObject::connect(object, SIGNAL(ssidChanged(const QString&)),
                parent(), SIGNAL(ssidChanged(const QString&)));
        QObject::connect(object, SIGNAL(frequencyChanged(uint)),
                parent(), SIGNAL(frequencyChanged(uint)));
    }
}

void Solid::Control::AccessPoint::_k_destroyed(QObject *object)
{
    Q_UNUSED(object);
    // nothing to do yet
}

#include "wirelessaccesspoint.moc"
