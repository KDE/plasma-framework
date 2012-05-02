/*
    Copyright (C) 2012 Giorgos Tsiapaliwkas <terietor@gmail.com>
    Copyright (C) 2012 Antonis Tsiapaliokas <kok3rs@gmail.com>
    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

//own
#include "calendarsystem.h"

//KDE
#include <KCalendarSystem>
#include <KGlobal>

//Qt
#include <QtCore/QDateTime>

CalendarSystem::CalendarSystem(QObject* parent)
        : QObject(parent)
{
    m_calendarSystem = KCalendarSystem::create(KGlobal::locale()->calendarSystem());
}

QList<int> CalendarSystem::calendarSystemsList()
{
     QList<int> list;

     foreach(KLocale::CalendarSystem calendarSystem, KCalendarSystem::calendarSystemsList()) {
        list.append((int)calendarSystem);
    }

    return list;
}

Locale::CalendarSystem CalendarSystem::calendarSystem() const
{
    return (Locale::CalendarSystem)m_calendarSystem->calendarSystem();
}


QString CalendarSystem::calendarLabel() const
{
    return m_calendarSystem->calendarLabel();
}

QDate CalendarSystem::epoch() const
{
    return m_calendarSystem->epoch();
}

QDate CalendarSystem::earliestValidDate() const
{
    return epoch();
}

QDate CalendarSystem::latestValidDate() const
{
    return m_calendarSystem->latestValidDate();
}

bool CalendarSystem::isValidIsoWeekDate(int year, int isoWeekNumber, int dayOfIsoWeek) const
{
    return m_calendarSystem->isValidIsoWeekDate(year, isoWeekNumber, dayOfIsoWeek);
}

bool CalendarSystem::isValid(const QDate &date) const
{
    return m_calendarSystem->isValid(date);
}

QVariantHash CalendarSystem::getDate(const QDate date) const
{
    QVariantHash hash;

    int year;
    int month;
    int day;

    m_calendarSystem->getDate(date, &year, &month, &day);

    hash["year"] = year;
    hash["month"] = month;
    hash["day"] = day;

    return hash;
}

int CalendarSystem::year(const QDate &date) const
{
    return m_calendarSystem->year(date);
}

int CalendarSystem::month(const QDate &date) const
{
    return m_calendarSystem->month(date);
}

int CalendarSystem::day(const QDate &date) const
{
    return m_calendarSystem->day(date);
}

QString CalendarSystem::eraName(const QDate &date, StringFormat format) const
{
    return m_calendarSystem->eraName(date, (KCalendarSystem::StringFormat)format);
}

QString CalendarSystem::eraYear(const QDate &date, StringFormat format) const
{
    return m_calendarSystem->eraYear(date, (KCalendarSystem::StringFormat)format);
}


int CalendarSystem::yearInEra(const QDate &date) const
{
    return m_calendarSystem->yearInEra(date);
}

QDate CalendarSystem::addYears(const QDate &date, int numYears) const
{
    return m_calendarSystem->addYears(date, numYears);
}

QDate CalendarSystem::addMonths(const QDate &date, int numMonths) const
{
    return m_calendarSystem->addMonths(date, numMonths);
}

QDate CalendarSystem::addDays(const QDate &date, int numDays) const
{
    return m_calendarSystem->addDays(date, numDays);
}

QVariantHash CalendarSystem::dateDifference(const QDate &fromDate, const QDate &toDate) const
{
    QVariantHash hash;

    int yearsDiff;
    int monthsDiff;
    int daysDiff;
    int direction;

    m_calendarSystem->dateDifference(fromDate, toDate, &yearsDiff, &monthsDiff, &daysDiff, &direction);

    hash["years"] = yearsDiff;
    hash["months"] = monthsDiff;
    hash["days"] = daysDiff;
    hash["direction"] = direction;

    return hash;
}

int CalendarSystem::yearsDifference(const QDate &fromDate, const QDate &toDate) const
{
    return m_calendarSystem->yearsDifference(fromDate, toDate);
}

int CalendarSystem::monthsDifference(const QDate &fromDate, const QDate &toDate) const
{
    return m_calendarSystem->monthsDifference(fromDate, toDate);
}

int CalendarSystem::daysDifference(const QDate &fromDate, const QDate &toDate) const
{
    return m_calendarSystem->daysDifference(fromDate, toDate);
}

int CalendarSystem::monthsInYear(int year) const
{
    return m_calendarSystem->monthsInYear(year);
}

int CalendarSystem::weeksInYear(int year, WeekNumberSystem weekNumberSystem) const
{
     return m_calendarSystem->weeksInYear(year, (KLocale::WeekNumberSystem)weekNumberSystem);
}

int CalendarSystem::daysInYear(int year) const
{
    return m_calendarSystem->daysInYear(year);
}

int CalendarSystem::daysInMonth(int year, int month) const
{
   return m_calendarSystem->daysInMonth(year, month);
}

int CalendarSystem::daysInWeek(const QDate &date) const
{
    return m_calendarSystem->daysInWeek(date);
}

int CalendarSystem::dayOfYear(const QDate &date) const
{
    return m_calendarSystem->dayOfYear(date);
}

int CalendarSystem::dayOfWeek(const QDate &date) const
{
    return m_calendarSystem->dayOfWeek(date);
}

int CalendarSystem::week(const QDate &date, WeekNumberSystem weekNumberSystem) const
{
    return m_calendarSystem->week(date, (KLocale::WeekNumberSystem)weekNumberSystem);
}

bool CalendarSystem::isLeapYear(int year) const
{
    return m_calendarSystem->isLeapYear(year);
}

QDate CalendarSystem::firstDayOfYear(int year) const
{
    return m_calendarSystem->firstDayOfYear(year);
}

QDate CalendarSystem::lastDayOfYear(int year) const
{
    return m_calendarSystem->lastDayOfYear(year);
}

QDate CalendarSystem::firstDayOfMonth(int year, int month) const
{
   return m_calendarSystem->firstDayOfMonth(year, month);
}

QDate CalendarSystem::lastDayOfMonth(int year, int month) const
{
    return m_calendarSystem->lastDayOfMonth(year, month);
}

QString CalendarSystem::monthName(int month, int year, CalendarSystem::MonthNameFormat format) const
{
    return m_calendarSystem->monthName(month, year, (KCalendarSystem::MonthNameFormat)format);
}

QString CalendarSystem::weekDayName(int weekDay, CalendarSystem::WeekDayNameFormat format) const
{
    return m_calendarSystem->weekDayName(weekDay, (KCalendarSystem::WeekDayNameFormat)format);
}

QString CalendarSystem::formatDate(const QDate &date, DateTimeComponent component,
                                    DateTimeComponentFormat format,
                                    WeekNumberSystem weekNumberSystem) const
{
    return m_calendarSystem->formatDate(date, (KLocale::DateTimeComponent)component,
                                        (KLocale::DateTimeComponentFormat)format,
                                        (KLocale::WeekNumberSystem)weekNumberSystem);
}

QDate CalendarSystem::readDate(const QString &str, ReadDateFlags flags) const
{
    return m_calendarSystem->readDate(str, (KLocale::ReadDateFlags)flags);
}

int CalendarSystem::shortYearWindowStartYear() const
{
    return m_calendarSystem->shortYearWindowStartYear();
}

int CalendarSystem::applyShortYearWindow(int inputYear) const
{
    return m_calendarSystem->applyShortYearWindow(inputYear);
}

int CalendarSystem::weekStartDay() const
{
    return m_calendarSystem->weekStartDay();
}

bool CalendarSystem::isSolar() const
{
    return m_calendarSystem->isSolar();
}

bool CalendarSystem::isLunar() const
{
    return m_calendarSystem->isLunar();
}

bool CalendarSystem::isLunisolar() const
{
    return m_calendarSystem->isLunisolar();
}

bool CalendarSystem::isProleptic() const
{
    return m_calendarSystem->isProleptic();
}
