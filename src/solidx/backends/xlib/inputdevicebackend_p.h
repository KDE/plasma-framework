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

#include "inputdevicebackend.h"
#include "utils/sharedsingleton.h"
#include "connection.h"

#include <X11/extensions/XInput.h>
#include <X11/extensions/XInput2.h>
#include <X11/extensions/XIproto.h>
#include <X11/Xatom.h>

#include <functional>
#include <iterator>
#include <algorithm>
#include <set>

namespace solidx {
namespace backends {
namespace xlib {

class InputDeviceBackend::Private
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
        using std::bind;
        using std::placeholders::_1;

        auto presenceEvent = (XDevicePresenceNotifyEvent*)(&event);

        if (!processInputDevice(presenceEvent->deviceid, bind(&Private::addDevice, this, _1))) {
            removeDevice(presenceEvent->deviceid);
        }
    }

    /**
     * Constructs an XAtom that represents the property
     * with the given name
     */
    inline
    Atom propertyAtom(const char * name)
    {
        return XInternAtom(connection.display(), name, True);
    }

    Private()
        : synapticsCapabilities (propertyAtom("Synaptics Capabilities")),
          synapticsOff          (propertyAtom("Synaptics Off")),
          touchMaxContacts      (propertyAtom("Max Contacts")),
          touchAxesPerContact   (propertyAtom("Axes Per Contact")),
          deviceNode            (propertyAtom("Device Node"))
    {
        using std::bind;
        using std::placeholders::_1;

        int xiEventType;
        XEventClass xiEventClass;

        // Getting the type and class from the server
        DevicePresence(connection.display(), xiEventType, xiEventClass);

        connection.handleExtensionEvent(xiEventType, xiEventClass,
                bind(&Private::processEvent, this, _1));

        processInputDevices(bind(&Private::addDevice, this, _1));
    }

    /**
     * Tests whether the device has the specified property
     */
    inline
    bool hasProperty(XIDeviceInfo * info, Atom property)
    {
        Atom atomType;
        int  atomFormat;
        unsigned long countItems, bytesAfter;
        unsigned char * data;

        return property &&
            Success == XIGetProperty(
                connection.display(),
                info->deviceid,
                property,
                0, 1000, False,
                AnyPropertyType,
                &atomType, &atomFormat, &countItems, &bytesAfter, &data
            )
            && atomFormat && data;
    }

    /**
     * Checks which properties a specific device has
     * Note: std::set used instead of QSet because of std::inserter
     * TODO: benchmark against std::vector
     */
    inline
    std::set<Atom> presentProperties(const XDeviceInfo & device,
            const std::vector<Atom> & propertiesToCheck)
    {
        std::set<Atom> result;

        int count = -1;
        auto info = XIQueryDevice(connection.display(), device.id, &count);

        if (count) {
            std::copy_if(
                propertiesToCheck.begin(), propertiesToCheck.end(),
                std::inserter(result, result.end()),
                [=] (Atom property) {
                    return hasProperty(info, property);
                }
            );
        }

        XIFreeDeviceInfo(info);

        return result;
    }

    /**
     * Adding the device to the list and
     * sends the appropriate event
     */
    void addDevice(const XDeviceInfo & device)
    {
        if (!knownDevices.contains(device.id)) {

            static const std::vector<Atom> testProperties = {
                synapticsOff,
                synapticsCapabilities,
                touchMaxContacts,
                touchAxesPerContact,
                deviceNode
            };

            auto properties = presentProperties(device, testProperties);

            solidx::InputDevice newDevice {
                // The id of the device
                device.name,

                // The name of the device as reported by X server
                device.name,


                // Ignoring the core devices since we can not tell anything from them
                device.use == IsXExtensionKeyboard ? solidx::InputDevice::Type::Keyboard :
                device.use == IsXExtensionPointer  ? solidx::InputDevice::Type::Pointer :
                /* default */                        solidx::InputDevice::Type::Any,


                // Trying to find out whether we have a touch-screen
                // or a touchpad
                properties.count(synapticsOff)          ? solidx::InputDevice::Subtype::Touchpad :
                properties.count(synapticsCapabilities) ? solidx::InputDevice::Subtype::Touchpad :
                properties.count(touchMaxContacts)      ? solidx::InputDevice::Subtype::Touchscreen :
                properties.count(touchAxesPerContact)   ? solidx::InputDevice::Subtype::Touchscreen :

                // TODO: Do the keyboard detection via udev
                // Currently, we are using some weird heuristics to
                // bypass the X server's unawareness of keyboard details.
                //
                // Something is a full keyboard if its name says so.
                // But, for the standard "AT Translated Set 2 keyboard" device,
                // it is a real keyboard if it has a 'Device Node' property.
                (
                    newDevice.type == solidx::InputDevice::Type::Keyboard &&
                    newDevice.name.toLower().contains("keyboard") &&
                    (
                        properties.count(deviceNode) ||
                        newDevice.name != "AT Translated Set 2 keyboard"
                    )
                )                                       ? solidx::InputDevice::Subtype::FullKeyboard :

                /* default */                             solidx::InputDevice::Subtype::Any

            };

            // Ignoring the XTEST devices for good measure
            if (!newDevice.name.contains("XTEST")) {

                knownDevices[device.id] = newDevice;
                internalDeviceIds[newDevice.id] = device.id;

                qDebug() << "We got a new input device: "
                     << device.id
                     << "\t of type " << (int)newDevice.type
                     << "/" << (int)newDevice.subtype
                     << " name: " << newDevice.name
                     ;

                emit addedDevice(newDevice.id);
            }
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
            auto device = knownDevices[id];

            knownDevices.remove(id);
            internalDeviceIds.remove(device.id);

            emit removedDevice(device.id);
        }
    }

Q_SIGNALS:
    void addedDevice(const QString & id);
    void removedDevice(const QString & id);

private:
    Connection connection;
    QMap<XID, solidx::InputDevice> knownDevices;
    QMap<QString, XID> internalDeviceIds;

    const Atom synapticsCapabilities;
    const Atom synapticsOff;
    const Atom touchMaxContacts;
    const Atom touchAxesPerContact;
    const Atom deviceNode;

};


} // namespace xlib
} // namespace backends
} // namespace solidx

#endif // SOLIDX_XLIB_INPUTDEVICE_P_H

