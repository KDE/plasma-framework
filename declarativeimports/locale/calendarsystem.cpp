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

#include "calendarsystem.h"

#include "kdebug.h"
#include "kconfiggroup.h"

#include <QtCore/QDateTime>

#include <KDateTime>

//FIXME fix all the create static methods
KCalendarSystem *CalendarSystem::create(const QString &calendarType, const KLocale *locale)
{
    return create(calendarSystem(calendarType), locale);
}

KCalendarSystem *CalendarSystem::create(const QString &calendarType, KSharedConfig::Ptr config,
                                         const KLocale *locale)
{
    return create(calendarSystem(calendarType), config, locale);
}

QString CalendarSystem::calendarLabel(const QString &calendarType)
{
    return m_calendarSystem->calendarLabel(calendarType);
}

KCalendarSystem *CalendarSystem::create(Locale::CalendarSystem calendarSystem, const KLocale *locale)
{
    return create(calendarSystem, KSharedConfig::Ptr(), locale);
}

KCalendarSystem *CalendarSystem::create(Locale::CalendarSystem calendarSystem,
                                         KSharedConfig::Ptr config,
                                         const KLocale *locale)
{
    switch (calendarSystem) {
    case Locale::QDateCalendar:
        return new KCalendarSystemQDate(config, locale);
    case Locale::CopticCalendar:
        return new KCalendarSystemCoptic(config, locale);
    case Locale::EthiopianCalendar:
        return new KCalendarSystemEthiopian(config, locale);
    case Locale::GregorianCalendar:
        return new KCalendarSystemGregorian(config, locale);
    case Locale::HebrewCalendar:
        return new KCalendarSystemHebrew(config, locale);
    case Locale::IndianNationalCalendar:
        return new KCalendarSystemIndianNational(config, locale);
    case Locale::IslamicCivilCalendar:
        return new KCalendarSystemIslamicCivil(config, locale);
    case Locale::JalaliCalendar:
        return new KCalendarSystemJalali(config, locale);
    case Locale::JapaneseCalendar:
        return new KCalendarSystemJapanese(config, locale);
    case Locale::JulianCalendar:
        return new KCalendarSystemJulian(config, locale);
    case Locale::MinguoCalendar:
        return new KCalendarSystemMinguo(config, locale);
    case Locale::ThaiCalendar:
        return new KCalendarSystemThai(config, locale);
    default:
        return new KCalendarSystemQDate(config, locale);
    }
}

QList<Locale::CalendarSystem> CalendarSystem::calendarSystemsList()
{
     QList<Locale::CalendarSystem> list;

     foreach(KLocale::CalendarSystem calendarSystem, m_calendarSystem->calendarSystemList()) {
        list.append((Locale::CalendarSystem)calendarSystem);
    }

    return list;
}

QString CalendarSystem::calendarLabel(Locale::CalendarSystem calendarSystem, const Locale *locale)
{
    return m_calendarSystem->calendarLabel((KLocale::CalendarSystem)calendarSystem, (KLocale)locale);
}

Locale::CalendarSystem CalendarSystem::calendarSystem(const QString &calendarType )
{
    return (Locale::CalendarSystem)m_calendarSystem->calendarSystem(calendarType);
}

// NOT VIRTUAL - If override needed use shared-d
Locale::CalendarSystem CalendarSystem::calendarSystem() const
{
    return (Locale::CalendarSystem)m_calendarSystem->calendarSystem();
}

// NOT VIRTUAL - If override needed use shared-d
QString CalendarSystem::calendarLabel() const
{
    return CalendarSystem::calendarLabel(calendarSystem());
}

// Dummy version using Gregorian as an example
// This method MUST be re-implemented in any new Calendar System
QDate CalendarSystem::epoch() const
{
    return m_calendarSystem->epoch();
}

QDate CalendarSystem::earliestValidDate() const
{
    return epoch();
}

// Dummy version using Gregorian as an example
// This method MUST be re-implemented in any new Calendar System
QDate CalendarSystem::latestValidDate() const
{
    // Default to Gregorian 9999-12-31
    return m_calendarSystem->latestValidDate();
}

bool CalendarSystem::isValid(int year, int month, int day) const
{
   return m_calendarSystem->isValid(year, month, day);
}

// NOT VIRTUAL - If override needed use shared-d
bool CalendarSystem::isValid(int year, int dayOfYear) const
{
    return m_calendarSystem->isValid(year, dayOfYear);
}

// NOT VIRTUAL - If override needed use shared-d
bool CalendarSystem::isValid(const QString &eraName, int yearInEra, int month, int day) const
{
    return m_calendarSystem->isValid(eraName, yearInEra, month, day);
}

// NOT VIRTUAL - If override needed use shared-d
bool CalendarSystem::isValidIsoWeekDate(int year, int isoWeekNumber, int dayOfIsoWeek) const
{
    return m_calendarSystem->isValidIsoWeekDate(year, isoWeekNumber, dayOfIsoWeek);
}

bool CalendarSystem::isValid(const QDate &date) const
{
    return m_calendarSystem->isValid(date);
}

bool CalendarSystem::setDate(QDate &date, int year, int month, int day) const
{
    return m_calendarSystem->setDate(date, year, month, day);
}

// NOT VIRTUAL - If override needed use shared-d
bool CalendarSystem::setDate(QDate &date, int year, int dayOfYear) const
{
    return m_calendarSystem->setDate(date, year, dayOfYear);
}

// NOT VIRTUAL - If override needed use shared-d
bool CalendarSystem::setDate(QDate &date, QString eraName, int yearInEra, int month, int day) const
{
    return m_calendarSystem->setDate(date, eraName, yearInEra, month, day);
}

// NOT VIRTUAL - If override needed use shared-d
bool CalendarSystem::setDateIsoWeek(QDate &date, int year, int isoWeekNumber, int dayOfIsoWeek) const
{
    m_calendarSystem->setDateIsoWeek(date, year, isoWeekNumber, dayOfIsoWeek);
}

// NOT VIRTUAL - If override needed use shared-d
void CalendarSystem::getDate(const QDate date, int *year, int *month, int *day) const
{
    return m_calendarSystem->getDate(date, year, month, day);
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

// NOT VIRTUAL - If override needed use shared-d
QString CalendarSystem::eraName(const QDate &date, StringFormat format) const
{
    return m_calendarSystem->eraName(date, format);
}

// NOT VIRTUAL - If override needed use shared-d
QString CalendarSystem::eraYear(const QDate &date, StringFormat format) const
{
    return m_calendarSystem->eraYear(date, format);
}

// NOT VIRTUAL - If override needed use shared-d
int CalendarSystem::yearInEra(const QDate &date) const
{
    return m_calendarSystem->yearInEra(date);
}

// NOT VIRTUAL - If override needed use shared-d
//FIXME we need a wrapper for KCalendarEra!!
/*QList<KCalendarEra> *CalendarSystem::eraList() const
{
    Q_D(const KCalendarSystem);

    return d->eraList();
}*/

// NOT VIRTUAL - If override needed use shared-d
/*FIXME we don't need the above.Correct?
KCalendarEra CalendarSystem::era(const QDate &eraDate) const
{
    Q_D(const KCalendarSystem);

    return d->era(eraDate);
}

// NOT VIRTUAL - If override needed use shared-d
KCalendarEra CalendarSystem::era(const QString &eraName, int yearInEra) const
{
    Q_D(const KCalendarSystem);

    return d->era(eraName, yearInEra);
}
*/
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

// NOT VIRTUAL - Uses shared-d instead
void CalendarSystem::dateDifference(const QDate &fromDate, const QDate &toDate,
                                     int *yearsDiff, int *monthsDiff, int *daysDiff, int *direction) const
{
    return m_calendarSystem->dateDifference(fromDate, toDate, yearsDiff, monthsDiff, daysDiff, direction);
}

// NOT VIRTUAL - Uses shared-d instead
int CalendarSystem::yearsDifference(const QDate &fromDate, const QDate &toDate) const
{
    return m_calendarSystem->yearsDifference(fromDate, toDate);
}

// NOT VIRTUAL - Uses shared-d instead
int CalendarSystem::monthsDifference(const QDate &fromDate, const QDate &toDate) const
{
    return m_calendarSystem->monthsDifference(fromDate, toDate);
}

// NOT VIRTUAL - Uses shared-d instead
int CalendarSystem::daysDifference(const QDate &fromDate, const QDate &toDate) const
{
    return m_calendarSystem->daysDifference(fromDate, toDate);
}

int CalendarSystem::monthsInYear(const QDate &date) const
{
    return m_calendarSystem->monthsInYear(date);
}

// NOT VIRTUAL - Uses shared-d instead
int CalendarSystem::monthsInYear(int year) const
{
    return m_calendarSystem->monthsInYear(year);
}

int CalendarSystem::weeksInYear(const QDate &date) const
{
    return weeksInYear(date, Locale::DefaultWeekNumber);
}

int CalendarSystem::weeksInYear(int year) const
{
    return weeksInYear(year, Locale::DefaultWeekNumber);
}

// NOT VIRTUAL - Uses shared-d instead
int CalendarSystem::weeksInYear(const QDate &date, Locale::WeekNumberSystem weekNumberSystem) const
{
    return m_calendarSystem->weeksInYear(date, (KLocale::WeekNumberSystem)weekNumberSystem);
}

// NOT VIRTUAL - Uses shared-d instead
int CalendarSystem::weeksInYear(int year, Locale::WeekNumberSystem weekNumberSystem) const
{
     return m_calendarSystem->weeksInYear(year, (KLocale::WeekNumberSystem)weekNumberSystem);
}

int CalendarSystem::daysInYear(const QDate &date) const
{
    return m_calendarSystem->daysInYear(date);
}

// NOT VIRTUAL - Uses shared-d instead
int CalendarSystem::daysInYear(int year) const
{
    return m_calendarSystem->daysInYear(year);
}

int CalendarSystem::daysInMonth(const QDate &date) const
{
    return m_calendarSystem->daysInMonth(date);
}

// NOT VIRTUAL - Uses shared-d instead
int CalendarSystem::daysInMonth(int year, int month) const
{
   return m_calendarSystem->daysInMonth(year, month);
}

int CalendarSystem::daysInWeek(const QDate &date) const
{
    return m_calendarSystem->daysInWeek();
}

int CalendarSystem::dayOfYear(const QDate &date) const
{
    return m_calendarSystem->dayOfYear(date);
}

int CalendarSystem::dayOfWeek(const QDate &date) const
{
    return m_calendarSystem->dayOfWeek(date);
}

int CalendarSystem::weekNumber(const QDate &date, int *yearNum) const
{
    return week(date, Locale::IsoWeekNumber, yearNum);
}

// NOT VIRTUAL - Uses shared-d instead
int CalendarSystem::week(const QDate &date, int *yearNum) const
{
    return week(date, Locale::DefaultWeekNumber, yearNum);
}

// NOT VIRTUAL - Uses shared-d instead
int CalendarSystem::week(const QDate &date, Locale::WeekNumberSystem weekNumberSystem, int *yearNum) const
{
    return m_calendarSystem->week(date, (KLocale::WeekNumberSystem)weekNumberSystem, yearNum);
}

bool CalendarSystem::isLeapYear(int year) const
{
    return m_calendarSystem->isLeapYear(year);
}

bool CalendarSystem::isLeapYear(const QDate &date) const
{
    return m_calendarSystem->isLeapYear(date);
}

// NOT VIRTUAL - If override needed use shared-d
QDate CalendarSystem::firstDayOfYear(int year) const
{
    return m_calendarSystem->firstDayOfYear(year);
}

// NOT VIRTUAL - If override needed use shared-d
QDate CalendarSystem::lastDayOfYear(int year) const
{
    return m_calendarSystem->lastDayOfYear(year);
}

// NOT VIRTUAL - If override needed use shared-d
QDate CalendarSystem::firstDayOfYear(const QDate &date) const
{
    return m_calendarSystem->firstDayOfYear(date);
}

// NOT VIRTUAL - If override needed use shared-d
QDate CalendarSystem::lastDayOfYear(const QDate &date) const
{
    return m_calendarSystem->lastDayOfYear(date);
}

// NOT VIRTUAL - If override needed use shared-d
QDate CalendarSystem::firstDayOfMonth(int year, int month) const
{
   return m_calendarSystem->firstDayOfMonth(year, month);
}

// NOT VIRTUAL - If override needed use shared-d
QDate CalendarSystem::lastDayOfMonth(int year, int month) const
{
    return m_calendarSystem->lastDayOfMonth(year, month);
}

// NOT VIRTUAL - If override needed use shared-d
QDate CalendarSystem::firstDayOfMonth(const QDate &date) const
{
   return m_calendarSystem->firstDayOfMonth(date);
}

// NOT VIRTUAL - If override needed use shared-d
QDate CalendarSystem::lastDayOfMonth(const QDate &date) const
{
    return m_calendarSystem->lastDayOfMonth(date);
}

QString CalendarSystem::monthName(int month, int year, CalendarSystem::MonthNameFormat format) const
{
    return m_calendarSystem->monthName(month, year, (KCalendarSystem::MonthNameFormat)format);
}

QString CalendarSystem::monthName(const QDate &date, MonthNameFormat format) const
{
    return m_calendarSystem->monthName(date, (KCalendarSystem::MonthNameFormat)format);
}

QString CalendarSystem::weekDayName(int weekDay, CalendarSystem::WeekDayNameFormat format) const
{
    return m_calendarSystem->weekDayName(weekDay, (KCalendarSystem::WeekDayNameFormat)format);
}

QString CalendarSystem::weekDayName(const QDate &date, WeekDayNameFormat format) const
{
   return m_calendarSystem->weekDayName(date, (KCalendarSystem::WeekDayNameFormat)format);
}

QString CalendarSystem::formatDate(const QDate &fromDate, Locale::DateFormat toFormat) const
{
    return m_calendarSystem->formatDate(fromDate, (KLocale::DateFormat)toFormat);
}

// NOT VIRTUAL - If override needed use shared-d
QString CalendarSystem::formatDate(const QDate &fromDate, const QString &toFormat,
                                    Locale::DateTimeFormatStandard standard) const
{
    return m_calendarSystem->formatDate(fromDate, toFormat, (KLocale::DateTimeFormatStandard)standar);
}

// NOT VIRTUAL - If override needed use shared-d
QString CalendarSystem::formatDate(const QDate &fromDate, const QString &toFormat, Locale::DigitSet digitSet,
                                    Locale::DateTimeFormatStandard formatStandard) const
{
    return m_calendarSystem->formatDate(fromDate, toFormat, (KLocale::DigitSet)digitSet,
                                        (KLocale::DateTimeFormatStandard)formatStandard);
}

// NOT VIRTUAL - If override needed use shared-d
QString CalendarSystem::formatDate(const QDate &date, Locale::DateTimeComponent component,
                                    Locale::DateTimeComponentFormat format,
                                    Locale::WeekNumberSystem weekNumberSystem) const
{
    return m_calendarSystem->formatDate(date, (KLocale::DateTimeComponent)component,
                                        (KLocale::DateTimeComponentFormat)format,
                                        (KLocale::WeekNumberSystem)weekNumberSystem);
}

QDate CalendarSystem::readDate(const QString &str, bool *ok) const
{
    return m_calendarSystem->readDate(str, ok);
}

QDate CalendarSystem::readDate(const QString &str, Locale::ReadDateFlags flags, bool *ok) const
{
    return m_calendarSystem->readDate(str, (KLocale::ReadDateFlags)flags, ok);
}

QDate CalendarSystem::readDate(const QString &inputString, const QString &formatString, bool *ok) const
{
    return readDate(inputString, formatString, ok, Locale::KdeFormat);
}

// NOT VIRTUAL - If override needed use shared-d
QDate CalendarSystem::readDate(const QString &inputString, const QString &formatString, bool *ok,
                                Locale::DateTimeFormatStandard formatStandard) const
{
    return m_calendarSystem->readDate(inputString, formatString, ok,
                                      (KLocale::DateTimeFormatStandard)formatStandard);
}

// NOT VIRTUAL - If override needed use shared-d
int CalendarSystem::shortYearWindowStartYear() const
{
    return m_calendarSystem->shortYearWindowStartYear();
}

// NOT VIRTUAL - If override needed use shared-d
int CalendarSystem::applyShortYearWindow(int inputYear) const
{
    return m_calendarSystem->applyShortYearWindow(inputYear);
}

int CalendarSystem::weekStartDay() const
{
    return m_calendarSystem->weekStartDay();
}

// Dummy version using Gregorian as an example
// This method MUST be re-implemented in any new Calendar System
// The implementation MUST NOT do validity checking on date ranges, all calls to this function MUST
// instead be wrapped in validity checks, as sometimes we want this to work outside the public valid
// range, i.e. to allow us to internally set dates of 1/1/10000 which are not publically valid but
// are required for internal maths
bool CalendarSystem::julianDayToDate(int jd, int &year, int &month, int &day) const
{
    return m_calendarSystem->julianDayToDate(jd, year, month, day);
}

// Dummy version using Gregorian as an example
// This method MUST be re-implemented in any new Calendar System
// The implementation MUST NOT do validity checking on date ranges, all calls to this function MUST
// instead be wrapped in validity checks, as sometimes we want this to work outside the public valid
// range, i.e. to allow us to internally set dates of 1/1/10000 which are not publically valid but
// are required for internal maths
bool CalendarSystem::dateToJulianDay(int year, int month, int day, int &jd) const
{
    return m_calendarSystem->dateToJulianDay(year, month, day, jd);
}

//FIXME what should I do with the above one?
/*const KLocale * CalendarSystem::locale() const
{
    Q_D(const KCalendarSystem);

    return d->locale();
}*/
