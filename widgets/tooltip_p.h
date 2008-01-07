/*
 *   Copyright 2007 by Dan Meltzer <hydrogen@notyetimplemented.com>
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
#ifndef PLASMATOOLTIP_H
#define PLASMATOOLTIP_H

#include <plasma/widgets/widget.h> //ToolTipData struct

#include <QWidget>  // base class
#include <QPixmap> // stack allocated
#include <QPoint>  // stack allocated
#include <QString> // stack allocated

namespace Plasma {

    class ToolTipData;

/**
    @author Dan Meltzer
*   A Singleton tooltip.  Before calling show it is necessary to
*   call setLocation and setData
*/
class ToolTip : public QWidget
{
    Q_OBJECT
public:
    ToolTip();
    ~ToolTip();

    static ToolTip *self();
    void show(const QPoint &location, Plasma::Widget *widget);
    void hide();

    Plasma::Widget *currentWidget() const;

protected:
    virtual void showEvent(QShowEvent *);

    void paintEvent(QPaintEvent */*event*/);

private Q_SLOTS:
    void slotResetTimer();
    void slotShowToolTip();

private:
    void setData(const Plasma::ToolTipData &data);

    class Private;
    Private *const d;
};

class WindowPreview : public QWidget
{
    Q_OBJECT
public:
    void setWindowId(WId w);
    void setInfo();
    virtual QSize sizeHint() const;
    bool previewsAvailable() const;
private:
    void readWindowSize() const;
    WId id;
    mutable QSize windowSize;
};

}

#endif

