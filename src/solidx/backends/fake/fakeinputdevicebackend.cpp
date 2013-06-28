/*
 *   Copyright (C) 2013 Ivan Cukic <ivan.cukic(at)kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License version 2,
 *   or (at your option) any later version, as published by the Free
 *   Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "fakeinputdevicebackend.h"
#include "fakeinputdevicebackend_p.h"

#include <QtDBus/QDBusConnection>

namespace solidx {
namespace backends {
namespace fake {

FakeInputDeviceBackend::Private::Private()
{
    QDBusConnection::sessionBus().registerService("org.kde.SolidXTest");
    QDBusConnection::sessionBus().registerObject(
            "/FakeInput", this,
            QDBusConnection::ExportAllContents
        );

}

void FakeInputDeviceBackend::Private::addDevice(
        const QString & id, const QString & name,
        int type, int subtype)
{
    if (devices.count(id)) return;

    std::unique_ptr<InputDevice> device(new InputDevice(id, name,
                    (InputDevice::Type)type,
                    (InputDevice::Subtype)subtype
        ));

    devices[id] = std::move(device);

    emit addedDevice(id);

}

void FakeInputDeviceBackend::Private::removeDevice(const QString & id)
{
    if (!devices.count(id)) return;

    devices.erase(id);

    emit removedDevice(id);
}

FakeInputDeviceBackend::FakeInputDeviceBackend(QObject * parent)
    : AbstractInputDeviceBackend(parent), d(Private::instance())
{
    connect(
        d.get(), SIGNAL(addedDevice(QString)),
        this,    SIGNAL(addedDevice(QString))
    );
    connect(
        d.get(), SIGNAL(removedDevice(QString)),
        this,    SIGNAL(removedDevice(QString))
    );
}

FakeInputDeviceBackend::~FakeInputDeviceBackend()
{
}

QStringList FakeInputDeviceBackend::devices() const
{
    QStringList result;

    for (auto & item: d->devices) {
        result << item.first;
    }

    return result;
}

const InputDevice & FakeInputDeviceBackend::device(const QString & id) const
{
    if (!d->devices.count(id)) return InputDevice::null;

    return *(d->devices[id].get());
}

} // namespace fake
} // namespace backends
} // namespace solidx

