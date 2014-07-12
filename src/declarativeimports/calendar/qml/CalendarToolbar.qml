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

Row {
    id: calendarOperations
    anchors {
        left: parent.left
        top: parent.top
        right: parent.right
    }
    spacing: 4
    visible: false
    
    Components.ToolButton {
        id: monthright
        flat: true;
        text: "<";
        width: height;
        anchors.left: parent.left
        anchors.rightMargin: 20
        onClicked: {
            calendarBackend.previousMonth()
        }
    }

//     Components.ToolButton {
//         id: month
//         anchors.left: monthright.right
//         anchors.right: monthYear.left
//         anchors.leftMargin: 20
//         Loader {
//             id: menuLoader
//         }
//         onClicked: {
//             if (menuLoader.source == "") {
//                 menuLoader.source = "MonthMenu.qml"
//             } else {
//                 //menuLoader.source = ""
//             }
//             menuLoader.item.open(0, height);
//         }
//         text: calendarBackend.monthName
//     }

    Components.ToolButton {
        id: monthYear
        text: calendarBackend.year
        anchors.leftMargin: 20
        anchors.left: month.right
        Components.ToolButton {
            id: increase
            text: "^"
            width: 12
            height: 12
            anchors.left: monthYear.right
            onClicked: calendarBackend.nextYear()
        }
        Components.ToolButton {
            id: decrease
            text: "v"
            width: 12
            height: 12
            anchors.left: monthYear.right
            anchors.top: increase.bottom
            onClicked: calendarBackend.previousYear()
        }
    }

    Components.ToolButton {
        id: previous
        flat: true;
        text: ">";
        width: height;
        anchors.right: parent.right
        onClicked: {
            calendarBackend.nextMonth()
        }
    }
}