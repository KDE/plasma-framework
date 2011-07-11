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
#include <QVarLengthArray>
#include <QMouseEvent>

#include <kwindowsystem.h>
#include <kdebug.h>

#include <plasma/framesvg.h>
#include <plasma/windoweffects.h>

#ifdef Q_WS_X11
#include <QX11Info>

#include <X11/Xlib.h>
#include <fixx11h.h>
#endif

namespace Plasma {

WindowPreview::WindowPreview(QWidget *parent)
    : QWidget(parent),
      m_highlightWindows(false)
{
    m_background = new Plasma::FrameSvg(this);
    m_background->setImagePath("widgets/frame");
    m_background->setElementPrefix("raised");
    setMouseTracking(true);
}

void WindowPreview::setWindowIds(const QList<WId> wids)
{
    if (!WindowEffects::isEffectAvailable(WindowEffects::WindowPreview)) {
        setMinimumSize(0,0);
        setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
        ids.clear();
        return;
    }

    //FIXME: need to get rid of this 4 window maximum by using a smarter layout
    if (wids.count() < 5) {
        ids = wids;
    } else {
        ids = wids.mid(0, 4);
    }

    windowSizes = WindowEffects::windowSizes(ids);
    QSize s(sizeHint());
    if (s.isValid()) {
        setFixedSize(sizeHint());
    }
}

QList<WId> WindowPreview::windowIds() const
{
    return ids;
}

QSize WindowPreview::sizeHint() const
{
    if (ids.size() == 0) {
        return QSize();
    }

    if (!windowSizes.size() == 0) {
        windowSizes = WindowEffects::windowSizes(ids);
    }

    int maxHeight = 0;
    int totalWidth = 0;

    foreach (const QSize &s, windowSizes) {
        if (s.height() > maxHeight) {
            maxHeight = s.height();
        }

        totalWidth += s.width();
    }

    QSize s(totalWidth, maxHeight);

    qreal left, top, right, bottom;
    m_background->getMargins(left, top, right, bottom);

    s.scale(WINDOW_WIDTH*windowSizes.size(), WINDOW_HEIGHT, Qt::KeepAspectRatio);
    s = s + QSize(left+right+WINDOW_MARGIN*(windowSizes.size()-1), top+bottom);
    return s;
}


bool WindowPreview::isEmpty() const
{
  foreach (WId id, ids) {
      if (id != 0) {
          return false;
      }
  }

  return true;
}


void WindowPreview::setHighlightWindows(const bool highlight)
{
    m_highlightWindows = highlight;
}

bool WindowPreview::highlightWindows() const
{
    return m_highlightWindows;
}

void WindowPreview::setInfo()
{
    QWidget *w = parentWidget();
    if (isEmpty()) {
        WindowEffects::showWindowThumbnails(w->winId());
        return;
    }

    if (windowSizes.size() == 0) {
        windowSizes = WindowEffects::windowSizes(ids);
    }

    if (windowSizes.size() == 0) {
        WindowEffects::showWindowThumbnails(w->winId());
        return;
    }

    Q_ASSERT(w->isWindow()); // parent must be toplevel

    QSize thumbnailSize = sizeHint();
    thumbnailSize.scale(size(), Qt::KeepAspectRatio);
    m_background->resizeFrame(thumbnailSize);

    qreal left, top, right, bottom;
    m_background->getMargins(left, top, right, bottom);
    const QRect thumbnailRect(QPoint(left, top), size() - QSize(left + right, top + bottom));
    const int numWindows = ids.size();
    const qreal thumbWidth = (thumbnailRect.width() - WINDOW_MARGIN*(numWindows - 1)) / numWindows;

    // we paint in parent coords, but accept events in local coords
    QList<QRect> inParentCoords;
    m_thumbnailRects.clear();

    int x = thumbnailRect.x();
    foreach (QSize s, windowSizes) {
        s.scale(thumbWidth, thumbnailRect.height(), Qt::KeepAspectRatio);
        int y = thumbnailRect.y() + (thumbnailRect.height() - s.height())/2;
        m_thumbnailRects.append(QRect(QPoint(x, y), s));
        inParentCoords.append(QRect(mapToParent(QPoint(x, y)), s));
        x += s.width() + WINDOW_MARGIN;
    }

    WindowEffects::showWindowThumbnails(w->winId(), ids, inParentCoords);
}

void WindowPreview::paintEvent(QPaintEvent *e)
{
    Q_UNUSED(e)

    QPainter painter(this);

    qreal left, top, right, bottom;
    m_background->getMargins(left, top, right, bottom);
    const QSize delta(left + right, top + bottom);
    const QPoint topLeft(left, top);

    foreach (const QRect &r, m_thumbnailRects) {
        //kWarning()<<r;
        m_background->resizeFrame(r.size() + delta);
        m_background->paintFrame(&painter, r.topLeft() - topLeft);
    }
}

void WindowPreview::mousePressEvent(QMouseEvent *event)
{
    for (int i = 0; i < m_thumbnailRects.size(); ++i) {
        if (m_thumbnailRects[i].contains(event->pos())) {
            emit windowPreviewClicked(ids[i], event->buttons(), event->modifiers(), event->globalPos());
            return;
        }
    }
}

void WindowPreview::mouseMoveEvent(QMouseEvent *event)
{
    if (!m_highlightWindows) {
        return;
    }

    for (int i = 0; i < m_thumbnailRects.size(); ++i) {
        if (m_thumbnailRects[i].contains(event->pos())) {
            WindowEffects::highlightWindows(effectiveWinId(), QList<WId>() << effectiveWinId() << ids[i]);
            return;
        }
    }

    WindowEffects::highlightWindows(effectiveWinId(), QList<WId>());
}

void WindowPreview::leaveEvent(QEvent *event)
{
    Q_UNUSED(event)
    if (!m_highlightWindows) {
        return;
    }
    WindowEffects::highlightWindows(effectiveWinId(), QList<WId>());
}

} // namespace Plasma

#include "windowpreview_p.moc"
