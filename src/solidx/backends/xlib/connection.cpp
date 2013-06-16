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

#include "connection.h"

#include <QBasicTimer>
#include <QTimerEvent>
#include <QDebug>

#include <X11/extensions/XInput.h>

namespace backends {
namespace xlib {

namespace {
    template <typename F>
    void processInputDevices(Display * display, F what)
    {
        int deviceNumber = -1;
        auto devices = XListInputDevices(display, &deviceNumber);

        if (!devices) return;

        for (int i = 0; i < deviceNumber; i++) {
            what(devices[i]);
        }

        XFreeDeviceList(devices);
    }

    template <typename F>
    bool processInputDevice(Display * display, int id, F what)
    {
        bool found = false;

        processInputDevices(display, [&found,id,what] (const XDeviceInfo & device) {
            if (device.id == id) {
                what(device);
            }
        });

        return found;
    }
}

class Connection::Private: public QObject {
public:
    static std::shared_ptr<Private> instance();

    Private();
    ~Private();

    Display * const display;

protected:
    void timerEvent(QTimerEvent * e) override;
    void startTimer();

private:
    QBasicTimer m_timer;
    int m_xiEventType;

    static std::weak_ptr<Private> s_instance;

};

std::weak_ptr<Connection::Private> Connection::Private::s_instance;

std::shared_ptr<Connection::Private> Connection::Private::instance()
{
    auto ptr = s_instance.lock();

    if (!ptr) {
        ptr = std::make_shared<Private>();
        s_instance = ptr;
    }

    return ptr;
}

Connection::Private::Private()
    : display(XOpenDisplay(nullptr))
{
    Q_ASSERT_X(display != nullptr, "Xlib connection", "Display not accessible");

    int xiEventType;
    XEventClass xiEventClass;
    DevicePresence(display, xiEventType, xiEventClass);

    XSelectExtensionEvent(display, DefaultRootWindow(display), &xiEventClass, 1);

    m_xiEventType = xiEventType;

    startTimer();

    processInputDevices(display, [] (const XDeviceInfo & device) {
        qDebug() << "This is an input device: " << device.id << " " << device.name;
    });
}

Connection::Private::~Private() {
    m_timer.stop();
    XCloseDisplay(display);
}

void Connection::Private::timerEvent(QTimerEvent * e)
{
    XEvent event;

    while (XCheckTypedEvent(display, m_xiEventType, &event)) {
        auto presenceEvent = (XDevicePresenceNotifyEvent*)(&event);

        switch (presenceEvent->devchange) {
        case DeviceEnabled:
            processInputDevice(display, presenceEvent->deviceid,
                [] (const XDeviceInfo & device) {
                    qDebug() << "we got a new device: " << device.id << " " << device.name;
                }
            );
            break;

        case DeviceDisabled:
            processInputDevice(display, presenceEvent->deviceid,
                [] (const XDeviceInfo & device) {
                    qDebug() << "we lost a device: " << device.id << " " << device.name;
                }
            );
            break;

        default:
            if (!processInputDevice(display, presenceEvent->deviceid,
                [] (const XDeviceInfo & device) {
                    qDebug() << "something happened, no idea what: " << device.id << " " << device.name;
                }))
            {
                qDebug() << "we have lost a device: " << presenceEvent->deviceid;
            }
        }
    }
}

void Connection::Private::startTimer()
{
    qDebug() << "Timer started";
    m_timer.start(500, Qt::CoarseTimer, this);
}


Connection::Connection()
    : d(Private::instance())
{
}

Connection::~Connection()
{
}

Display * Connection::display() const
{
    return d->display;
}

} // namespace xlib
} // namespace backends


