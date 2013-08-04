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

#ifndef HARDWARE_INTERFACE_P_H
#define HARDWARE_INTERFACE_P_H

#include "interface.h"

#include <QObject>
#include <QList>
#include "backends/abstractinputdevicebackend.h"

namespace hardware {

class Interface::Private: public QObject {
    Q_OBJECT

public:
    Private(Interface * parent);
    void addedDevice(AbstractInputDeviceBackend * backend, const QString & id);

public Q_SLOTS:
    void addedDevice(const QString & id);
    void removedDevice(const QString & id);

Q_SIGNALS:
    void keyboardPresenceChanged(bool present);
    void touchscreenPresenceChanged(bool present);

private:
    Interface * const q;

public:
    int numKeyboard;
    int numTouchscreen;

    QList<AbstractInputDeviceBackend*> backends;
};

} // namespace hardware


#endif /* HARDWARE_INTERFACE_P_H */

