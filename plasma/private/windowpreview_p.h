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

#ifndef PLASMA_WINDOWPREVIEW_P_H
#define PLASMA_WINDOWPREVIEW_P_H

#include <QWidget> // base class
#include <QSize> // stack allocated

namespace Plasma {

class FrameSvg;

/**
 * @internal
 *
 * A widget which reserves area for window preview and sets hints on the toplevel
 * tooltip widget that tells KWin to render the preview in this area. This depends
 * on KWin's TaskbarThumbnail compositing effect (which is automatically detected).
 */
class WindowPreview : public QWidget
{
    Q_OBJECT

public:
    static bool previewsAvailable();

    WindowPreview(QWidget *parent = 0);

    void setWindowIds(const QList<WId> w);
    QList<WId> windowIds() const;
    void setInfo();
    bool isEmpty() const;
    virtual QSize sizeHint() const;
    void setHighlightWindows(const bool highlight);
    bool highlightWindows() const;

Q_SIGNALS:
    void windowPreviewClicked(WId wid, Qt::MouseButtons buttons, Qt::KeyboardModifiers keys, const QPoint &screenPos);

protected:
    void paintEvent(QPaintEvent *e);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void leaveEvent(QEvent *event);

private:

    QList<WId> ids;
    mutable QList<QSize> windowSizes;
    QList <QRect> m_thumbnailRects;
    FrameSvg *m_background;
    bool m_highlightWindows;

    static const int WINDOW_MARGIN = 10;
    static const int WINDOW_WIDTH = 200;
    static const int WINDOW_HEIGHT = 150;
};

} // namespace Plasma

#endif // PLASMA_WINDOWPREVIEW_P_H

