/*
    Copyright (C) 2013 Mark Gaiser <markg85@gmail.com>
    Copyright (C) 2016 Martin Klapetek <mklapetek@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

*/

#include <QDebug>

#include "calendar.h"

Calendar::Calendar(QObject *parent)
    : QObject(parent)
    , m_types(Holiday | Event | Todo | Journal)
    , m_dayList()
    , m_weekList()
    , m_days(0)
    , m_weeks(0)
    , m_firstDayOfWeek(QLocale::system().firstDayOfWeek())
    , m_errorMessage()
{
    m_daysModel = new DaysModel(this);
    m_daysModel->setSourceData(&m_dayList);

    //  m_dayHelper = new CalendarDayHelper(this);
//   connect(m_dayHelper, SIGNAL(calendarChanged()), this, SLOT(updateData()));
}

QDateTime Calendar::displayedDate() const
{
    return QDateTime(m_displayedDate);
}

void Calendar::setDisplayedDate(const QDate &dateTime)
{
    if (m_displayedDate == dateTime) {
        return;
    }

    const int oldMonth = m_displayedDate.month();
    const int oldYear = m_displayedDate.year();

    m_displayedDate = dateTime;

    //  m_dayHelper->setDate(m_displayedDate.year(), m_displayedDate.month());

    updateData();
    emit displayedDateChanged();
    if (oldMonth != m_displayedDate.month()) {
        emit monthNameChanged();
    }
    if (oldYear != m_displayedDate.year()) {
        emit yearChanged();
    }
}

void Calendar::setDisplayedDate(const QDateTime &dateTime)
{
    setDisplayedDate(dateTime.date());
}

QDateTime Calendar::today() const
{
    return QDateTime(m_today);
}

void Calendar::setToday(const QDateTime &dateTime)
{
    QDate date = dateTime.date();
    if (date == m_today) {
        return;
    }

    m_today = date;
    if (m_displayedDate.isNull()) {
        resetToToday();
    } else {
        // the else is to prevent calling updateData() twice
        // if the resetToToday() was called
        updateData();
    }
    emit todayChanged();
}

void Calendar::resetToToday()
{
    m_displayedDate = m_today;
    updateData();
    emit displayedDateChanged();
}

int Calendar::types() const
{
    return m_types;
}

void Calendar::setTypes(int types)
{
    if (m_types == static_cast<Types>(types)) {
        return;
    }

//    m_types = static_cast<Types>(types);
//    updateTypes();

    emit typesChanged();
}

int Calendar::days()
{
    return m_days;
}

void Calendar::setDays(int days)
{
    if (m_days != days) {
        m_days = days;
        updateData();
        emit daysChanged();
    }
}

int Calendar::weeks() const
{
    return m_weeks;
}

void Calendar::setWeeks(int weeks)
{
    if (m_weeks != weeks) {
        m_weeks = weeks;
        updateData();
        emit weeksChanged();
    }
}

int Calendar::firstDayOfWeek() const
{
    // QML has Sunday as 0, so we need to accommodate here
    return m_firstDayOfWeek == 7 ? 0 : m_firstDayOfWeek;
}

void Calendar::setFirstDayOfWeek(int day)
{
    if (day > 7) {
        return;
    }

    if (m_firstDayOfWeek != day) {
        // QML has Sunday as 0, so we need to accommodate here
        // for QDate functions which have Sunday as 7
        if (day == 0) {
            m_firstDayOfWeek = 7;
        } else {
            m_firstDayOfWeek = day;
        }

        updateData();
        emit firstDayOfWeekChanged();
    }
}

QString Calendar::errorMessage() const
{
    return m_errorMessage;
}

int Calendar::currentWeek() const
{
    QDate date(QDate::currentDate());
    return date.weekNumber();
}

QString Calendar::dayName(int weekday) const
{
    return QLocale::system().dayName(weekday, QLocale::ShortFormat);
}

QString Calendar::monthName() const
{
    // Simple QDate::longMonthName won't do the job as it
    // will return the month name using LC_DATE locale which is used
    // for date formatting etc. So for example, in en_US locale
    // and cs_CZ LC_DATE, it would return Czech month names while
    // it should return English ones. So here we force the LANG
    // locale and take the month name from that.
    //
    // See https://bugs.kde.org/show_bug.cgi?id=353715
    const QString lang = QLocale().uiLanguages().at(0);
    // If lang is empty, it will create just a system locale
    QLocale langLocale(lang);
    return langLocale.standaloneMonthName(m_displayedDate.month());
}

int Calendar::year() const
{
    return m_displayedDate.year();
}

QAbstractListModel *Calendar::daysModel() const
{
    return m_daysModel;
}
QJsonArray Calendar::weeksModel() const
{
    return m_weekList;
}

void Calendar::updateData()
{
    if (m_days == 0 || m_weeks == 0) {
        return;
    }

    m_dayList.clear();
    m_weekList = QJsonArray();

    int totalDays = m_days * m_weeks;

    int daysBeforeCurrentMonth = 0;
    int daysAfterCurrentMonth = 0;

    QDate firstDay(m_displayedDate.year(), m_displayedDate.month(), 1);

    // If the first day is the same as the starting day then we add a complete row before it.
    if (m_firstDayOfWeek < firstDay.dayOfWeek()) {
        daysBeforeCurrentMonth = firstDay.dayOfWeek() - m_firstDayOfWeek;
    } else {
        daysBeforeCurrentMonth = days() - (m_firstDayOfWeek - firstDay.dayOfWeek());
    }

    int daysThusFar = daysBeforeCurrentMonth + m_displayedDate.daysInMonth();
    if (daysThusFar < totalDays) {
        daysAfterCurrentMonth = totalDays - daysThusFar;
    }

    if (daysBeforeCurrentMonth > 0) {
        QDate previousMonth = m_displayedDate.addMonths(-1);
        //QDate previousMonth(m_displayedDate.year(), m_displayedDate.month() - 1, 1);
        for (int i = 0; i < daysBeforeCurrentMonth; i++) {
            DayData day;
            day.isCurrent = false;
            day.dayNumber = previousMonth.daysInMonth() - (daysBeforeCurrentMonth - (i + 1));
            day.monthNumber = previousMonth.month();
            day.yearNumber = previousMonth.year();
            //      day.containsEventItems = false;
            m_dayList << day;
        }
    }

    for (int i = 0; i < m_displayedDate.daysInMonth(); i++) {
        DayData day;
        day.isCurrent = true;
        day.dayNumber = i + 1; // +1 to go form 0 based index to 1 based calendar dates
        //  day.containsEventItems = m_dayHelper->containsEventItems(i + 1);
        day.monthNumber = m_displayedDate.month();
        day.yearNumber = m_displayedDate.year();
        m_dayList << day;

    }

    if (daysAfterCurrentMonth > 0) {
        for (int i = 0; i < daysAfterCurrentMonth; i++) {
            DayData day;
            day.isCurrent = false;
            day.dayNumber = i + 1; // +1 to go form 0 based index to 1 based calendar dates
            //   day.containsEventItems = false;
            day.monthNumber = m_displayedDate.addMonths(1).month();
            day.yearNumber = m_displayedDate.addMonths(1).year();
            m_dayList << day;
        }
    }
    const int numOfDaysInCalendar = m_dayList.count();

    // Week numbers are always counted from Mondays
    // so find which index is Monday
    int mondayOffset = 0;
    if (!m_dayList.isEmpty()) {
        const DayData &data = m_dayList.at(0);
        QDate firstDay(data.yearNumber, data.monthNumber, data.dayNumber);
        // If the first day is not already Monday, get offset for Monday
        if (firstDay.dayOfWeek() != 1) {
            mondayOffset = 8 - firstDay.dayOfWeek();
        }
    }

    // Fill weeksModel with the week numbers
    for (int i = mondayOffset; i < numOfDaysInCalendar; i += 7) {
        const DayData &data = m_dayList.at(i);
        m_weekList.append(QDate(data.yearNumber, data.monthNumber, data.dayNumber).weekNumber());
    }
    emit weeksModelChanged();
    m_daysModel->update();

//    qDebug() << "---------------------------------------------------------------";
//    qDebug() << "Date obj: " << m_displayedDate;
//    qDebug() << "Month: " << m_displayedDate.month();
//    qDebug() << "m_days: " << m_days;
//    qDebug() << "m_weeks: " << m_weeks;
//    qDebug() << "Days before this month: " << daysBeforeCurrentMonth;
//    qDebug() << "Days after this month: " << daysAfterCurrentMonth;
//    qDebug() << "Days in current month: " << m_displayedDate.daysInMonth();
//    qDebug() << "m_dayList size: " << m_dayList.count();
//    qDebug() << "---------------------------------------------------------------";
}

void Calendar::nextDecade()
{
    setDisplayedDate(m_displayedDate.addYears(10));
}

void Calendar::previousDecade()
{
    setDisplayedDate(m_displayedDate.addYears(-10));
}

void Calendar::nextYear()
{
    setDisplayedDate(m_displayedDate.addYears(1));
}

void Calendar::previousYear()
{
    setDisplayedDate(m_displayedDate.addYears(-1));
}

void Calendar::nextMonth()
{
    setDisplayedDate(m_displayedDate.addMonths(1));
}

void Calendar::previousMonth()
{
    setDisplayedDate(m_displayedDate.addMonths(-1));
}

void Calendar::goToMonth(int month)
{
    setDisplayedDate(QDate(m_displayedDate.year(), month, 1));
}

void Calendar::goToYear(int year)
{
    setDisplayedDate(QDate(year, m_displayedDate.month(), 1));
}
