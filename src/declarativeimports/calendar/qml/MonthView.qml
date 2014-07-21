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
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.extras 2.0 as PlasmaExtras

Item {
    id: root
    width: parent.width
    height: parent.height

    property QtObject date
    property date showDate: new Date()

    property alias selectedMonth: calendarBackend.monthName
    property alias selectedYear: calendarBackend.year

    property int mWidth: theme.mSize(theme.defaultFont).width
    property int mHeight: theme.mSize(theme.defaultFont).height
    property int borderWidth: 1
    property real borderOpacity: 0.4

    property int columns: calendarBackend.days
    property int rows: calendarBackend.weeks

    property int cellWidth: prefCellWidth()
    property int cellHeight: prefCellHeight()

    property Item selectedItem
    property int week;
    property int firstDay: new Date(showDate.getFullYear(), showDate.getMonth(), 1).getDay()
    property date today

    function prefCellWidth() {
        return Math.min(
            Math.max(
                mWidth * 3,
                // Take the calendar width, subtract the inner and outer spacings and divide by number of columns (==days in week)
                Math.floor((calendar.width - (root.columns + 1) * borderWidth) / root.columns)
            ),
            mWidth * 100
        )
    }

    function prefCellHeight() {
        return Math.min(
            Math.max(
                mHeight * 1.5,
                // Take the calendar height, subtract the inner spacings and divide by number of rows (root.weeks + one row for day names)
                Math.floor((calendar.height - (root.rows + 1) * borderWidth) / (root.rows + 1))
            ),
            mHeight * 40
        )
    }

    function isToday(date) {
        if (date.toDateString() == new Date().toDateString()) {
            return true;
        }

        return false;
    }

    function eventDate(yearNumber,monthNumber,dayNumber) {
        var d = new Date(yearNumber, monthNumber-1, dayNumber);
        return Qt.formatDate(d, "dddd dd MMM yyyy");
    }

    function resetToToday() {
        calendarBackend.resetToToday();
    }

    PlasmaExtras.Heading {
        id: monthHeading

        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
        }

        level: 1
        text: calendarBackend.displayedDate.getFullYear() == new Date().getFullYear() ? root.selectedMonth :  root.selectedMonth + ", " + root.selectedYear
        elide: Text.ElideRight
        font.capitalization: Font.Capitalize

        Loader {
            id: menuLoader
            property QtObject calendarBackend: calendarBackend
        }
        MouseArea {
            id: monthMouse
            width: monthHeading.paintedWidth
            anchors {
                left: parent.left
                top: parent.top
                bottom: parent.bottom
            }
            onClicked: {
                if (menuLoader.source == "") {
                    menuLoader.source = "MonthMenu.qml"
                }
                menuLoader.item.open(0, height);
            }
        }
    }

    Calendar {
        id: calendarBackend

        days: 7
        weeks: 6
        firstDayOfWeek: Qt.locale().firstDayOfWeek
        today: root.today
    }

    DaysCalendar {
        id: calendar

        anchors {
            top: monthHeading.bottom
            left: parent.left
            right: parent.right
            bottom: parent.bottom
        }

        PlasmaComponents.Label {
            text: "◀"
            opacity: leftmouse.containsMouse ? 1 : 0.4
            Behavior on opacity { NumberAnimation {} }
            anchors {
                top: parent.top
                left: parent.left
                leftMargin: Math.floor(units.largeSpacing / 2)
            }
            MouseArea {
                id: leftmouse
                anchors.fill: parent
                anchors.margins: -units.largeSpacing / 3
                hoverEnabled: true
                onClicked: {
                    calendarBackend.previousMonth()
                }
            }
        }
        PlasmaComponents.Label {
            text: "▶"
            opacity: rightmouse.containsMouse ? 1 : 0.4
            Behavior on opacity { NumberAnimation {} }
            anchors {
                top: parent.top
                right: parent.right
                rightMargin: Math.floor(units.largeSpacing / 2)
            }
            MouseArea {
                id: rightmouse
                anchors.fill: parent
                anchors.margins: -units.largeSpacing / 3
                hoverEnabled: true
                onClicked: {
                    calendarBackend.nextMonth()
                }
            }
        }
    }

/*
    Item {
        id: calendarToolbar
        visible: false
        anchors {
            left: parent.left
            right: parent.right
            bottomMargin: 20
            bottom: parent.bottom
        }

        PlasmaComponents.ToolButton {
            id: currentDate
            iconSource: "view-pim-calendar"
            width: height
            onClicked: {
                calendarBackend.startDate = today();
            }
            PlasmaCore.ToolTipArea {
                id: tool
                anchors.fill: currentDate
                mainText: "Select Today"
            }
            anchors {
                left: parent.left
            }
        }

        PlasmaComponents.TextField {
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

        PlasmaComponents.TextField {
            id: weekField
            text: week == 0 ? calendarBackend.currentWeek(): week
            width: calendarOperations.width/10
            anchors {
                right: parent.right
            }
        }
    }
    */
}
