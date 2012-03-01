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
import org.kde.plasma.core 0.1 as PlasmaCore
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

                console.log("formatLocaleTime:" + locale.formatLocaleTime("2010-05-05"))

                console.log("dayPeriodText:" + locale.dayPeriodText("2010-05-05"))

                console.log("readMoney:" + locale.readMoney("one"))//TODO

                console.log("readNumber:" + locale.readNumber("one"))//TODO

                console.log("readDate:" + locale.readDate("one"))//TODO

                console.log("readTime:" + locale.readTime("one"))//TODO

                console.log("readLocaleTime:" + locale.readLocaleTime("one"))//TODO

                console.log("fileEncodingMib:" + locale.fileEncodingMib)

                console.log("languageCodeToName:" + locale.languageCodeToName("en_US"))

                console.log("isApplicationTranslatedInto:" + locale.isApplicationTranslatedInto("en_US"))

                console.log("removeAcceleratorMarker:" + locale.removeAcceleratorMarker("&*hello"))//TODO

                //console.log("convertDigits:" + locale.convertDigits) TODO

                console.log("===========end===========")
            }
        }

        PlasmaComponents.Button {
            id: bt2
            anchors.horizontalCenter: column.horizontalCenter
            text: "click in order to test the CalendarSystem component"
            onClicked:{
                console.log("=====CalendarSystem Component====")

                console.log("calendarLabel:" + calendar.CalendarLabel)

                console.log("epoch:" + calendar.epoch)

                console.log("earliestValidDate:" + calendar.earliestValidDate)

                console.log("latestValidDate:" + calendar.latestValidDate)

                console.log("shortYearWindowStartYear:" + calendar.shortYearWindowStartYear)

                console.log("weekStartDay:" + locale.weekStartDay)

                console.log("isLunar:" + calendar.isLunar)

                console.log("isLunisolar:" + calendar.isLunisolar)

                console.log("isSolar:" + calendar.isSolar)

                console.log("isProleptic:" + calendar.isProleptic)

                console.log("===============end===============")
            }
        }

        Locale {
            id: locale
            //TODO enums with negative value(like -1) doesn't work in QML!
            binaryUnitDialect: Locale.IECBinaryDialect
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

            onUse12ClockChanged: {
                console.log("the use12Clock property has been changed")
            }

            onDefaultLanguageChanged: {
                console.log("the defaultLanguage property has been changed")
            }

            onDefaultCountryChanged: {
                console.log("the defaultCountry property has been changed")
            }

            onDefaultCurrencyCodeChanged: {
                console.log("the defaultCurrencyCode property has been changed")
            }

            onUseTranscriptChanged: {
                console.log("the useTranscript property has been changed")
            }

            onCountryChanged: {
                console.log("the country property has been changed")
            }

            onLanguageChanged: {
                console.log("the language property has been changed")
            }

            onFileEncodingMibChanged: {
                console.log("the fileEncodingMib property has been changed")
            }

            onLanguageListChanged: {
                console.log("the languageList property has been changed")
            }

            onCurrencyCodeListChanged: {
                console.log("the currencyCodeList property has been changed")
            }

            onInstalledLanguagesChanged: {
                console.log("the installedLanguages property has been changed")
            }

            onAllCountriesListChanged: {
                console.log("the allCountriesList property has been changed")
            }
        }

        CalendarSystem {
            id: calendar

            onCalendarSystemChanged: {
                console.log("the calendarSystem property has been changed")
            }

            onCalendarLabelChanged: {
                console.log("the calendarLabel property has been changed")
            }
            onEpochChanged: {
                console.log("the epoch property has been changed")
            }
            onEarliestValidDateChanged: {
                console.log("the earliestValidDate property has been changed")
            }
            onLatestValidDateChanged: {
                console.log("the latestValidDate property has been changed")
            }
            onShortYearWindowStartYearChanged: {
                console.log("the shortYearWindowStartYear property has been changed")
            }
            onWeekStartDayChanged: {
                console.log("the weekStartDay property has been changed")
            }
            onIsLunarChanged: {
                console.log("the isLunar property has been changed")
            }
            onIsLunisolarChanged: {
                console.log("the isLunisolar property has been changed")
            }
            onIsSolarChanged: {
                console.log("the isSolar property has been changed")
            }
            onIsProlepticChanged: {
                console.log("the isProleptic property has been changed")
            }
        }
    }
}
