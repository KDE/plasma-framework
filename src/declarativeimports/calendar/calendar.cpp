/*
    Copyright (C) 2013 Mark Gaiser <markg85@gmail.com>

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
    , m_startDay(Qt::Sunday)
    , m_errorMessage()
{
    m_daysModel = new DaysModel(this);
    m_daysModel->setSourceData(&m_dayList);

  //  m_dayHelper = new CalendarDayHelper(this);
 //   connect(m_dayHelper, SIGNAL(calendarChanged()), this, SLOT(updateData()));
}

QDate Calendar::startDate() const
{
    return m_startDate;
}

void Calendar::setStartDate(const QDate &dateTime)
{
    if(m_startDate == dateTime) {
        return;
    }
    m_startDate = dateTime;
  //  m_dayHelper->setDate(m_startDate.year(), m_startDate.month());
    updateData();
    emit startDateChanged();
}

int Calendar::types() const
{
    return m_types;
}

void Calendar::setTypes(int types)
{
    if (m_types == types)
        return;

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
    if(m_days != days) {
        m_days = days;
        updateData();
        emit daysChanged();
    }
}

int Calendar::weeks()
{
    return m_weeks;
}

void Calendar::setWeeks(int weeks)
{
    if(m_weeks != weeks) {
        m_weeks = weeks;
        updateData();
        emit weeksChanged();
    }
}

int Calendar::startDay()
{
    return m_startDay;
}

void Calendar::setStartDay(int day)
{
    if(day > 7 || day < 1) {
        // set the errorString to some useful message and return.
        return;
    }

    if(m_startDay != day) {
        m_startDay = day;
        emit startDayChanged();
    }
}

QString Calendar::errorMessage() const
{
    return m_errorMessage;
}

QString Calendar::monthName() const
{
    return QDate::longMonthName(m_startDate.month());
}

int Calendar::year() const
{
    return m_startDate.year();
}

QAbstractListModel *Calendar::daysModel() const
{
    return m_daysModel;
}
QList<int> Calendar::weeksModel() const
{
    return m_weekList;
}

void Calendar::updateData()
{
    if(m_days == 0 || m_weeks == 0) {
        return;
    }

    m_dayList.clear();
    m_weekList.clear();

    int totalDays = m_days * m_weeks;

    int daysBeforeCurrentMonth;
    int daysAfterCurrentMonth;

    QDate firstDay(m_startDate.year(), m_startDate.month(), 1);


    // If the first day is the same as the starting day then we add a complete row before it.
    daysBeforeCurrentMonth = firstDay.dayOfWeek();

    int daysThusFar = daysBeforeCurrentMonth + m_startDate.daysInMonth();
    if(daysThusFar < totalDays) {
        daysAfterCurrentMonth = totalDays - daysThusFar;
    }

    if(daysBeforeCurrentMonth > 0) {
        QDate previousMonth = m_startDate.addMonths(-1);
        //QDate previousMonth(m_startDate.year(), m_startDate.month() - 1, 1);
        for(int i = 0; i < daysBeforeCurrentMonth; i++) {
            DayData day;
            day.isCurrentMonth = false;
            day.isNextMonth = false;
            day.isPreviousMonth = true;
            day.dayNumber = previousMonth.daysInMonth() - (daysBeforeCurrentMonth - (i + 1));
             day.monthNumber =previousMonth.month();
            day.yearNumber =previousMonth.year();
      //      day.containsEventItems = false;
            m_dayList << day;
        }
    }

    for(int i = 0; i < m_startDate.daysInMonth(); i++) {
        DayData day;
        day.isCurrentMonth = true;
        day.isNextMonth = false;
        day.isPreviousMonth = false;
        day.dayNumber = i + 1; // +1 to go form 0 based index to 1 based calendar dates
      //  day.containsEventItems = m_dayHelper->containsEventItems(i + 1);
        day.monthNumber = m_startDate.month();
        day.yearNumber = m_startDate.year();
        m_dayList << day;
        
    }

    if(daysAfterCurrentMonth > 0) {
        for(int i = 0; i < daysAfterCurrentMonth; i++) {
            DayData day;
            day.isCurrentMonth = false;
            day.isNextMonth = true;
            day.isPreviousMonth = false;
            day.dayNumber = i + 1; // +1 to go form 0 based index to 1 based calendar dates
         //   day.containsEventItems = false;
          day.monthNumber = m_startDate.addMonths(1).month();
            day.yearNumber = m_startDate.addMonths(1).year();
            m_dayList << day;
        }
    }
    const int numOfDaysInCalendar = m_dayList.count();

    // Fill weeksModel (just a QList<int>) with the week numbers. This needs some tweaking!
    for(int i = 0; i < numOfDaysInCalendar; i += 7) {
        const DayData& data = m_dayList.at(i);
        m_weekList << QDate(data.yearNumber, data.monthNumber, data.dayNumber).weekNumber();
    }


    m_daysModel->update();

//    qDebug() << "---------------------------------------------------------------";
//    qDebug() << "Date obj: " << m_startDate;
//    qDebug() << "Month: " << m_startDate.month();
//    qDebug() << "m_days: " << m_days;
//    qDebug() << "m_weeks: " << m_weeks;
//    qDebug() << "Days before this month: " << daysBeforeCurrentMonth;
//    qDebug() << "Days after this month: " << daysAfterCurrentMonth;
//    qDebug() << "Days in current month: " << m_startDate.daysInMonth();
//    qDebug() << "m_dayList size: " << m_dayList.count();
//    qDebug() << "---------------------------------------------------------------";
}
void Calendar::nextYear()
{
    m_startDate = m_startDate.addYears(1);
    updateData();
    emit startDateChanged();
}

void Calendar::previousYear()
{
    m_startDate = m_startDate.addYears(-1);
    updateData();
    emit startDateChanged();
}

void Calendar::nextMonth()
{
    m_startDate = m_startDate.addMonths(1);
    updateData();
    emit monthNameChanged();
    emit yearChanged();
}
int Calendar::currentWeek() const
{
    QDate date(QDate::currentDate());
    return date.weekNumber();
}
QString Calendar::dayName(int weekday) const
{
    return QDate::shortDayName(weekday);
}
void Calendar::previousMonth()
{
    m_startDate = m_startDate.addMonths(-1);
    updateData();
    emit monthNameChanged();
    emit yearChanged();
}
