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

#include "utils/d_ptr_implementation.h"

#include "backends/abstractinputdevicebackend.h"
#include "backends/xlib/xlibinputdevicebackend.h"
#include "backends/fake/fakeinputdevicebackend.h"

namespace solidx {

class Interface::Private {
public:
    QList<AbstractInputDeviceBackend*> backends;

};

Interface::Interface(QObject * parent)
    : QObject(parent)
{
    d->backends
        << new backends::xlib::XlibInputDeviceBackend(this)
        << new backends::fake::FakeInputDeviceBackend(this)
    ;
}

bool Interface::isKeyboardPresent() const
{
    return true;
}

bool Interface::isTouchscreenPresent() const
{
    return true;
}

} // namespace solidx


