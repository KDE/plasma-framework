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
    property real borderOpacity: 0.2

    Rectangle {
        id: frameTop
        height: borderWidth
        width: root.columns * root.cellWidth
        color: theme.textColor
        opacity: borderOpacity
//         visible: showtop

        anchors {
            top: parent.top
            left: calendarDays.left
            //right: calendarDays.right
        }
    }

    Rectangle {
        id: frameLeft
        width: borderWidth
        height: root.cellHeight * root.columns
        color: theme.textColor
        opacity: borderOpacity
        //         visible: showtop

        anchors {
            right: calendarDays.left
            top: calendarDays.top
            //bottom: calendarDays.bottom
        }
    }

    Rectangle {
        id: frameRight
        width: borderWidth
        height: root.cellHeight
        color: theme.textColor
        opacity: borderOpacity
        //         visible: showtop

        anchors {
            right: frameTop.right
            top: calendarDays.top
            //bottom: calendarDays.bottom
        }
    }

    Rectangle {
        id: frameSecond
        height: borderWidth
        color: theme.textColor
        opacity: borderOpacity
// //         visible: /*showtop*/
        y: cellHeight - borderWidth
        anchors {
            //top: parent.top
            left: calendarDays.left
            right: frameTop.right
        }
    }


    Grid {
        id: calendarDays
        anchors.fill: parent
        columns: monthCalendar.days
        rows: 1 + monthCalendar.weeks
        spacing: 0
        property Item selectedItem
        property bool containsEventItems: false // FIXME
        property bool containsTodoItems: false // FIXME

        Repeater {
            id: days
            model: monthCalendar.days
            Item {
                width: cellWidth
                height: cellHeight
                Components.Label {
                    text: Qt.formatDate(new Date(showDate.getFullYear(), showDate.getMonth(), index - firstDay +1), "ddd");
                    font.pixelSize: Math.max(theme.smallestFont.pixelSize, root.cellHeight / 6)
                    //font: theme.smallestFont
                    opacity: 0.2
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignBottom
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.bottom: parent.bottom
                    anchors.bottomMargin: borderWidth * 2
                }
            }
        }

        Repeater {
            id: repeater
            model: monthCalendar.daysModel

            DayDelegate {}
        }
    }
}