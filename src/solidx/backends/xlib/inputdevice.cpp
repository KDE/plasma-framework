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

#include "inputdevice.h"
#include "sharedsingleton_p.h"
#include "connection.h"

#include <QDebug>
#include <X11/extensions/XInput.h>

namespace backends {
namespace xlib {

class InputDeviceList::Private
    : public SharedSingleton<Private>
{
public:
    /**
     * RAII class to handle the list of X Input devices, and
     * to provide an iterator-enabled interface
     */
    class XDeviceInfoList {
    public:
        XDeviceInfoList(Display * display)
            : m_deviceCount(-1),
              m_start(XListInputDevices(display, &m_deviceCount))
        {
        }

        ~XDeviceInfoList() { XFreeDeviceList(m_start); }

        const XDeviceInfo * begin() const { return m_start; }
        const XDeviceInfo * end()   const { return m_start + m_deviceCount; }

    private:
        int           m_deviceCount;
        XDeviceInfo * m_start;

    };

    /**
     * Applies a function to all the input devices
     */
    template <typename F>
    void processInputDevices(F what) const
    {
        const XDeviceInfoList devices(connection.display());

        std::for_each(devices.begin(), devices.end(), what);
    }

    /**
     * Applies a function to the input device with the specified
     * id. Returns true if such a device has been found
     */
    template <typename F>
    bool processInputDevice(int id, F what) const
    {
        const XDeviceInfoList devices(connection.display());

        const auto device = std::find_if(devices.begin(), devices.end(),
            [id] (const XDeviceInfo & device) { return device.id == id; });

        const auto found = device != devices.end();

        if (found) what(*device);

        return found;
    }

    void processEvent(const XEvent & event)
    {
        auto presenceEvent = (XDevicePresenceNotifyEvent*)(&event);

        switch (presenceEvent->devchange) {
        case DeviceEnabled:
            processInputDevice(presenceEvent->deviceid,
                [] (const XDeviceInfo & device) {
                    qDebug() << "we got a new device: " << device.id << " " << device.name;
                }
            );
            break;

        case DeviceDisabled:
            processInputDevice(presenceEvent->deviceid,
                [] (const XDeviceInfo & device) {
                    qDebug() << "we lost a device: " << device.id << " " << device.name;
                }
            );
            break;

        default:
            if (!processInputDevice(presenceEvent->deviceid,
                    [] (const XDeviceInfo & device) {
                        qDebug() << "something happened, no idea what: " << device.id << " " << device.name;
                    }
            )) {
                qDebug() << "we have lost a device: " << presenceEvent->deviceid;
            }
        }

    }

    Private()
    {
        int xiEventType;
        XEventClass xiEventClass;

        // Getting the type and class from the server
        DevicePresence(connection.display(), xiEventType, xiEventClass);

        connection.handleExtensionEvent(xiEventType, xiEventClass,
                [this] (const XEvent & event) { processEvent(event); }
            );

        processInputDevices([] (const XDeviceInfo & device) {
            qDebug() << "This is an input device: " << device.id << " " << device.name;
        });
    }

private:
    Connection connection;
};

InputDeviceList::InputDeviceList()
    : d(Private::instance())
{
}

InputDeviceList::~InputDeviceList()
{
}



} // namespace xlib
} // namespace backends
