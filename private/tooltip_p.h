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

#ifndef PLASMA_TOOLTIP_P_H
#define PLASMA_TOOLTIP_P_H

#include <QWidget> // base class

#include <plasma/tooltipmanager.h> //Content struct

namespace Plasma {

class ToolTipPrivate;

class ToolTip : public QWidget
{
    Q_OBJECT

public:
    ToolTip(QWidget *parent);
    ~ToolTip();

    void setContent(QObject *tipper, const ToolTipContent &data);
    void prepareShowing();
    void moveTo(const QPoint &to);
    bool autohide() const;
    void setDirection(Plasma::Direction);
    void linkActivated(const QString &anchor, QMouseEvent *event);

Q_SIGNALS:
    void activateWindowByWId(WId wid,
                             Qt::MouseButtons buttons, Qt::KeyboardModifiers modifiers,
                             const QPoint& screenPos);
    void linkActivated(const QString &anchor, 
                       Qt::MouseButtons buttons, Qt::KeyboardModifiers modifiers,
                       const QPoint& screenPos);
    void hovered(bool hovered);

protected:
    void checkSize();
    void adjustPosition(const QSize &previous, const QSize &current);
    void showEvent(QShowEvent *);
    void hideEvent(QHideEvent *);
    void mouseReleaseEvent(QMouseEvent *);
    void enterEvent(QEvent *);
    void leaveEvent(QEvent *);

    void resizeEvent(QResizeEvent *);
    void paintEvent(QPaintEvent *);

private Q_SLOTS:
    void updateTheme();

private:
    ToolTipPrivate * const d;
};

} // namespace Plasma

#endif // PLASMA_TOOLTIP_P_H

