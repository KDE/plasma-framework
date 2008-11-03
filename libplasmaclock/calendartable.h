/*
 *   Copyright 2008 Davide Bettio <davide.bettio@kdemail.net>
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

#ifndef PLASMA_CALENDARWIDGET_H
#define PLASMA_CALENDARWIDGET_H

#include <QtGui/QGraphicsWidget>

#include "plasmaclock_export.h"

#include <kcalendarsystem.h>

namespace Plasma
{

class CalendarTablePrivate;

class PLASMACLOCK_EXPORT CalendarTable : public QGraphicsWidget
{
    Q_OBJECT

public:
    enum CellType { NoType = 0,
                    Today = 1,
                    Selected = 2,
                    Hovered = 4,
                    NotInCurrentMonth = 8 };
    Q_DECLARE_FLAGS(CellTypes, CellType)

    explicit CalendarTable(QGraphicsWidget *parent = 0);
    CalendarTable(const QDate &, QGraphicsWidget *parent = 0);
    ~CalendarTable();

    const KCalendarSystem *calendar () const;

    bool setDate(const QDate &date);
    const QDate& date() const;

    bool setCalendar(KCalendarSystem *calendar = 0);

Q_SIGNALS:
    void dateChanged(const QDate &cur, const QDate &old);
    void dateChanged(const QDate &date);
    void tableClicked();
    void displayedMonthChanged(int calendarSystemYear, int calendarSystemMonth);
    void displayedYearChanged(int calendarSystemYear, int calendarSystemMonth);

protected:
    int cellX(int weekDay);
    int cellY(int week);

    void paint(QPainter *p, const QStyleOptionGraphicsItem *option, QWidget *widget = 0);
    void wheelEvent(QGraphicsSceneWheelEvent *event);
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    void hoverMoveEvent(QGraphicsSceneHoverEvent *event);
    void resizeEvent(QGraphicsSceneResizeEvent * event);

    virtual void paintCell(QPainter *p, int cell, int week, int weekDay, CellTypes type, const QDate &cellDate);
    virtual void paintBorder(QPainter *p, int cell, int week, int weekDay, CellTypes type, const QDate &cellDate);
    virtual QString cellSVGSuffix(int cell, int week, int weekDay, CellTypes type, const QDate &cellDate);

private:
    Q_PRIVATE_SLOT(d, void setupThemedElements())

    friend class CalendarTablePrivate;
    CalendarTablePrivate* const d;
};

}

Q_DECLARE_OPERATORS_FOR_FLAGS(Plasma::CalendarTable::CellTypes)
#endif
