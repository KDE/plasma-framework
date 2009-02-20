/*
 *   Copyright 2007 by Dan Meltzer <hydrogen@notyetimplemented.com>
 *   Copyright (C) 2008 by Alexis Ménard <darktears31@gmail.com>
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

#include "windowpreview_p.h"

#include <QPainter>

#include <kwindowsystem.h>
#include <kdebug.h>

#include <plasma/framesvg.h>

#ifdef Q_WS_X11
#include <QX11Info>

#include <X11/Xlib.h>
#include <fixx11h.h>
#endif

namespace Plasma {

bool WindowPreview::previewsAvailable() // static
{
    if (!KWindowSystem::compositingActive()) {
        return false;
    }
#ifdef Q_WS_X11
    // hackish way to find out if KWin has the effect enabled,
    // TODO provide proper support
    Display *dpy = QX11Info::display();
    Atom atom = XInternAtom(dpy, "_KDE_WINDOW_PREVIEW", False);
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

WindowPreview::WindowPreview(QWidget *parent)
    : QWidget(parent)
{
    m_background = new Plasma::FrameSvg(this);
    m_background->setImagePath("widgets/frame");
    m_background->setElementPrefix("raised");
}

void WindowPreview::setWindowId(WId w)
{
    if (!previewsAvailable()) {
        setMinimumSize(0,0);
        setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
        id = 0;
        return;
    }
    id = w;
    readWindowSize();
    QSize s(sizeHint());
    if (s.isValid()) {
        setFixedSize(sizeHint());
    }
}

WId WindowPreview::windowId() const
{
    return id;
}

QSize WindowPreview::sizeHint() const
{
    if (id == 0) {
        return QSize();
    }
    if (!windowSize.isValid()) {
        readWindowSize();
    }
    QSize s = windowSize;
    s.scale(200, 150, Qt::KeepAspectRatio);

    return s;
}

void WindowPreview::readWindowSize() const
{
#ifdef Q_WS_X11
    if (id > 0) {
        KWindowInfo info = KWindowSystem::windowInfo(id, NET::WMGeometry|NET::WMFrameExtents);
        windowSize = info.frameGeometry().size();
    } else {
        windowSize = QSize();
    }
#else
    windowSize = QSize();
#endif
}

void WindowPreview::setInfo()
{
#ifdef Q_WS_X11
    Display *dpy = QX11Info::display();
    Atom atom = XInternAtom(dpy, "_KDE_WINDOW_PREVIEW", False);
    if (id == 0) {
        XDeleteProperty(dpy, parentWidget()->winId(), atom);
        return;
    }
    if (!windowSize.isValid()) {
        readWindowSize();
    }
    if (!windowSize.isValid()) {
        XDeleteProperty(dpy, parentWidget()->winId(), atom);
        return;
    }
    Q_ASSERT(parentWidget()->isWindow()); // parent must be toplevel

    QSize thumbnailSize = sizeHint();
    thumbnailSize.scale(size(), Qt::KeepAspectRatio);
    m_background->resizeFrame(thumbnailSize);

    qreal left, top, right, bottom;
    m_background->getMargins(left, top, right, bottom);
    QRect thumbnailRect = geometry().adjusted(left, top, -right, -bottom);


    long data[] = { 1, 5, id, thumbnailRect.x(), thumbnailRect.y(), thumbnailRect.width(), thumbnailRect.height() };
    XChangeProperty(dpy, parentWidget()->winId(), atom, atom, 32, PropModeReplace,
        reinterpret_cast<unsigned char *>(data), sizeof(data) / sizeof(data[ 0 ]));
#endif
}

void WindowPreview::paintEvent(QPaintEvent *e)
{
    Q_UNUSED(e)
#ifdef Q_WS_X11
    QPainter painter(this);
    QRect r(QPoint(0,0), m_background->frameSize().toSize());
    r.moveCenter(QPoint(size().width()/2, size().height()/2));
    m_background->paintFrame(&painter, r.topLeft());
#endif
}

} // namespace Plasma

#include "windowpreview_p.moc"
