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

#ifndef HARDWARE_XLIB_INPUTDEVICE_P_H
#define HARDWARE_XLIB_INPUTDEVICE_P_H

#include <QObject>
#include <QDebug>

#include "inputdevice.h"

#include "xlibinputdevicebackend.h"
#include "xlibconnection.h"
#include "utils/sharedsingleton.h"

#include <functional>
#include <iterator>
#include <algorithm>
#include <map>
#include <set>

#include <X11/X.h>
#include <X11/extensions/XInput.h>
#include <X11/extensions/XInput2.h>
#include <X11/extensions/XIproto.h>

// Somebody thought it was a smart thing to define
// a 'Bool' macro in XLib, which screws up with the
// QMetaType::Bool
#include <fixx11h.h>
#undef Bool
#undef None

namespace hardware {
namespace backends {
namespace xlib {

class XlibInputDeviceBackend::Private
    : public QObject, public utils::SharedSingleton<Private>
{
    Q_OBJECT

    friend class XlibInputDeviceBackend;

public:
    /**
     * RAII class to handle the list of X Input devices, and
     * to provide an iterator-enabled interface
     */
    class XDeviceInfoList;

    /**
     * Applies a function to all the input devices
     */
    template <typename F>
    void processInputDevices(F what) const;

    /**
     * Applies a function to the input device with the specified
     * id. Returns true if such a device has been found
     */
    template <typename F>
    bool processInputDevice(int id, F what) const;

    /**
     * Process the event sent by the X server
     */
    void processEvent(const _XEvent & event);

    /**
     * Constructs an XAtom that represents the property
     * with the given name
     */
    inline
    Atom propertyAtom(const char * name);

    Private();

    /**
     * Tests whether the device has the specified property
     */
    inline
    bool hasProperty(XIDeviceInfo * info, Atom property);

    /**
     * Checks which properties a specific device has
     * Note: std::set used instead of QSet because of std::inserter
     * TODO: benchmark against std::vector
     */
    inline
    std::set<Atom> presentProperties(const XDeviceInfo & device,
            const std::vector<Atom> & propertiesToCheck);

    /**
     * Adding the device to the list and
     * sends the appropriate event
     */
    void addDevice(const XDeviceInfo & device);

    /**
     * Removing the device from the list and
     * sends the appropriate event
     */
    void removeDevice(XID id);

Q_SIGNALS:
    void addedDevice(const QString & id);
    void removedDevice(const QString & id);

private:
    XlibConnection connection;

    // QMap doesn't work with the unique_ptr
    std::map<XID, std::unique_ptr<InputDevice>> knownDevices;
    std::map<QString, XID> internalDeviceIds;

    const Atom synapticsCapabilities;
    const Atom synapticsOff;
    const Atom touchMaxContacts;
    const Atom touchAxesPerContact;
    const Atom deviceNode;

};


} // namespace xlib
} // namespace backends
} // namespace hardware

#endif // HARDWARE_XLIB_INPUTDEVICE_P_H

