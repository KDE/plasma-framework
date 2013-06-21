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
#include <X11/extensions/XInput2.h>
#include <X11/extensions/XIproto.h>
#include <X11/Xatom.h>

#include <functional>
#include <iterator>
#include <algorithm>
#include <set>

// struct _XDisplay;
// struct _XDeviceInfo;
// union  _XEvent;
//
static void
print_property_xi2(Display *dpy, int deviceid, Atom property)
{
    Atom                act_type;
    char                *name;
    int                 act_format;
    unsigned long       nitems, bytes_after;
    unsigned char       *data, *ptr;
    int                 j, done = False;

    name = XGetAtomName(dpy, property);
    printf("\t[%s] (%ld):\t", name, property);

    if (XIGetProperty(dpy, deviceid, property, 0, 1000, False,
                           AnyPropertyType, &act_type, &act_format,
                           &nitems, &bytes_after, &data) == Success)
    {
        Atom float_atom = XInternAtom(dpy, "FLOAT", True);

        ptr = data;

        if (nitems == 0)
            printf("<no items>");

        for (j = 0; j < nitems; j++)
        {
            switch(act_type)
            {
                case XA_INTEGER:
                    switch(act_format)
                    {
                        case 8:
                            printf("%d", *((int8_t*)ptr));
                            break;
                        case 16:
                            printf("%d", *((int16_t*)ptr));
                            break;
                        case 32:
                            printf("%d", *((int32_t*)ptr));
                            break;
                    }
                    break;
                case XA_CARDINAL:
                    switch(act_format)
                    {
                        case 8:
                            printf("%u", *((uint8_t*)ptr));
                            break;
                        case 16:
                            printf("%u", *((uint16_t*)ptr));
                            break;
                        case 32:
                            printf("%u", *((uint32_t*)ptr));
                            break;
                    }
                    break;
                case XA_STRING:
                    if (act_format != 8)
                    {
                        printf("Unknown string format.\n");
                        done = True;
                        break;
                    }
                    printf("\"%s\"", ptr);
                    j += strlen((char*)ptr); /* The loop's j++ jumps over the
                                                terminating 0 */
                    ptr += strlen((char*)ptr); /* ptr += size below jumps over
                                                  the terminating 0 */
                    break;
                case XA_ATOM:
                    {
                        Atom a = *(uint32_t*)ptr;
                        printf("\"%s\" (%ld)",
                                (a) ? XGetAtomName(dpy, a) : "None",
                                a);
                        break;
                    }
                    break;
                default:
                    if (float_atom != None && act_type == float_atom)
                    {
                        printf("%f", *((float*)ptr));
                        break;
                    }

                    printf("\t... of unknown type %s\n",
                            XGetAtomName(dpy, act_type));
                    done = True;
                    break;
            }

            ptr += act_format/8;

            if (done == True)
                break;
            if (j < nitems - 1)
                printf(", ");
        }
        printf("\n");
        XFree(data);
    } else
        printf("\tFetch failure\n");

}


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
          test                  (propertyAtom("THISDOESNOTEXISTRIGHT"))
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
                touchAxesPerContact
            };

            auto properties = presentProperties(device, testProperties);

            Device newDevice {
                // The name of the device as reported by X server
                device.name,

                // Ignoring the core devices since we can not tell anything from them
                device.use == IsXExtensionKeyboard ? Device::Type::Keyboard :
                device.use == IsXExtensionPointer  ? Device::Type::Pointer :
                /* default */                        Device::Type::Unknown,

                // Trying to find out whether we have a touch-screen
                // or a touchpad
                properties.count(synapticsOff)          ? Device::Subtype::Touchpad :
                properties.count(synapticsCapabilities) ? Device::Subtype::Touchpad :
                properties.count(touchMaxContacts)      ? Device::Subtype::Touchscreen :
                properties.count(touchAxesPerContact)   ? Device::Subtype::Touchscreen :

                // Testing keyboard
                name.startsWith("AT")                   ? Device::Subtype::Keyboard

                /* default */                             Device::Subtype::Unknown

            };

            // Ignoring the XTEST devices for good measure
            if (!newDevice.name.contains("XTEST")) {
                knownDevices[device.id] = newDevice;

                qDebug() << "We got a new input device: "
                     << device.id
                     << "\t of type " << (int)newDevice.type
                     << "/" << (int)newDevice.subtype
                     << " name: " << newDevice.name
                     ;

                emit addedDevice(device.id);
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
            knownDevices.remove(id);

            emit removedDevice(id);
        }
    }

Q_SIGNALS:
    void addedDevice(int id);
    void removedDevice(int id);

private:
    Connection connection;
    QMap<XID, Device> knownDevices;

    const Atom synapticsCapabilities;
    const Atom synapticsOff;
    const Atom touchMaxContacts;
    const Atom touchAxesPerContact;
    const Atom test;

};


} // namespace xlib
} // namespace backends

#endif // SOLIDX_XLIB_INPUTDEVICE_P_H

