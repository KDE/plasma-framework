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

#include <kwindowsystem.h>

#ifdef Q_WS_X11
#include <X11/Xlib.h>
#include <QX11Info>
#endif

namespace Plasma
{

namespace WindowEffects
{

//FIXME: check if this works for any atom?
bool isEffectAvailable(Effect effect)
{
    if (!KWindowSystem::compositingActive()) {
        return false;
    }
#ifdef Q_WS_X11
    QString effectName;

    switch (effect) {
    case Slide:
        effectName = "_KDE_SLIDE";
        break;
    case WindowPreview:
        effectName = "_KDE_WINDOW_PREVIEW";
        break;
    default:
        return false;
    }

    // hackish way to find out if KWin has the effect enabled,
    // TODO provide proper support
    Display *dpy = QX11Info::display();
    Atom atom = XInternAtom(dpy, effectName.toLatin1(), False);
    int cnt;
    Atom *list = XListProperties(dpy, DefaultRootWindow(dpy), &cnt);
    if (list != NULL) {
        bool ret = (qFind(list, list + cnt, atom) != list + cnt);
        XFree(list);
        return ret;
    }
#endif
    return false;
}

void slideWindow(WId id, Plasma::Location location, int offset)
{
#ifdef Q_WS_X11
    Display *dpy = QX11Info::display();
    Atom atom = XInternAtom( dpy, "_KDE_SLIDE", False );
    QVarLengthArray<long, 1024> data(2);

    data[0] = offset;

    switch (location) {
    case LeftEdge:
        data[1] = 0;
        break;
    case TopEdge:
        data[1] = 1;
        break;
    case RightEdge:
        data[1] = 2;
        break;
    case BottomEdge:
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

void slideWindow(QWidget *widget, Plasma::Location location)
{
#ifdef Q_WS_X11
    Display *dpy = QX11Info::display();
    Atom atom = XInternAtom( dpy, "_KDE_SLIDE", False );
    QVarLengthArray<long, 1024> data(2);

    switch (location) {
    case LeftEdge:
        data[0] = widget->geometry().left();
        data[1] = 0;
        break;
    case TopEdge:
        data[0] = widget->geometry().top();
        data[1] = 1;
        break;
    case RightEdge:
        data[0] = widget->geometry().right();
        data[1] = 2;
        break;
    case BottomEdge:
        data[0] = widget->geometry().bottom();
        data[1] = 3;
    default:
        break;
    }

    if (location == Desktop || location == Floating) {
        XDeleteProperty(dpy, widget->effectiveWinId(), atom);
    } else {
        XChangeProperty(dpy, widget->effectiveWinId(), atom, atom, 32, PropModeReplace,
                        reinterpret_cast<unsigned char *>(data.data()), data.size());
    }
#endif
}

QList<QSize> windowSizes(const QList<WId> &ids)
{
    QList<QSize> windowSizes;
    foreach (WId id, ids) {
#ifdef Q_WS_X11
        if (id > 0) {
            KWindowInfo info = KWindowSystem::windowInfo(id, NET::WMGeometry|NET::WMFrameExtents);
            windowSizes.append(info.frameGeometry().size());
        } else {
            windowSizes.append(QSize());
        }
#else
        windowSizes.append(QSize());
#endif
    }
    return windowSizes;
}

void showWindowThumbnails(WId parent, const QList<WId> &windows, const QList<QRect> &rects)
{
    if (windows.size() != rects.size()) {
        return;
    }
#ifdef Q_WS_X11
    Display *dpy = QX11Info::display();
    Atom atom = XInternAtom(dpy, "_KDE_WINDOW_PREVIEW", False);
    if (windows.isEmpty()) {
        XDeleteProperty(dpy, parent, atom);
        return;
    }

    int numWindows = windows.size();

    QVarLengthArray<long, 1024> data(1 + (6 * numWindows));
    data[0] = numWindows;

    QList<WId>::const_iterator windowsIt;
    QList<QRect>::const_iterator rectsIt = rects.constBegin();
    int i = 0;
    for (windowsIt = windows.constBegin(); windowsIt != windows.constEnd(); ++windowsIt) {

        const int start = (i * 6) + 1;
        const QRect thumbnailRect = (*rectsIt);

        data[start] = 5;
        data[start+1] = (*windowsIt);
        data[start+2] = thumbnailRect.x();
        data[start+3] = thumbnailRect.y();
        data[start+4] = thumbnailRect.width();
        data[start+5] = thumbnailRect.height();
        ++rectsIt;
        ++i;
    }

    XChangeProperty(dpy, parent, atom, atom, 32, PropModeReplace,
                    reinterpret_cast<unsigned char *>(data.data()), data.size());
#endif
}

}

}
