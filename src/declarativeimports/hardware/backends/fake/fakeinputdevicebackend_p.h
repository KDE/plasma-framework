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

#ifndef HARDWARE_FAKE_INPUTDEVICE_P_H
#define HARDWARE_FAKE_INPUTDEVICE_P_H

#include <QObject>
#include <QDebug>

#include "inputdevice.h"

#include "fakeinputdevicebackend.h"
#include "utils/sharedsingleton.h"

#include <map>

namespace hardware {
namespace backends {
namespace fake {

class FakeInputDeviceBackend::Private
    : public QObject, public utils::SharedSingleton<Private>
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.SolidXTest")

public:
    Private();

    std::map<QString, std::unique_ptr<InputDevice>> devices;

Q_SIGNALS:
    void addedDevice(const QString & id);
    void removedDevice(const QString & id);

public Q_SLOTS:
    void addDevice(const QString & id, const QString & name,
            int type, int subtype);

    void removeDevice(const QString & id);

};


} // namespace fake
} // namespace backends
} // namespace hardware

#endif // HARDWARE_FAKE_INPUTDEVICE_P_H

