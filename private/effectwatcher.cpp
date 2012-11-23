/*
 *   Copyright 2011 Marco Martin <mart@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "effectwatcher_p.h"

#include <QCoreApplication>

#include <X11/Xlib.h>
#include <xcb/xcb.h>
#include <qx11info_x11.h>

namespace Plasma
{

EffectWatcher::EffectWatcher(const QString& property, QObject *parent)
    : QObject(parent),
      m_property(property)
{
    m_effectActive = isEffectActive();

    QCoreApplication::instance()->installNativeEventFilter(this);

    Display *dpy = QX11Info::display();
    Window root = DefaultRootWindow(dpy);
    XWindowAttributes attrs;
    //Don't reset eventual other masks already there
    XGetWindowAttributes(dpy, root, &attrs);
    attrs.your_event_mask |= PropertyChangeMask;
    XSelectInput(dpy, root, attrs.your_event_mask);
}

bool EffectWatcher::nativeEventFilter(const QByteArray& eventType, void *message, long *result)
{
    Q_UNUSED(result);
    if (eventType != "xcb_generic_event_t")
        return false;
    xcb_generic_event_t* event = reinterpret_cast<xcb_generic_event_t *>(message);
    uint response_type = event->response_type & ~0x80;
    if (response_type != XCB_PROPERTY_NOTIFY)
        return false;

    xcb_property_notify_event_t* prop_event = reinterpret_cast<xcb_property_notify_event_t *>(event);
    Display *dpy = QX11Info::display();
    Atom testAtom = XInternAtom(dpy, m_property.toLatin1(), False);
    if (prop_event->atom == testAtom) {
        bool nowEffectActive = isEffectActive();
        if (m_effectActive != nowEffectActive) {
            m_effectActive = nowEffectActive;
            emit effectChanged(m_effectActive);
        }
    }
    return false;
}

bool EffectWatcher::isEffectActive() const
{
    Display *dpy = QX11Info::display();
    Atom testAtom = XInternAtom(dpy, m_property.toLatin1(), False);

    bool nowEffectActive = false;
    int cnt;
    Atom *list = XListProperties(dpy, DefaultRootWindow(dpy), &cnt);
    if (list != NULL) {
        nowEffectActive = (qFind(list, list + cnt, testAtom) != list + cnt);
        XFree(list);
    }
    return nowEffectActive;
}

} // namespace Plasma
