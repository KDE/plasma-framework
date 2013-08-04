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

#ifndef HARDWARE_INPUTDEVICE_H
#define HARDWARE_INPUTDEVICE_H

#include <QAbstractListModel>
#include <QString>

#include "utils/d_ptr.h"

namespace hardware {

/**
 * InputDevices
 */
class InputDevice: public QObject {
    Q_OBJECT

public:
    enum class Type {
        Any          = 0,

        Keyboard     = 1,
        Pointer      = 2,

        Error        = 255
    };

    enum class Subtype {
        Any          = 0,

        Touchpad     = 1,
        Touchscreen  = 2,

        FullKeyboard = 3,
        SpecialKeys  = 4,

        Error        = 255
    };

    InputDevice(
            const QString & id,
            const QString & name,
            Type type = Type::Error,
            Subtype subtype = Subtype::Error
        );

    QString id;
    QString name;
    Type type;
    Subtype subtype;

    static InputDevice null;
};

} // namespace hardware

#endif /* HARDWARE_INPUTDEVICE_H */

