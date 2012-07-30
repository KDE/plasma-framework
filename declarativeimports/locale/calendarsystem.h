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

#ifndef CALENDARSYSTEM_H
#define CALENDARSYSTEM_H

//own
#include "locale_p.h"  // needed for enums

//Qt
#include <QtCore/QDate>

class KCalendarSystem;

class CalendarSystem : public QObject
{
Q_OBJECT

//enums
Q_ENUMS(StringFormat)
Q_ENUMS(MonthNameFormat)
Q_ENUMS(WeekDayNameFormat)
Q_ENUMS(WeekNumberSystem)
Q_ENUMS(ReadDateFlags)
Q_ENUMS(DateTimeComponent)
Q_ENUMS(DateTimeComponentFormat)

//properties
Q_PROPERTY(QList<int> calendarSystemsList READ calendarSystemsList CONSTANT) //read-only
Q_PROPERTY(Locale::CalendarSystem calendarSystem READ calendarSystem CONSTANT)//read-only
Q_PROPERTY(QString calendarLabel READ calendarLabel CONSTANT)//read-only
Q_PROPERTY(QDate epoch READ epoch CONSTANT)//read-only
Q_PROPERTY(QDate earliestValidDate READ earliestValidDate CONSTANT)//read-only
Q_PROPERTY(QDate latestValidDate READ latestValidDate CONSTANT)//read-only
Q_PROPERTY(int shortYearWindowStartYear READ shortYearWindowStartYear CONSTANT)
Q_PROPERTY(int weekStartDay READ weekStartDay CONSTANT)//read-only
Q_PROPERTY(bool isLunar READ isLunar CONSTANT)//read-only
Q_PROPERTY(bool isLunisolar READ isLunisolar CONSTANT)//read-only
Q_PROPERTY(bool isSolar READ isSolar CONSTANT)//read-only
Q_PROPERTY(bool isProleptic READ isProleptic CONSTANT)//read-only

public:

    //ctor
     CalendarSystem(QObject *parent = 0);

     //the above are enums which I borrow from the Locale class
     /**
      *
      * System used for Week Numbers
      */
     enum WeekNumberSystem {
         DefaultWeekNumber = 1000, /**< The system locale default */
         IsoWeekNumber     =  0, /**< ISO Week Number */
         FirstFullWeek     =  1, /**< Week 1 starts on the first Week Start Day in year ends after 7 days */
         FirstPartialWeek  =  2, /**< Week 1 starts Jan 1st ends day before first Week Start Day in year */
         SimpleWeek        =  3  /**< Week 1 starts Jan 1st ends after 7 days */
     };

     enum ReadDateFlags {
        NormalFormat          =    1, /**< Only accept a date string in
                                           the locale LongDate format */
        shortFormat           =    2, /**< Only accept a date string in
                                           the locale ShortDate format */
        IsoFormat             =    4, /**< Only accept a date string in
                                           ISO date format (YYYY-MM-DD) */
        IsoWeekFormat         =    8, /**< Only accept a date string in
                                           ISO Week date format (YYYY-Www-D) */
        IsoOrdinalFormat      =   16  /**< Only accept a date string in
                                           ISO Week date format (YYYY-DDD) */
    };

    /**
     * The various Components that make up a Date / Time
     * In the future the Components may be combined as flags for dynamic
     * generation of Date Formats.
     */
    enum DateTimeComponent {
        Year          = 0x1,        /**< The Year portion of a date, may be number or name */
        YearName      = 0x2,        /**< The Year Name portion of a date */
        Month         = 0x4,        /**< The Month portion of a date, may be number or name */
        MonthName     = 0x8,        /**< The Month Name portion of a date */
        Day           = 0x10,       /**< The Day portion of a date, may be number or name */
        DayName       = 0x20,       /**< The Day Name portion of a date */
        JulianDay     = 0x40,       /**< The Julian Day of a date */
        EraName       = 0x80,       /**< The Era Name portion of a date */
        EraYear       = 0x100,      /**< The Era and Year portion of a date */
        YearInEra     = 0x200,      /**< The Year In Era portion of a date */
        DayOfYear     = 0x400,      /**< The Day Of Year portion of a date, may be number or name */
        DayOfYearName = 0x800,      /**< The Day Of Year Name portion of a date */
        DayOfWeek     = 0x1000,     /**< The Day Of Week / Weekday portion of a date, may be number or name */
        DayOfWeekName = 0x2000,     /**< The Day Of Week Name / Weekday Name portion of a date */
        Week          = 0x4000,     /**< The Week Number portion of a date */
        WeekYear      = 0x8000,     /**< The Week Year portion of a date */
        MonthsInYear  = 0x10000,    /**< The Months In Year portion of a date */
        WeeksInYear   = 0x20000,    /**< The Weeks In Year portion of a date */
        DaysInYear    = 0x40000,    /**< The Days In Year portion of a date */
        DaysInMonth   = 0x80000,    /**< The Days In Month portion of a date */
        DaysInWeek    = 0x100000,   /**< The Days In Week portion of a date */
        Hour          = 0x200000,   /**< The Hours portion of a date */
        Minute        = 0x400000,   /**< The Minutes portion of a date */
        Second        = 0x800000,   /**< The Seconds portion of a date */
        Millisecond   = 0x1000000,  /**< The Milliseconds portion of a date */
        DayPeriod     = 0x2000000,  /**< The Day Period portion of a date, e.g. AM/PM */
        DayPeriodHour = 0x4000000,  /**< The Day Period Hour portion of a date */
        Timezone      = 0x8000000,  /**< The Time Zone portion of a date, may be offset or name */
        TimezoneName  = 0x10000000, /**< The Time Zone Name portion of a date */
        UnixTime      = 0x20000000  /**< The UNIX Time portion of a date */
    };

    enum DateTimeComponentFormat {
        DefaultComponentFormat = 1000, /**< The system locale default for the componant */
        ShortNumber = 0,             /**< Number at its natural width, e.g. 2 for the 2nd*/
        LongNumber,                  /**< Number padded to a required width, e.g. 02 for the 2nd*/
        narrowName = 3,              /**< Narrow text format, may not be unique, e.g. M for Monday */
        shortName,                   /**< Short text format, e.g. Mon for Monday */
        longName                     /**< Long text format, e.g. Monday for Monday */
        };
    //end of the borrowing enums

    /**
     * Format for returned year number / month number / day number as string.
     */
    enum StringFormat {
        ShortFormat,      /**< Short string format, e.g. 2000 = "00" or 6 = "6" */
        LongFormat        /**< Long string format, e.g. 2000 = "2000" or 6 = "06" */
    };

    /**
     * Format for returned month / day name.
     */
    enum MonthNameFormat {
        ShortName,                /**< Short name format, e.g. "Dec" */
        LongName,                 /**< Long name format, e.g. "December" */
        ShortNamePossessive,      /**< Short name possessive format, e.g. "of Dec" */
        LongNamePossessive,       /**< Long name possessive format, e.g. "of December" */
        NarrowName                /**< Narrow name format, e.g. "D" */
    };

    /**
     * Format for returned month / day name.
     */
    enum WeekDayNameFormat {
        ShortDayName,                /**< Short name format, e.g. "Fri" */
        LongDayName,                 /**< Long name format, e.g. "Friday" */
        NarrowDayName                /**< Narrow name format, e.g. "F" */
    };

    /**
     * Returns the list of currently supported Calendar Systems
     * @return list of Calendar Systems
     */
    QList<int> calendarSystemsList();

    /**
     *
     * Returns the Calendar System type of the CalendarSystem object
     *
     * @return type of calendar system
     */
    Locale::CalendarSystem calendarSystem() const;

    /**
     *
     * Returns a localized label to display for the current Calendar System type.
     *
     * @return localized label for this Calendar System
     */
    QString calendarLabel() const; //TODO, it returns undefined

    /**
     * Returns a QDate holding the epoch of the calendar system.  Usually YMD
     * of 1/1/1, access the returned QDates method toJulianDay() if you
     * require the actual Julian day number.  Note: a particular calendar
     * system implementation may not include the epoch in its supported range,
     * or the calendar system may be proleptic in which case it supports dates
     * before the epoch.
     *
     * @see CalendarSystem::earliestValidDate
     * @see CalendarSystem::latestValidDate
     * @see CalendarSystem::isProleptic
     * @see CalendarSystem::isValid
     *
     * @return epoch of calendar system
     */
    QDate epoch() const;

    /**
     * Returns the earliest date valid in this calendar system implementation.
     *
     * If the calendar system is proleptic then this may be before epoch.
     *
     * @see CalendarSystem::epoch
     * @see CalendarSystem::latestValidDate
     *
     * @return date the earliest valid date
     */
    QDate earliestValidDate() const;

    /**
     * Returns the latest date valid in this calendar system implementation.
     *
     * @see CalendarSystem::epoch
     * @see CalendarSystem::earliestValidDate
     *
     * @return date the latest valid date
     */
     QDate latestValidDate() const;

    /**
     *
     * Returns whether a given date is valid in this calendar system.
     *
     * @param year the year portion of the date to check
     * @param isoWeekNumber the ISO week portion of the date to check
     * @param dayOfIsoWeek the day of week portion of the date to check
     * @return @c true if the date is valid, @c false otherwise
     */
    Q_INVOKABLE bool isValidIsoWeekDate(int year, int isoWeekNumber, int dayOfIsoWeek) const;

    /**
     * Returns whether a given date is valid in this calendar system.
     *
     * @param date the date to check
     * @return @c true if the date is valid, @c false otherwise
     */
    Q_INVOKABLE  bool isValid(const QDate &date) const;

    /**
     *
     * Returns the year, month and day portion of a given date in the current calendar system
     * The values are returned in a hash, the available keys are,
     * ["year"] the year of the date
     * ["month"] the month of the date
     * ["day"] the day of the date
     * @param date date to get year, month and day for
     */
    Q_INVOKABLE QVariantHash getDate(const QDate date) const;

    /**
     * Returns the year portion of a given date in the current calendar system
     *
     * @param date date to return year for
     * @return year, 0 if input date is invalid
     */
    Q_INVOKABLE int year(const QDate &date) const;

    /**
     * Returns the month portion of a given date in the current calendar system
     *
     * @param date date to return month for
     * @return month of year, 0 if input date is invalid
     */
    Q_INVOKABLE int month(const QDate &date) const;

    /**
     * Returns the day portion of a given date in the current calendar system
     *
     * @param date date to return day for
     * @return day of the month, 0 if input date is invalid
     */
    Q_INVOKABLE int day(const QDate &date) const;

    /**
     *
     * Returns the Era Name portion of a given date in the current calendar system,
     * for example "AD" or "Anno Domini" for the Gregorian calendar and Christian Era.
     *
     * @param date date to return Era Name for
     * @param format format to return, either short or long
     * @return era name, empty string if input date is invalid
     */
    Q_INVOKABLE QString eraName(const QDate &date, StringFormat format = ShortFormat) const;

    /**
     *
     * Returns the Era Year portion of a given date in the current
     * calendar system, for example "2000 AD" or "Heisei 22".
     *
     * @param date date to return Era Year for
     * @param format format to return, either short or long
     * @return era name, empty string if input date is invalid
     */
    Q_INVOKABLE QString eraYear(const QDate &date, StringFormat format = ShortFormat) const;

    /**
     *
     * Returns the Year In Era portion of a given date in the current calendar
     * system, for example 1 for "1 BC".
     *
     * @param date date to return Year In Era for
     * @return Year In Era, -1 if input date is invalid
     */
    Q_INVOKABLE int yearInEra(const QDate &date) const;

    /**
     * Returns a QDate containing a date @p nyears years later.
     *
     * @param date The old date
     * @param nyears The number of years to add
     * @return The new date, null date if any errors
     */
    Q_INVOKABLE QDate addYears(const QDate &date, int nyears) const;

    /**
     * Returns a QDate containing a date @p nmonths months later.
     *
     * @param date The old date
     * @param nmonths number of months to add
     * @return The new date, null date if any errors
     */
    Q_INVOKABLE  QDate addMonths(const QDate &date, int nmonths) const;

    /**
     * Returns a QDate containing a date @p ndays days later.
     *
     * @param date The old date
     * @param ndays number of days to add
     * @return The new date, null date if any errors
     */
    Q_INVOKABLE  QDate addDays(const QDate &date, int ndays) const;

    /**
     * Returns the difference between two dates with a hash, the available keys are
     * ["years"] Returns number of years difference
     * ["months"] Returns number of months difference
     * ["days"] Returns number of days difference
     * ["direction"] Returns direction of difference, 1 if fromDate <= toDate, -1 otherwise
     * The difference is always caculated from the earlier date to the later
     * date in year, month and day order, with the @p direction parameter
     * indicating which direction the difference is applied from the @p toDate.
     *
     * For example, the difference between 2010-06-10 and 2012-09-5 is 2 years,
     * 2 months and 26 days.  Note that the difference between two last days of
     * the month is always 1 month, e.g. 2010-01-31 to 2010-02-28 is 1 month
     * not 28 days.
     *
     * @param fromDate The date to start from
     * @param toDate The date to end at
     */
    Q_INVOKABLE QVariantHash dateDifference(const QDate &fromDate, const QDate &toDate)const;

    /**
    * Returns the difference between two dates in completed calendar years.
    * The returned value will be negative if @p fromDate > @p toDate.
    *
    * For example, the difference between 2010-06-10 and 2012-09-5 is 2 years.
    *
    * @param fromDate The date to start from
    * @param toDate The date to end at
    * @return The number of years difference
    */
    Q_INVOKABLE int yearsDifference(const QDate &fromDate, const QDate &toDate) const;

    /**
     * Returns the difference between two dates in completed calendar months
     * The returned value will be negative if @p fromDate > @p toDate.
     *
     * For example, the difference between 2010-06-10 and 2012-09-5 is 26 months.
     * Note that the difference between two last days of the month is always 1
     * month, e.g. 2010-01-31 to 2010-02-28 is 1 month not 28 days.
     *
     * @param fromDate The date to start from
     * @param toDate The date to end at
     * @return The number of months difference
     */
    Q_INVOKABLE int monthsDifference(const QDate &fromDate, const QDate &toDate) const;

    /**
     * Returns the difference between two dates in days
     * The returned value will be negative if @p fromDate > @p toDate.
     *
     * @param fromDate The date to start from
     * @param toDate The date to end at
     * @return The number of days difference
     */
    Q_INVOKABLE int daysDifference(const QDate &fromDate, const QDate &toDate) const;

    /**
     *
     * Returns number of months in the given year
     *
     * @param year the required year
     * @return number of months in the year, -1 if input date invalid
     */
    Q_INVOKABLE int monthsInYear(int year) const;

    /**
     *
     * Returns the number of Weeks in a year using the required Week Number System.
     *
     * Unless you specifically want a particular Week Number System (e.g. ISO Weeks)
     * you should use the localized number of weeks provided by weeksInYear().
     *
     * @see week()
     * @see formatDate()
     * @param year the year
     * @param weekNumberSystem the week number system to use
     * @return number of weeks in the year, -1 if  date invalid
     */
    Q_INVOKABLE int weeksInYear(int year, WeekNumberSystem weekNumberSystem) const;

    /**
     *
     * Returns the number of days in the given year.
     *
     * @param year the year
     * @return number of days in year, -1 if input date invalid
     */
    Q_INVOKABLE int daysInYear(int year) const;

    /**
     *
     * Returns the number of days in the given month.
     *
     * @param year the year the month is in
     * @param month the month
     * @return number of days in month, -1 if input date invalid
     */
    Q_INVOKABLE int daysInMonth(int year, int month) const;

    /**
     * Returns the number of days in the given week.
     *
     * @param date the date to obtain week from
     * @return number of days in week, -1 if input date invalid
     */
    Q_INVOKABLE int daysInWeek(const QDate &date) const;

    /**
     * Returns the day number of year for the given date
     *
     * The days are numbered 1..daysInYear()
     *
     * @param date the date to obtain day from
     * @return day of year number, -1 if input date not valid
     */
    Q_INVOKABLE int dayOfYear(const QDate &date) const;

    /**
     * Returns the weekday number for the given date
     *
     * The weekdays are numbered 1..7 for Monday..Sunday.
     *
     * This value is @em not affected by the value of weekStartDay()
     *
     * @param date the date to obtain day from
     * @return day of week number, -1 if input date not valid
     */
    Q_INVOKABLE int dayOfWeek(const QDate &date) const;

    /**
     * Returns the Week Number for the date in the required Week Number System.
     *
     * Unless you want a specific Week Number System (e.g. ISO Week), you should
     * use the localized Week Number form of week().
     *
     * If the date falls in the last week of the previous year or the first
     * week of the following year, then the yearNum returned will be set to the
     * appropriate year.
     *
     * Technically, the ISO Week Number only applies to the ISO/Gregorian Calendar
     * System, but the same rules will be applied to the current Calendar System.
     *
     * @see weeksInYear()
     * @see formatDate()
     * @param date the date to obtain week from
     * @param weekNumberSystem the Week Number System to use
     * @param yearNum returns the year the date belongs to
     * @return week number, -1 if input date invalid
     */
    Q_INVOKABLE int week(const QDate &date, WeekNumberSystem weekNumberSystem) const;

    /**
     * Returns whether a given year is a leap year.
     *
     * Input year must be checked for validity in current Calendar System prior to calling, no
     * validity checking performed in this routine, behaviour is undefined in invalid case.
     *
     * @param year the year to check
     * @return @c true if the year is a leap year, @c false otherwise
     */
    Q_INVOKABLE bool isLeapYear(int year) const;

    /**
     *
     * Returns a QDate containing the first day of the year
     *
     * @param year The year to return the date for
     * @return The first day of the year
     */
    Q_INVOKABLE QDate firstDayOfYear(int year) const;

    /**
     *
     * Returns a QDate containing the last day of the year
     *
     * @param year The year to return the date for
     * @return The last day of the year
     */
    Q_INVOKABLE QDate lastDayOfYear(int year) const;

    /**
     *
     * Returns a QDate containing the first day of the month
     *
     * @param year The year to return the date for
     * @param month The month to return the date for
     * @return The first day of the month
     */
    Q_INVOKABLE QDate firstDayOfMonth(int year, int month) const;

    /**
     *
     * Returns a QDate containing the last day of the month
     *
     * @param year The year to return the date for
     * @param month The month to return the date for
     * @return The last day of the month
     */
    Q_INVOKABLE QDate lastDayOfMonth(int year, int month) const;

    /**
     * Gets specific calendar type month name for a given month number
     * If an invalid month is specified, QString() is returned.
     *
     * @param month the month number
     * @param year the year the month belongs to
     * @param format specifies whether the short month name or long month name should be used
     * @return name of the month, empty string if any error
     */
    Q_INVOKABLE QString monthName(int month, int year, MonthNameFormat format = LongName) const;

    /**
     * Gets specific calendar type week day name.
     * If an invalid week day is specified, QString() is returned.
     *
     * @param weekDay number of day in week (Monday = 1, ..., Sunday = 7)
     * @param format specifies whether the short month name or long month name should be used
     * @return day name, empty string if any error
     */
    Q_INVOKABLE QString weekDayName(int weekDay, WeekDayNameFormat format = LongDayName) const;

    /**
     *
     * Returns a Date Component as a localized string in the requested format.
     *
     * For example for 2010-01-01 the Locale::Month with en_US Locale and Gregorian calendar may return:
     *   CalendarSystem::ShortNumber = "1"
     *   CalendarSystem::LongNumber  = "01"
     *   CalendarSystem::NarrowName  = "J"
     *   CalendarSystem::ShortName   = "Jan"
     *   CalendarSystem::LongName    = "January"
     *
     * @param date The date to format
     * @param component The date component to return
     * @param format The format to return the @p component in
     * @param weekNumberSystem To override the default Week Number System to use
     * @return The localized string form of the date component
     */
    Q_INVOKABLE QString formatDate(const QDate &date, DateTimeComponent component,
                       DateTimeComponentFormat format, WeekNumberSystem weekNumberSystem) const;

    /**
     * Converts a localized date string to a QDate.
     * This method is stricter than readDate(str,&ok): it will either accept
     * a date in full format or a date in short format, depending on @p flags.
     *
     * Uses the calendar system's internal locale set when the instance was
     * created, which ensures that the correct calendar system and locale
     * settings are respected, which would not occur in some cases if using
     * the global locale.  Defaults to global locale.
     *
     * @see Locale::readDate
     *
     * @param str the string to convert
     * @param flags whether the date string is to be in full format or in short format
     * @param ok if non-null, will be set to @c true if the date is valid, @c false if invalid
     *
     * @return the string converted to a QDate
     */
    Q_INVOKABLE QDate readDate(const QString &str, ReadDateFlags flags) const;

    /**
     *
     * Returns the Short Year Window Start Year for the current Calendar System.
     *
     * Use this function to get the Start Year for the Short Year Window to be
     * applied when 2 digit years are entered for a Short Year input format,
     * e.g. if the Short Year Window Start Year is 1930, then the input Short
     * Year value of 40 is interpreted as 1940 and the input Short Year value
     * of 10 is interpreted as 2010.
     *
     * The Short Year Window is only ever applied when reading the Short Year
     * format and not the Long Year format, i.e. Locale::ShortFormat or '%y'
     * only and not Locale::LongFormat or '%Y'.
     *
     * The Start Year 0 effectively means not to use a Short Year Window
     *
     * Each Calendar System requires a different Short Year Window as they have
     * different epochs. The Gregorian Short Year Window usually pivots around
     * the year 2000, whereas the Hebrew Short Year Window usually pivots around
     * the year 5000.
     *
     * This value must always be used when evaluating user input Short Year
     * strings.
     *
     * @see Locale::shortYearWindowStartYear
     * @see Locale::applyShortYearWindow
     * @return the short year window start year
     */
    int shortYearWindowStartYear() const;

    /**
     *
     * Returns the Year Number after applying the Year Window.
     *
     * If the @p inputYear is between 0 and 99, then apply the Year Window and
     * return the calculated Year Number.
     *
     * If the @p inputYear is not between 0 and 99, then the original Year Number
     * is returned.
     *
     * @see Locale::setYearWindowOffset
     * @see Locale::yearWindowOffset
     * @param inputYear the year number to apply the year window to
     * @return the year number after applying the year window
     */
    Q_INVOKABLE int applyShortYearWindow(int inputYear) const;

    /**
     * Use this to determine which day is the first day of the week.
     *
     * Uses the calendar system's internal locale set when the instance was
     * created, which ensures that the correct calendar system and locale
     * settings are respected, which would not occur in some cases if using
     * the global locale.  Defaults to global locale.
     *
     * @see Locale::weekStartDay
     *
     * @return an integer (Monday = 1, ..., Sunday = 7)
     */
     int weekStartDay() const;

    /**
     * Returns whether the calendar is lunar based.
     *
     * @return @c true if the calendar is lunar based, @c false if not
     */
    bool isLunar() const;

    /**
     * Returns whether the calendar is lunisolar based.
     *
     * @return @c true if the calendar is lunisolar based, @c false if not
     */
    bool isLunisolar() const;

    /**
     * Returns whether the calendar is solar based.
     *
     * @return @c true if the calendar is solar based, @c false if not
     */
    bool isSolar() const;

    /**
     * Returns whether the calendar system is proleptic, i.e. whether dates
     * before the epoch are supported.
     *
     * @see CalendarSystem::epoch
     *
     * @return @c true if the calendar system is proleptic, @c false if not
     */
    bool isProleptic() const;

private:
    KCalendarSystem *m_calendarSystem;
};

#endif
