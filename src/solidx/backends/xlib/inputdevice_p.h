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

#ifndef SOLIDX_XLIB_INPUTDEVICE_P_H
#define SOLIDX_XLIB_INPUTDEVICE_P_H

#include <QObject>
#include <QDebug>

#include "inputdevice.h"
#include "utils/sharedsingleton.h"
#include "connection.h"

#include <X11/extensions/XInput.h>
#include <functional>

// struct _XDisplay;
// struct _XDeviceInfo;
// union  _XEvent;

namespace backends {
namespace xlib {

class InputDeviceListModel::Private
    : public QObject, public utils::SharedSingleton<Private>
{
    Q_OBJECT

public:
    /**
     * RAII class to handle the list of X Input devices, and
     * to provide an iterator-enabled interface
     */
    class XDeviceInfoList {
    public:
        XDeviceInfoList(_XDisplay * display)
            : m_deviceCount(-1),
              m_start(XListInputDevices(display, &m_deviceCount))
        {
        }

        ~XDeviceInfoList() { XFreeDeviceList(m_start); }

        const _XDeviceInfo * begin() const { return m_start; }
        const _XDeviceInfo * end()   const { return m_start + m_deviceCount; }

    private:
        int           m_deviceCount;
        _XDeviceInfo * m_start;

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

    /**
     * Process the event sent by the X server
     */
    void processEvent(const _XEvent & event)
    {
        auto presenceEvent = (XDevicePresenceNotifyEvent*)(&event);

        switch (presenceEvent->devchange) {
        case DeviceEnabled:
            processInputDevice(presenceEvent->deviceid,
                [this] (const XDeviceInfo & device) {
                    addDevice(device);
                }
            );
            break;

        default:
            if (!processInputDevice(presenceEvent->deviceid,
                    [&] (const XDeviceInfo & device) {
                        qDebug()
                            << "something happened, no idea what: "
                            << presenceEvent->devchange << " "
                            << device.id << " " << device.name;
                    }
            )) {
                removeDevice(presenceEvent->deviceid);
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
                [this] (const _XEvent & event) { processEvent(event); }
            );

        processInputDevices([this] (const XDeviceInfo & device) {
            addDevice(device);
        });
    }

    /**
     * Adding the device to the list and
     * sends the appropriate event
     */
    void addDevice(const XDeviceInfo & device)
    {
        if (!knownDevices.contains(device.id)) {

            knownDevices[device.id] = {
                device.name,
                device.use == IsXExtensionKeyboard ? Device::Keyboard :
                device.use == IsXKeyboard          ? Device::Keyboard :
                device.use == IsXExtensionPointer  ? Device::Pointer :
                device.use == IsXPointer           ? Device::Pointer :
                /* default */                        Device::Unknown
            };

            qDebug() << "This is an input device: " << device.id
                     << " " << device.name
                     << "of type " << knownDevices[device.id].type;

            emit addedDevice(device.id);
        }
    }

    /**
     * Removing the device from the list and
     * sends the appropriate event
     */
    void removeDevice(XID id)
    {
        if (knownDevices.contains(id)) {
            qDebug() << "Lost an input device: " << id << " " << knownDevices[id].name;
            knownDevices.remove(id);

            emit removedDevice(id);
        }
    }

Q_SIGNALS:
    void addedDevice(int id);
    void removedDevice(int id);

private:
    Connection connection;

    struct Device {
        QString name;
        enum Type {
            Unknown       = 0,
            Keyboard      = 1,
            Pointer       = 2,
            Toushscreen   = 3
        } type;
    };

    QMap<XID, Device> knownDevices;

};


} // namespace xlib
} // namespace backends

#endif // SOLIDX_XLIB_INPUTDEVICE_P_H

