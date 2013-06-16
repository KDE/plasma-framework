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

#include "input.h"

#include <X11/extensions/XInput.h>

#include <iostream>

namespace backends {
namespace xlib {

void Input::test() const
{
    // Listing input devices
    int  device_count = -1;

    auto devices = XListInputDevices(connection.display(), &device_count);

    if (devices) for (int i = 0; i < device_count; i++) {
        auto device = devices[i];

        std::cerr
            << device.name << " "
            << device.use << " "
            << device.id << std::endl;
    }

    XFreeDeviceList(devices);

}

} // namespace xlib
} // namespace backends



