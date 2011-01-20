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

#include <kapplication.h>
#include <kdebug.h>

#ifdef Q_WS_X11
#include <X11/Xlib.h>
#endif

namespace Plasma
{


EffectWatcher::EffectWatcher(QString property, QWidget *parent)
    : QWidget(parent),
      m_property(property)
{
    m_effectActive = isEffectActive();
#ifdef Q_WS_X11
    kapp->installX11EventFilter( this );
    Display *dpy = QX11Info::display();
    Window root = DefaultRootWindow(dpy);
    XWindowAttributes attrs;
    //Don't reset eventual other masks already there
    XGetWindowAttributes(dpy, root, &attrs);
    attrs.your_event_mask |= PropertyChangeMask;
    XSelectInput(dpy, root, attrs.your_event_mask);
#endif
}


#ifdef Q_WS_X11
bool EffectWatcher::x11Event(XEvent *event)
{
    if (event->type == PropertyNotify) {
        Display *dpy = QX11Info::display();
        Atom testAtom = XInternAtom(dpy, m_property.toLatin1(), False);
        if (event->xproperty.atom == testAtom) {
            bool nowEffectActive = isEffectActive();
            if (m_effectActive != nowEffectActive) {
                m_effectActive = nowEffectActive;
                emit blurBehindChanged(m_effectActive);
            }
        }
    }
    return QWidget::x11Event(event);
}
#endif

bool EffectWatcher::isEffectActive() const
{
#ifdef Q_WS_X11
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
#else
    return false;
#endif
}

}

#include "effectwatcher_p.moc"
