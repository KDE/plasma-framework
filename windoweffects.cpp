/*
 *   Copyright 2009 Marco Martin <notmart@gmail.com>
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

#include "windoweffects.h"

#include <QVarLengthArray>


namespace Plasma
{

namespace WindowEffects
{

void setSlidingWindow(WId id, Plasma::Location location)
{
#ifdef Q_WS_X11
    Display *dpy = QX11Info::display();
    //set again the atom, the location could have changed
    QDesktopWidget *desktop = QApplication::desktop();

    Window dummy;
    int x;
    int y;
    uint width;
    uint height;
    uint bw;
    uint d;
    XGetGeometry(dpy, id, &dummy, &x, &y, &width, &height, &bw, &d);

    QRect avail = desktop->availableGeometry(QPoint(x, y));//desktop->screenNumber(pos()));

    Atom atom = XInternAtom( dpy, "_KDE_SLIDE", False );
    QVarLengthArray<long, 1024> data(2);

    switch (location) {
    case LeftEdge:
        data[0] = avail.left();
        data[1] = 0;
        break;
    case TopEdge:
        data[0] = avail.top();
        data[1] = 1;
        break;
    case RightEdge:
        data[0] = avail.right();
        data[1] = 2;
        break;
    case BottomEdge:
        data[0] = avail.bottom();
        data[1] = 3;
    default:
        break;
    }

    if (location == Desktop || location == Floating) {
        XDeleteProperty(dpy, id, atom);
    } else {
        XChangeProperty(dpy, id, atom, atom, 32, PropModeReplace,
                        reinterpret_cast<unsigned char *>(data.data()), data.size());
    }
#endif
}

}

}
