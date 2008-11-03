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

#ifndef PLASMA_CALENDAR_H
#define PLASMA_CALENDAR_H

#include <QtGui/QGraphicsWidget>

#include "plasmaclock_export.h"

#include <kcalendarsystem.h>

#include "calendartable.h"

namespace Plasma
{

class CalendarTable;
class CalendarPrivate;

class PLASMACLOCK_EXPORT Calendar : public QGraphicsWidget
{
    Q_OBJECT

public:
    explicit Calendar(QGraphicsWidget *parent = 0);
    Calendar(const QDate &, QGraphicsWidget *parent = 0);
    ~Calendar();

    const KCalendarSystem *calendar () const;

    bool setDate(const QDate &date);
    const QDate& date() const;

    bool setCalendar(KCalendarSystem *calendar = 0);

    CalendarTable *calendarTable() const;

Q_SIGNALS:
    void dateChanged(const QDate &cur, const QDate &old);
    void dateChanged(const QDate &date);
    void tableClicked();

private Q_SLOTS:
    void displayedMonthChanged(int calendarSystemYear, int calendarSystemMonth);
    void prevMonth();
    void nextMonth();

private:
    CalendarPrivate* const d;
};

}

#endif
