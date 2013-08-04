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

#ifndef HARDWARE_FAKE_INPUTDEVICE_H
#define HARDWARE_FAKE_INPUTDEVICE_H

#include "../abstractinputdevicebackend.h"

#include <memory>

namespace hardware {
namespace backends {
namespace fake {

class FakeInputDeviceBackend: public AbstractInputDeviceBackend {
    Q_OBJECT

public:
    explicit FakeInputDeviceBackend(QObject *parent = 0);
    ~FakeInputDeviceBackend();

    QStringList devices() const Q_DECL_OVERRIDE;
    const InputDevice & device(const QString & id) const Q_DECL_OVERRIDE;

Q_SIGNALS:
    void addedDevice(const QString & id);
    void removedDevice(const QString & id);

private:
    class Private;
    friend class Private;
    const std::shared_ptr<Private> d;
};

} // namespace fake
} // namespace backends
} // namespace hardware

#endif /* HARDWARE_FAKE_INPUTDEVICE_H */

