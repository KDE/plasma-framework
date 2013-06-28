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

#ifndef SOLIDX_INPUTDEVICE_H
#define SOLIDX_INPUTDEVICE_H

#include <QAbstractListModel>
#include <QString>

#include "utils/d_ptr.h"

namespace solidx {

/**
 * InputDevices
 */
struct InputDevice {

    QString id;
    QString name;

    enum class Type {
        Any          = 0,

        Keyboard     = 1,
        Pointer      = 2,

        Error        = 255
    } type;

    enum class Subtype {
        Any          = 0,

        Touchpad     = 1,
        Touchscreen  = 2,

        FullKeyboard = 3,
        SpecialKeys  = 4,

        Error        = 255
    } subtype;

};

} // namespace solidx

#endif /* SOLIDX_INPUTDEVICE_H */

