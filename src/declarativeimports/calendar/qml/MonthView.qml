/*
 * Copyright 2013  Heena Mahour <heena393@gmail.com>
 * Copyright 2013 Sebastian Kügler <sebas@kde.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
import QtQuick 2.0
import org.kde.plasma.calendar 2.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as Components
import org.kde.plasma.extras 2.0 as PlasmaExtras

Item {
    id: root
    width: parent.width
    height: parent.height
    property string date ;
    property date showDate: new Date()
  
    property alias calendarGrid: calendarGrid
    property int mWidth: theme.mSize(theme.defaultFont).width
    property int mHeight: theme.mSize(theme.defaultFont).height
    property int borderWidth: 1

    property int columns: monthCalendar.days
    property int rows: 1 + monthCalendar.weeks

    property int cellWidth: prefCellWidth()
    property int cellHeight: prefCellHeight()

    property int miniumWidth: implicitWidth
    property int miniumHeight: implicitHeight
    property int implicitWidth: theme.mSize(theme.defaultFont).width * 6 * 8
    property int implicitHeight: theme.mSize(theme.defaultFont).height * 2 * 9

    //property int cellFontPixelSize: theme.defaultFont.pixelSize
 
    property Item selectedItem
    property int week;
    property int firstDay: new Date(showDate.getFullYear(), showDate.getMonth(), 1).getDay()
    
    anchors.margins: theme.largeSpacing * 3

    function prefCellWidth() {
        return Math.min(
            Math.max(
                mWidth * 4,
                calendarGrid.width / (root.columns)
            ),
            mWidth * 100
        )
    }

    function prefCellHeight() {
        return Math.min(
            Math.max(
                mHeight * 1.5,
                calendarGrid.height / (root.rows + 1)
            ),
            mHeight * 40
        )
    }

    function isToday(date) {
        if (date == Qt.formatDateTime(new Date(), "d/M/yyyy")) {
            return true;
        }
        else return false;
    }

    function isTodayMonth() {
        return Qt.formatDateTime(new Date(), "yyyy-MM-dd")
    }

    function eventDate(yearNumber,monthNumber,dayNumber) {
        var d = new Date(yearNumber, monthNumber-1, dayNumber);
        return Qt.formatDate(d, "dddd dd MMM yyyy");
    }

    //Rectangle { anchors.fill: monthCalendar; color: "green"; opacity: 0.3 }

    PlasmaExtras.Heading {
        id: monthHeading

        anchors {
            top: parent.top
            left: calendarGrid.left
            right: parent.right
            leftMargin: -borderWidth
        }

        level: 1
        opacity: 0.8
        text: monthCalendar.monthName
    }

    Calendar {
        id: monthCalendar

        days: 7
        weeks: 6
        startDay: 1
        startDate: "2013-08-01"
        onStartDateChanged: {
            //monthHeading.text = monthName
            month.text = monthName
            monthYear.text = year
        }
    }

//     CalendarToolbar {
//
//     }

    //Rectangle { anchors.fill: calendarGrid; color: "orange"; opacity: 0.3; }
    DaysCalendar {
        id: calendarGrid
        anchors {
            top: monthHeading.bottom
            left: parent.left
            right: parent.right
            bottom: calendarToolbar.top
            margins: theme.largeSpacing
            bottomMargin: theme.largeSpacing * 3
            topMargin: - (theme.largeSpacing / 2)
        }

    }

    Item {
        id: calendarToolbar
        anchors {
            left: parent.left
            right: parent.right
            bottomMargin: 20
            bottom: parent.bottom
        }

        Components.ToolButton {
            id: currentDate
            iconSource: "view-pim-calendar"
            width: height
            onClicked: {
                monthCalendar.startDate = isTodayMonth();
            }
            PlasmaCore.ToolTip {
                id: tool
                target: currentDate
                mainText: "Select Today"
            }
            anchors {
                left: parent.left
            }
        }

        Components.TextField {
            id: dateField
            text: date == "" ? Qt.formatDateTime ( new Date(), "d/M/yyyy" ): date
            width: calendarOperations.width/3
            anchors {
                leftMargin: 20
                rightMargin: 30
                left: currentDate.right
                right: weekField.left
            }
        }

        Components.TextField {
            id: weekField
            text: week == 0 ? monthCalendar.currentWeek(): week
            width: calendarOperations.width/10
            anchors {
                right: parent.right
            }
        }
    }

}
