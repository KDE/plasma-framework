// -*- coding: iso-8859-1 -*-
/*
 *   Author: Giorgos Tsiapaliwkas <terietor@gmail.com>
 *   Date: Wed Feb 15 2012, 18:28:32
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2 or
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

import QtQuick 1.1
import org.kde.plasma.components 0.1 as PlasmaComponents
import org.kde.plasma.locale 0.1

Item {
    id: root
    property int minimumHeight: 200
    property int minimumWidth: 500
    Column {
        id: column
        width: parent.width
        height: parent.height
        anchors.horizontalCenter: root.horizontalCenter
        spacing: 20
        Text {
            id: text1
            anchors.horizontalCenter: column.horizontalCenter
            text: "<B>This is a test plasmoid for the locale bindings</B>"
            color: "black"
        }

        Text {
            id: text2
            anchors.horizontalCenter: column.horizontalCenter
            text: "<B>If you see this text,that means that every " +
            " non printable property/method has been already set. And it works!!</B>"
            color: "black"
        }

        PlasmaComponents.Button {
            id: bt1
            anchors.horizontalCenter: column.horizontalCenter
            text: "click in order to test the Locale component"
            onClicked:{
                console.log("=====Locale Component====")
                console.log("country:" + locale.country)

                locale.binaryUnitDialect = Locale.IECBinaryDialect
                locale.calendarSystem = Locale.CopticCalendar

                locale.countryDivisionCode = "AL"
                console.log("countryDivisionCode:" + locale.countryDivisionCode)

                console.log("currencyCode:" + locale.currencyCode)

                locale.currencySymbol = "$"
                console.log("currencySymbol:" + locale.currencySymbol)

                locale.dateTimeDigitSet = Locale.EasternArabicIndicDigits

                locale.dateFormat = "Y"
                console.log("dateFormat:" + locale.dateFormat)

                locale.dateFormatShort = "Y"
                console.log("dateFormatShort:" + locale.dateFormatShort)

                locale.dateMonthNamePossessive = false
                console.log("dateMonthNamePossessive:" + locale.dateMonthNamePossessive)

                locale.decimalPlaces = 2
                console.log("decimalPlaces:" + locale.decimalPlaces)

                locale.decimalSymbol = "."
                console.log("decimalSymbol:" + locale.decimalSymbol)

                locale.digitSet = Locale.EasternArabicIndicDigits
                console.log("digitSet:" + locale.digitSet)

                console.log("language:" + locale.language)

                locale.measureSystem = Locale.Imperial

                locale.monetaryDecimalPlaces = 3
                console.log("monetaryDecimalPlaces:" + locale.monetaryDecimalPlaces)

                locale.monetaryDecimalSymbol = "."
                console.log("monetaryDecimalSymbol:" + locale.monetaryDecimalSymbol)

                locale.monetaryDigitSet = Locale.EasternArabicIndicDigits

                locale.monetaryThousandsSeparator = "."
                console.log("monetaryThousandsSeparator:" + locale.monetaryThousandsSeparator)

                locale.negativeMonetarySignPosition = Locale.AfterMoney

                locale.negativePrefixCurrencySymbol = false
                console.log("negativePrefixCurrencySymbol:" + locale.negativePrefixCurrencySymbol)

                locale.negativeSign = "-"
                console.log("negativeSign:" + locale.negativeSign)

                locale.pageSize = 6
                console.log("pageSize:" + locale.pageSize)

                locale.positiveMonetarySignPosition = Locale.AfterMoney

                locale.positivePrefixCurrencySymbol = false
                console.log("positivePrefixCurrencySymbol:" + locale.positivePrefixCurrencySymbol)

                locale.positiveSign = "+"
                console.log("positiveSign:" + locale.positiveSign)

                locale.thousandsSeparator = "."
                console.log("thousandsSeparator:" + locale.thousandsSeparator)

                locale.weekDayOfPray = 10
                console.log("weekDayOfPray:" + locale.weekDayOfPray)

                locale.weekNumberSystem = Locale.FirstFullWeek

                locale.weekStartDay = 3
                console.log("weekStartDay:" + locale.weekStartDay)

                locale.workingWeekEndDay = 6
                console.log("workingWeekEndDay:" + locale.workingWeekEndDay)

                locale.workingWeekStartDay = 2
                console.log("workingWeekStartDay:" + locale.workingWeekEndDay)

                console.log("use12Clock:" + locale.use12Clock)

                console.log("defaultLanguage:" + locale.defaultLanguage)

                console.log("defaultCountry:" + locale.defaultCountry)

                console.log("defaultCurrencyCode:" + locale.defaultCurrencyCode)

                console.log("the useTranscript:" + locale.useTranscript)

                console.log("the encodingMib:" + locale.encodingMib)

                for (var i in locale.languageList) {
                    console.log("languageList:" + locale.languageList[i])
                }

                for (var i in locale.currencyCodeList) {
                    console.log("currencyCodeList:" + locale.currencyCodeList[i])
                }

                for (var i in locale.installedLanguages) {
                    console.log("installedLanguages:" + locale.installedLanguages[i])
                }

                for (var i in locale.allCountriesList) {
                    console.log("allCountriesList:" + locale.allCountriesList[i])
                }

                console.log("formatDate:" + locale.formatDate("2010-05-05"))

                console.log("formatDateTime:" + locale.formatDateTime("2010-05-05"))

                console.log("formatMoney:" + locale.formatMoney(10))

                console.log("formatLong:" + locale.formatLong(10))

                console.log("formatByteSize:" + locale.formatByteSize(10))

                console.log("formatDuration:" + locale.formatDuration(10))

                console.log("prettyFormatDuration:" + locale.prettyFormatDuration(10))

                console.log("formatLocaleTime:" + locale.formatLocaleTime("11:12:13"))

                console.log("dayPeriodText:" + locale.dayPeriodText("11:22:33"))

                console.log("readMoney:" + locale.readMoney("$ 21"))

                console.log("readNumber:" + locale.readNumber(10.0,3))

                console.log("readDate:" + locale.readDate("2004-02-01", Locale.IsoFormat))

                console.log("readTime:" + locale.readTime("11:22:33 AM"))

                console.log("readLocaleTime:" + locale.readLocaleTime("11:12:13 AM"))

                console.log("fileEncodingMib:" + locale.fileEncodingMib)

                console.log("languageCodeToName:" + locale.languageCodeToName("en_US"))

                console.log("isApplicationTranslatedInto:" + locale.isApplicationTranslatedInto("en_US"))

                console.log("removeAcceleratorMarker:" + locale.removeAcceleratorMarker("&*hello"))

                console.log("convertDigits:" + locale.convertDigits(locale.digitSet, Locale.ArabicDigits))

                for (var i in locale.allDigitSetsList) {
                    console.log("allDigitSetsList:" + locale.allDigitSetsList[i])
                }

                console.log("===========end===========")
            }
        }

        PlasmaComponents.Button {
            id: bt2
            property variant hash
            anchors.horizontalCenter: column.horizontalCenter
            text: "click in order to test the CalendarSystem component"
            onClicked:{
                console.log("=====CalendarSystem Component====")

                console.log("calendarLabel:" + calendar.calendarLabel)

                console.log("epoch:" + calendar.epoch)

                console.log("earliestValidDate:" + calendar.earliestValidDate)

                console.log("latestValidDate:" + calendar.latestValidDate)

                console.log("shortYearWindowStartYear:" + calendar.shortYearWindowStartYear)

                console.log("weekStartDay:" + locale.weekStartDay)

                console.log("isLunar:" + calendar.isLunar)

                console.log("isLunisolar:" + calendar.isLunisolar)

                console.log("isSolar:" + calendar.isSolar)

                console.log("isProleptic:" + calendar.isProleptic)
                //Q_INVOKABLE methods

                console.log("isValidIsoWeekDate:" + calendar.isValidIsoWeekDate(2012, 2, 3))

                console.log("isValid:" + calendar.isValid("2012-02-03"))

                console.log("year:" + calendar.year("2012-02-03"))

                console.log("month:" + calendar.month("2012-02-03"))

                console.log("day:" + calendar.day("2012-02-03"))

                console.log("eraName:" + calendar.eraName("2012-02-03"))

                console.log("eraYear:" + calendar.eraYear("2012-02-03"))

                console.log("yearInEra:" + calendar.yearInEra("2012-02-03"))

                console.log("addYears:" + calendar.addYears("2012-02-03", 3))

                console.log("addMonthss:" + calendar.addMonths("2012-02-03", 3))

                console.log("addDays:" + calendar.addDays("2012-02-03", 3))

                console.log("yearsDifference:" + calendar.yearsDifference("2010-02-03", "2012-02-03"))

                console.log("monthsDifference:" + calendar.monthsDifference("2012-02-03", "2012-04-03"))

                console.log("daysDifference:" + calendar.daysDifference("2012-02-03", "2012-02-13"))

                console.log("monthsInYear:" + calendar.monthsInYear(2012))

                console.log("weeksInYear:" + calendar.weeksInYear(2012, CalendarSystem.SimpleWeek))

                console.log("daysInYear:" + calendar.daysInYear("2012-02-03"))

                console.log("daysInMonth:" + calendar.daysInMonth(2012, 8))

                console.log("daysInWeek:" + calendar.daysInWeek("2012-02-03"))

                console.log("dayOfYear:" + calendar.dayOfYear("2012-02-03"))

                console.log("week:" + calendar.week("2012-02-03", CalendarSystem.SimpleWeek))

                console.log("isLeapYear:" + calendar.isLeapYear(2012))

                console.log("firstDayOfYear:" + calendar.firstDayOfYear(2012))

                console.log("lastDayOfYear:" + calendar.lastDayOfYear(2012))

                console.log("firstDayOfMonth:" + calendar.firstDayOfMonth(2012, 02))

                console.log("lastDayOfMonth:" + calendar.lastDayOfMonth(2012, 02))

                console.log("monthName:" + calendar.monthName(02, 2012))

                console.log("weekDayName:" + calendar.weekDayName(3))

                console.log("formatDate:" + calendar.formatDate("2012-02-03", CalendarSystem.Year, CalendarSystem.ShortNumber, CalendarSystem.SimpleWeek))

                console.log("readDate:" + calendar.readDate("2012-02-03", CalendarSystem.IsoFormat))

                console.log("applyShortYearWindow:" + calendar.applyShortYearWindow(50))

                console.log("calendarSystem:" + calendar.calendarSystem)
                console.log("getDate:")
                hash = calendar.getDate("2012-02-03")
                for (var i in hash) {
                    console.log("        " + i, "=", hash[i])
                }

                console.log("dateDifference:")
                hash = calendar.dateDifference("2012-01-01", "2014-03-03")
                for (var i in hash) {
                    console.log("        " + i, "=", hash[i])
                }

                for (var i in calendar.calendarSystemsList) {
                    console.log("calendarSystemsList:" + calendar.calendarSystemsList[i])
                }

                console.log("===============end===============")
            }
        }

        Locale {
            id: locale

            binaryUnitDialect: Locale.DefaultBinaryUnits
            onBinaryUnitDialectChanged: {
            console.log("the binaryUnitDialect property has been changed")
            }

            calendarSystem: Locale.QDateCalendar
            onCalendarSystemChanged: {
                console.log("the calendarSystem property has been changed")
            }

            onCurrencyCodeChanged: {
                console.log("the currencyCode property has been changed")
            }

            currencySymbol: "$"
            onCurrencySymbolChanged: {
                console.log("the currencySymbol property has been changed")
            }

            countryDivisionCode: "AD"
            onCountryDivisionCodeChanged: {
                console.log("the countryDivisionCode property has been changed")
            }

            dateFormat: "y"
            onDateFormatChanged: {
                console.log("the dateFormat property has been changed")
            }

            dateFormatShort: "y"
            onDateFormatShortChanged: {
                console.log("the dateFormatShort property has been changed")
            }

            dateMonthNamePossessive: true
            onDateMonthNamePossessiveChanged: {
                console.log("the dateMonthNamePossessive property has been changed")
            }

            dateTimeDigitSet: Locale.ArabicDigits
            onDateTimeDigitSetChanged: {
                console.log("the dateTimeDigitSet property has been changed")
            }

            decimalPlaces: 1
            onDecimalPlacesChanged: {
                console.log("the decimalPlaces property has been changed")
            }

            decimalSymbol: ","
            onDecimalSymbolChanged: {
                console.log("the decimalSymbol property has been changed")
            }

            digitSet: Locale.ArabicDigits
            onDigitSetChanged: {
                console.log("the digitSet property has been changed")
            }

            measureSystem: Locale.Metric
            onMeasureSystemChanged: {
                console.log("the measureSystem property has been changed")
            }

            monetaryDecimalPlaces: 2
            onMonetaryDecimalPlacesChanged: {
                console.log("the monetaryDecimalPlaces property has been changed")
            }

            monetaryDecimalSymbol: ","
            onMonetaryDecimalSymbolChanged: {
                console.log("the monetaryDecimalSymbol property has been changed")
            }

            monetaryDigitSet: Locale.ArabicDigits
            onMonetaryDigitSetChanged: {
                console.log("the monetaryDigitSet property has been changed")
            }

            monetaryThousandsSeparator: ","
            onMonetaryThousandsSeparatorChanged: {
                console.log("the monetaryThousandsSeparator property has been changed")
            }

            negativeMonetarySignPosition: Locale.ParensAround
            onNegativeMonetarySignPositionChanged: {
                console.log("the negativeMonetarySignPosition property has been changed")
            }

            negativePrefixCurrencySymbol: true
            onNegativePrefixCurrencySymbolChanged: {
                console.log("the negativePrefixCurrencySymbol property has been changed")
            }

            negativeSign: "-----"
            onNegativeSignChanged: {
                console.log("the negativeSign property has been changed")
            }

            pageSize: 5
            onPageSizeChanged: {
                console.log("the pageSign property has been changed")
            }

            positiveMonetarySignPosition: Locale.ParensAround
            onPositiveMonetarySignPositionChanged: {
                console.log("the positiveMonetarySignPosition property has been changed")
            }

            positivePrefixCurrencySymbol: true
            onPositivePrefixCurrencySymbolChanged: {
                console.log("the positivePrefixCurrencySymbol property has been changed")
            }

            positiveSign: " "
            onPositiveSignChanged: {
                console.log("the positiveSign property has been changed")
            }

            thousandsSeparator: ","
            onThousandsSeparatorChanged: {
                console.log("the thousandsSeparator property has been changed")
            }

            weekDayOfPray: 20
            onWeekDayOfPrayChanged: {
                console.log("the weekDayOfPray property has been changed")
            }

            weekNumberSystem: Locale.IsoWeekNumber
            onWeekNumberSystemChanged: {
                console.log("the weekNumberSystem property has been changed")
            }

            weekStartDay: 2
            onWeekStartDayChanged: {
                console.log("the weekStartDay property has been changed")
            }

            workingWeekEndDay: 5
            onWorkingWeekEndDayChanged: {
                console.log("the workingWeekEndDay property has been changed")
            }

            workingWeekStartDay: 1
            onWorkingWeekStartDayChanged: {
                console.log("the workingWeekStartDay property has been changed")
            }
        }

        CalendarSystem {
            id: calendar
        }
    }
}
