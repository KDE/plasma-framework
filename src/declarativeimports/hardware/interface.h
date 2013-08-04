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

#ifndef HARDWARE_INTERFACE_H
#define HARDWARE_INTERFACE_H

#include <QObject>
#include <QString>

#include "utils/d_ptr.h"

namespace hardware {

/**
 * InputDevices
 */
class Interface: public QObject {
    Q_OBJECT

    Q_PROPERTY(bool keyboardPresent READ isKeyboardPresent NOTIFY keyboardPresenceChanged);
    Q_PROPERTY(bool touchscreenPresent READ isTouchscreenPresent NOTIFY touchscreenPresenceChanged);

public:
    Interface(QObject * parent = Q_NULLPTR);
    ~Interface();

    bool isKeyboardPresent() const;
    bool isTouchscreenPresent() const;

Q_SIGNALS:
    void keyboardPresenceChanged(bool present);
    void touchscreenPresenceChanged(bool present);

private:
    D_PTR;
};

} // namespace hardware

#endif /* HARDWARE_INTERFACE_H */

