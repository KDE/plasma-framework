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

#include "xlibinputdevicebackend.h"
#include "xlibconnection.h"
#include "xlibinputdevicebackend_p.h"

namespace solidx {
namespace backends {
namespace xlib {

XlibInputDeviceBackend::XlibInputDeviceBackend(QObject * parent)
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

XlibInputDeviceBackend::~XlibInputDeviceBackend()
{
}

QStringList XlibInputDeviceBackend::devices() const
{
    QStringList result;

    foreach (auto item, d->internalDeviceIds) {
        result << item.first;
    }

    return result;
}

const InputDevice & XlibInputDeviceBackend::device(const QString & id) const
{
    if (!d->internalDeviceIds.count(id)) return InputDevice::null;

    auto internalId = d->internalDeviceIds[id];

    if (!d->knownDevices.count(internalId)) return InputDevice::null;

    return *(d->knownDevices[internalId].get());
}

} // namespace xlib
} // namespace backends
} // namespace solidx

