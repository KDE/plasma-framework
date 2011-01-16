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


EffectWatcher::EffectWatcher(Plasma::WindowEffects::Effect effect, QWidget *parent)
    : QWidget(parent),
      m_effect(effect),
      m_effectActive(false)
{
#ifdef Q_WS_X11
    kapp->installX11EventFilter( this );
    Display *dpy = QX11Info::display();
    XSelectInput(dpy, RootWindow(dpy, 0), PropertyChangeMask);
#endif
}


#ifdef Q_WS_X11
bool EffectWatcher::x11Event(XEvent *event)
{
    bool nowEffectActive = WindowEffects::isEffectAvailable(m_effect);
    if (m_effectActive != nowEffectActive) {
        m_effectActive = nowEffectActive;
        emit blurBehindChanged(m_effectActive);
    }
    return QWidget::x11Event(event);
}
#endif


}

#include "effectwatcher_p.moc"
