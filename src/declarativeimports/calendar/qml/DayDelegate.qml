/*
 * Copyright 2013 Heena Mahour <heena393@gmail.com>
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


Item {
    id: dayStyle
    width: root.cellWidth
    height: root.cellHeight

    property real borderOpacity: daysCalendar.borderOpacity
    property bool today: isToday(new Date(yearNumber, monthNumber - 1, dayNumber))

    onHeightChanged: {
        // this is needed here as the text is first rendered, counting with the default root.cellHeight
        // then root.cellHeight actually changes to whatever it should be, but the Label does not pick
        // it up after that, so we need to change it explicitly after the cell size changes
        label.font.pixelSize = Math.max(theme.smallestFont.pixelSize, Math.floor(root.cellHeight / 3))
    }

    Rectangle {
        id: todayRect
        x: 0
        y: 0
        width: parent.width - (borderWidth)
        height: parent.height - (borderWidth)
        opacity: {
            if (calendarDays.selectedItem == dayStyle && today) {
                0.6
            } else if (today) {
                0.4
            } else {
                0
            }
        }
        Behavior on opacity { NumberAnimation { duration: units.shortDuration*2 } }
        color: theme.textColor
    }

    Rectangle {
        id: highlightDate
        anchors {
            fill: todayRect
        }
        opacity: {
            if (calendarDays.selectedItem == dayStyle) {
                0.6
            } else if (dateMouse.containsMouse) {
                0.4
            } else {
                0
            }
        }
        visible: !today
        Behavior on opacity { NumberAnimation { duration: units.shortDuration*2 } }
        color: theme.highlightColor
        z: todayRect.z - 1
    }

    Rectangle {
        id: frameRight
        width: borderWidth
        color: theme.textColor
        opacity: borderOpacity

        anchors {
            top: parent.top
            bottom: parent.bottom
            right: parent.right
        }
    }

    Rectangle {
        id: frameBottom
        height: borderWidth
        color: theme.textColor
        opacity: borderOpacity

        anchors {
            bottom: parent.bottom
            left: parent.left
            right: parent.right
        }
    }

    Components.Label {
        id: label
        anchors.centerIn: parent
        text: dayNumber
        opacity: (isPreviousMonth || isNextMonth) ? 0.5: 1.0
        color: today ? theme.backgroundColor : theme.textColor
    }



    MouseArea {
        id: dateMouse
        anchors.fill: parent
        //z: label.z + 1
        hoverEnabled: true
        onClicked: {
            print("date change:" + dayNumber + monthNumber)
            var rowNumber = Math.floor ( index / 7)   ;
            week = 1+monthCalendar.weeksModel[rowNumber];
            root.date = model
            calendarDays.selectedItem = dayStyle
        }
    }

    Component.onCompleted: {
        var t = isToday(new Date(yearNumber, monthNumber - 1, dayNumber))
        if (t) {
            root.date = model;
        }
    }
}
