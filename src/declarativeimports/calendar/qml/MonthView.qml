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
import QtQuick.Layouts 1.1
import org.kde.plasma.calendar 2.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.extras 2.0 as PlasmaExtras

Item {
    id: root

    anchors.fill: parent

    property QtObject date
    property date showDate: new Date()

    property alias selectedMonth: calendarBackend.monthName
    property alias selectedYear: calendarBackend.year

    property int borderWidth: 1
    property real borderOpacity: 0.4

    property int columns: calendarBackend.days
    property int rows: calendarBackend.weeks

    // Take the calendar width, subtract the inner and outer spacings and divide by number of columns (==days in week)
    property int cellWidth: Math.floor((calendar.width - (root.columns + 1) * borderWidth) / (root.columns + (root.showWeekNumbers ? 1 : 0))) //prefCellWidth()

    // Take the calendar height, subtract the inner spacings and divide by number of rows (root.weeks + one row for day names)
    property int cellHeight: Math.floor((calendar.height - (root.rows + 1) * borderWidth) / (root.rows + 1)) //prefCellHeight()

    property Item selectedItem
    property int week;
    property int firstDay: new Date(showDate.getFullYear(), showDate.getMonth(), 1).getDay()
    property date today
    property bool showWeekNumbers: false


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

    Calendar {
        id: calendarBackend

        days: 7
        weeks: 6
        firstDayOfWeek: Qt.locale().firstDayOfWeek
        today: root.today
    }

    ColumnLayout {
        // This is to ensure that the inner grid.width is always aligned to be divisible by 7,
        // fixes wrong side margins because of the rounding of cell size
        // (consider the parent.width to be 404, the cell width would be 56,
        // but 56*7 + 6 (the inner spacing) is 398, so we split the remaining 6 to avoid
        // wrong alignment)
        anchors {
            fill: parent
            leftMargin: Math.floor(((parent.width - (calendar.gridColumns + 1) * borderWidth) % calendar.gridColumns) / 2)
            rightMargin: anchors.leftMargin
            bottomMargin: anchors.leftMargin
        }

        PlasmaExtras.Heading {
            id: monthHeading

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
                property int previousPixelDelta

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
                    menuLoader.item.year = selectedYear
                    menuLoader.item.open(0, height);
                }
                onExited: previousPixelDelta = 0
                onWheel: {
                    var delta = wheel.angleDelta.y || wheel.angleDelta.x
                    var pixelDelta = wheel.pixelDelta.y || wheel.pixelDelta.x

                    // For high-precision touchpad scrolling, we get a wheel event for basically every slightest
                    // finger movement. To prevent the view from suddenly ending up in the next century, we
                    // cumulate all the pixel deltas until they're larger than the label and then only change
                    // the month. Standard mouse wheel scrolling is unaffected since it's fine.
                    if (pixelDelta) {
                        if (Math.abs(previousPixelDelta) < monthMouse.height) {
                            previousPixelDelta += pixelDelta
                            return
                        }
                    }

                    if (delta >= 15) {
                        calendarBackend.previousMonth()
                    } else if (delta <= -15) {
                        calendarBackend.nextMonth()
                    }
                    previousPixelDelta = 0
                }
            }
        }

        DaysCalendar {
            id: calendar

            Layout.fillWidth: true
            Layout.fillHeight: true

            PlasmaComponents.Label {
                text: "◀"
                opacity: leftmouse.containsMouse ? 1 : 0.4
                Behavior on opacity { NumberAnimation {} }
                font.pixelSize: Math.max(theme.smallestFont.pixelSize, Math.floor(root.cellHeight / 3))
                anchors {
                    top: parent.top
                    left: parent.left
                    leftMargin: Math.floor(units.largeSpacing / 2) + root.borderWidth
                    topMargin: anchors.leftMargin
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
                font.pixelSize: Math.max(theme.smallestFont.pixelSize, Math.floor(root.cellHeight / 3))
                anchors {
                    top: parent.top
                    right: parent.right
                    rightMargin: Math.floor(units.largeSpacing / 2) + root.borderWidth
                    topMargin: anchors.rightMargin
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
