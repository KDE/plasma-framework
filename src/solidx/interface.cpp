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

#include "interface.h"
#include "interface_p.h"

#include "utils/d_ptr_implementation.h"

#include "backends/xlib/xlibinputdevicebackend.h"
#include "backends/fake/fakeinputdevicebackend.h"

#include <QDebug>

namespace solidx {

Interface::Private::Private(Interface * parent)
    : q(parent),
      numKeyboard(0),
      numTouchscreen(0)
{
    backends
        << new backends::xlib::XlibInputDeviceBackend(this)
        << new backends::fake::FakeInputDeviceBackend(this)
    ;

    foreach (const auto & backend, backends) {
        connect(backend, SIGNAL(addedDevice(QString)),
            this, SLOT(addedDevice(QString)));
        connect(backend, SIGNAL(removedDevice(QString)),
            this, SLOT(removedDevice(QString)));

        foreach(const auto & device, backend->devices()) {
            addedDevice(backend, device);
        }
    }

}

void Interface::Private::addedDevice(const QString & id)
{
    auto backend = static_cast<AbstractInputDeviceBackend*>(sender());

    addedDevice(backend, id);
}

void Interface::Private::addedDevice(
        AbstractInputDeviceBackend * backend,
        const QString & id)
{
    auto & device = backend->device(id);

    qDebug() << "added device " << id;

    if (device.type == InputDevice::Type::Keyboard &&
            device.subtype == InputDevice::Subtype::FullKeyboard
    ) {
        qDebug() << "added a keyboard";
        if (++numKeyboard == 1) {
            q->keyboardPresenceChanged(true);
        }
    }

    else if (device.type == InputDevice::Type::Pointer &&
            device.subtype == InputDevice::Subtype::Touchscreen
    ) {
        qDebug() << "added a touchscreen";
        if (++numTouchscreen == 1) {
            q->touchscreenPresenceChanged(true);
        }
    }
}

void Interface::Private::removedDevice(const QString & id)
{
    auto backend = static_cast<AbstractInputDeviceBackend*>(sender());
    auto & device = backend->device(id);

    qDebug() << "removed device " << id;

    if (device.type == InputDevice::Type::Keyboard &&
            device.subtype == InputDevice::Subtype::FullKeyboard
    ) {
        qDebug() << "removed a keyboard";
        if (--numKeyboard == 0) {
            q->keyboardPresenceChanged(false);
        }
    }

    else if (device.type == InputDevice::Type::Pointer &&
            device.subtype == InputDevice::Subtype::Touchscreen
    ) {
        qDebug() << "removed a touchscreen";
        if (--numTouchscreen == 0) {
            q->touchscreenPresenceChanged(false);
        }
    }

}

Interface::Interface(QObject * parent)
    : QObject(parent), d(this)
{
}

Interface::~Interface()
{
}

bool Interface::isKeyboardPresent() const
{
    return d->numKeyboard > 0;
}

bool Interface::isTouchscreenPresent() const
{
    return d->numTouchscreen > 0;
}

} // namespace solidx


