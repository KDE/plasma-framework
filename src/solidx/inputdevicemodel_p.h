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

#ifndef SOLIDX_INPUTDEVICEMODEL_P_H
#define SOLIDX_INPUTDEVICEMODEL_P_H

#include "inputdevicemodel.h"

#include "backends/xlib/inputdevicebackend.h"

namespace solidx {

class InputDeviceModel::Private: public QObject {
    Q_OBJECT

public:
    Private(InputDeviceModel * parent);
    void reload();

public Q_SLOTS:
    void addedDevice(const QString & id);
    void removedDevice(const QString & id);

public:
    QStringList devices;
    backends::xlib::InputDeviceBackend backendXLib;

    InputDevice::Type type;
    InputDevice::Subtype subtype;

    bool initialized;
    InputDeviceModel * const q;
};

} // namespace solidx

#endif /* SOLIDX_INPUTDEVICEMODEL_P_H */

