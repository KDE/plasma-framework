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
import org.kde.plasma.extras 2.0 as PlasmaExtras


DayStyle {
    id: myRectangle
    width: cellWidth
    height: cellHeight

    Components.Label {
        id: label
        anchors.centerIn: parent
        //anchors.topMargin: 10
        font.pixelSize: Math.max(theme.defaultFont.pixelSize, cellHeight / 3)
        text: dayNumber
        z: 99
        font.bold: (containsEventItems)||(containsTodoItems) ? true: false
        opacity: (isPreviousMonth || isNextMonth) ? 0.5: 1.0
    }
    MouseArea {
        id: dateMouse
        anchors.fill: parent
        z: label.z + 1
        hoverEnabled: true
        onClicked: {
            var rowNumber = Math.floor ( index / 7)   ;
            week = 1+monthCalendar.weeksModel[rowNumber];
            date = dayNumber + "/" + monthNumber + "/" + yearNumber
            calendarDays.selectedItem = myRectangle
        }
    }
}
