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
    property int minimumHeight:200
    property int minimumWidth:500
    Column {
        id: column
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
            text: "<B>If you see this text,that means that every" +
            "non printable property/method has been already set. And it works!!</B>"
            color: "black"
        }

        PlasmaComponents.Button {
            id: bt1
            anchors.horizontalCenter: column.horizontalCenter
            text: "click in order to test the Locale component"
            onClicked:{
                console.log("=====Locale Component====")
                console.log("country:" + locale.country)

               // locale.binaryUnitDialect = Locale.BinaryUnitDialect.IECBinaryDialect
               // locale.calendarSystem = CalendarSystem.CopticCalendar

                locale.countryDivisionCode = "AL"
                console.log("countryDivisionCode:" + locale.countryDivisionCode)

                //locale.currencyCode = "AFN" //TODO add the right value
                //console.log("currencyCode:" + locale.currencyCode)

                //locale.currencySymbol = TODO
                //console.log("currencySymbol" + locale.currencySymbol)

                locale.dateFormat = "Y"
                console.log("dateFormat:" + locale.dateFormat)

                locale.dateFormatShort = "Y"
                console.log("dateFormatShort:" + locale.dateFormatShort)

                locale.dateMonthNamePossessive = false
                console.log("dateMonthNamePossessive:" + locale.dateMonthNamePossessive)

                console.log("===========end===========")
            }
        }

        PlasmaComponents.Button {
            id: bt2
            anchors.horizontalCenter: column.horizontalCenter
            text: "click in order to test the CalendarSystem component"
            onClicked:{
                console.log("=====CalendarSystem Component====")
                console.log("===============end===============")
            }
        }

        Locale {
            id: locale
            binaryUnitDialect: BinaryUnitDialect.DefaultBinaryDialect
            onBinaryUnitDialectChanged: {
                console.log("the binaryUnitDialect property has been changed")
            }

            calendarSystem: Locale.QDateCalendar
            onCalendarSystemChanged: {
                console.log("the calendarSystem property has been changed")
            }

            //TODO:Add the proper value
            /*currencyCode: "AED"
            onCurrencyCodeChanged: {
                console.log("the currencyCode property has been changed")
            }*/

            /*TODO
            currencySymbol: ""
            onCurrencySymbolChanged: {
                console.log("the currencySymbol property has been changed")
            }*/

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

    }

        CalendarSystem {
            id: calendar
        }
    }
}
