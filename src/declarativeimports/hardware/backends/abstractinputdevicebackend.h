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

#ifndef HARDWARE_ABSTRACT_INPUTDEVICE_H
#define HARDWARE_ABSTRACT_INPUTDEVICE_H

#include "inputdevice.h"

#include <QStringList>

namespace hardware {

class AbstractInputDeviceBackend: public QObject {
    Q_OBJECT

public:
    explicit AbstractInputDeviceBackend(QObject *parent = 0);
    ~AbstractInputDeviceBackend();

    virtual
    QStringList devices() const = 0;

    virtual
    const InputDevice & device(const QString & id) const = 0;

//  Q_SIGNALS:
//      void addedDevice(const QString & id);
//      void removedDevice(const QString & id);

};

} // namespace hardware

#endif /* HARDWARE_ABSTRACT_INPUTDEVICE_H */

