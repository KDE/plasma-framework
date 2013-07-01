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

#include "xlibinputdevicebackend.h"
#include "xlibconnection.h"
#include "xlibinputdevicebackend_p.h"

namespace solidx {
namespace backends {
namespace xlib {

// XlibInputDeviceBackend::Private
/**
 * RAII class to handle the list of X Input devices, and
 * to provide an iterator-enabled interface
 */
class XlibInputDeviceBackend::Private::XDeviceInfoList {
public:
    XDeviceInfoList(_XDisplay * display)
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
void XlibInputDeviceBackend::Private::processInputDevices(F what) const
{
    const XDeviceInfoList devices(connection.display());

    std::for_each(devices.begin(), devices.end(), what);
}

/**
 * Applies a function to the input device with the specified
 * id. Returns true if such a device has been found
 */
template <typename F>
bool XlibInputDeviceBackend::Private::processInputDevice(int id, F what) const
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
void XlibInputDeviceBackend::Private::processEvent(const XEvent & event)
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
Atom XlibInputDeviceBackend::Private::propertyAtom(const char * name)
{
    return XInternAtom(connection.display(), name, True);
}

XlibInputDeviceBackend::Private::Private()
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
bool XlibInputDeviceBackend::Private::hasProperty(XIDeviceInfo * info, Atom property)
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
std::set<Atom> XlibInputDeviceBackend::Private::presentProperties(const XDeviceInfo & device,
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
void XlibInputDeviceBackend::Private::addDevice(const XDeviceInfo & device)
{
    if (!knownDevices.count(device.id)) {

        static const std::vector<Atom> testProperties = {
            synapticsOff,
            synapticsCapabilities,
            touchMaxContacts,
            touchAxesPerContact,
            deviceNode
        };

        auto properties = presentProperties(device, testProperties);

        auto deviceType =
            // Ignoring the core devices since we can not tell anything from them
            device.use == IsXExtensionKeyboard ? InputDevice::Type::Keyboard :
            device.use == IsXExtensionPointer  ? InputDevice::Type::Pointer :
            /* default */                        InputDevice::Type::Any;

        auto deviceSubtype =
            // Trying to find out whether we have a touch-screen
            // or a touchpad
            properties.count(synapticsOff)          ? InputDevice::Subtype::Touchpad :
            properties.count(synapticsCapabilities) ? InputDevice::Subtype::Touchpad :
            properties.count(touchMaxContacts)      ? InputDevice::Subtype::Touchscreen :
            properties.count(touchAxesPerContact)   ? InputDevice::Subtype::Touchscreen :

            // TODO: Do the keyboard detection via udev
            // Currently, we are using some weird heuristics to
            // bypass the X server's unawareness of keyboard details.
            //
            // Something is a full keyboard if its name says so.
            // But, for the standard "AT Translated Set 2 keyboard" device,
            // it is a real keyboard if it has a 'Device Node' property.
            (
                deviceType == InputDevice::Type::Keyboard &&
                QString::fromLatin1(device.name).toLower().contains(QLatin1String("keyboard")) &&
                (
                    properties.count(deviceNode) ||
                    QLatin1String(device.name) != QLatin1String("AT Translated Set 2 keyboard")
                )
            )                                       ? InputDevice::Subtype::FullKeyboard :

            /* default */                             InputDevice::Subtype::Any
        ;

        std::unique_ptr<InputDevice> newDevice(new InputDevice {
            QString::number(device.id) + QString::fromLatin1(device.name),
            QString::fromLatin1(device.name), deviceType, deviceSubtype
        });

        // Ignoring the XTEST devices for good measure
        if (!newDevice->name.contains(QLatin1String("XTEST"))) {

            internalDeviceIds[newDevice->id] = device.id;

            qDebug() << "We got a new input device: "
                 << device.id
                 << "\t of type " << (int)newDevice->type
                 << "/" << (int)newDevice->subtype
                 << " name: " << newDevice->name
                 ;

            emit addedDevice(newDevice->id);

            knownDevices[device.id] = std::move(newDevice);
        }
    }
}

/**
 * Removing the device from the list and
 * sends the appropriate event
 */
void XlibInputDeviceBackend::Private::removeDevice(XID id)
{
    if (knownDevices.count(id)) {
        qDebug() << "Lost an input device: " << id << " " << knownDevices[id]->name;
        auto device = std::move(knownDevices[id]);

        emit removedDevice(device->id);

        internalDeviceIds.erase(device->id);
        knownDevices.erase(id);
    }
}



// XlibInputDeviceBackend

XlibInputDeviceBackend::XlibInputDeviceBackend(QObject * parent)
    : AbstractInputDeviceBackend(parent), d(Private::instance())
{
    connect(
        d.get(), SIGNAL(addedDevice(QString)),
        this,    SIGNAL(addedDevice(QString))
    );
    connect(
        d.get(), SIGNAL(removedDevice(QString)),
        this,    SIGNAL(removedDevice(QString))
    );
}

XlibInputDeviceBackend::~XlibInputDeviceBackend()
{
}

QStringList XlibInputDeviceBackend::devices() const
{
    QStringList result;

    foreach (auto item, d->internalDeviceIds) {
        result << item.first;
    }

    return result;
}

const InputDevice & XlibInputDeviceBackend::device(const QString & id) const
{
    if (!d->internalDeviceIds.count(id)) return InputDevice::null;

    auto internalId = d->internalDeviceIds[id];

    if (!d->knownDevices.count(internalId)) return InputDevice::null;

    return *(d->knownDevices[internalId].get());
}

} // namespace xlib
} // namespace backends
} // namespace solidx

