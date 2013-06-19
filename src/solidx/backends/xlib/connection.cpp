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
#include "sharedsingleton_p.h"

#include <QBasicTimer>
#include <QTimerEvent>
#include <QDebug>

#include <algorithm>
#include <map>

#include <X11/extensions/XInput.h>

namespace backends {
namespace xlib {

class Connection::Private:
    public QObject,
    public SharedSingleton<Private>
{
public:
    Private();
    ~Private();

    Display * const display;

    // Using std::map because of: https://bugreports.qt-project.org/browse/QTBUG-25997
    // and we don't need COW or other QMap fancy things like that.
    std::map<int, std::function<void(const XEvent &)>> eventHandlers;

protected:
    void timerEvent(QTimerEvent * e) override;
    void startTimer();

private:
    QBasicTimer m_timer;

};


Connection::Private::Private()
    : display(XOpenDisplay(nullptr))
{
    Q_ASSERT_X(display != nullptr, "Xlib connection", "Display not accessible");

    startTimer();
}

Connection::Private::~Private() {
    m_timer.stop();
    XCloseDisplay(display);
}

void Connection::Private::timerEvent(QTimerEvent * e)
{
    XEvent event;

    for (auto handler: eventHandlers) {
        while (XCheckTypedEvent(display, handler.first, &event)) {
            handler.second(event);
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

void Connection::handleExtensionEvent(int eventType, XEventClass & eventClass, std::function<void(const XEvent &)> handler)
{
    d->eventHandlers[eventType] = handler;
    XSelectExtensionEvent(d->display, DefaultRootWindow(d->display), &eventClass, 1);
}

void Connection::releaseExtensionEventHandler(int eventType)
{

}

} // namespace xlib
} // namespace backends


