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
    id: daysCalendar

    // This is to ensure that the inner grid.width is always aligned to be divisible by 7,
    // fixes wrong side margins because of the rounding of cell size
    // (consider the parent.width to be 404, the cell width would be 56,
    // but 56*7 + 6 (the inner spacing) is 398, so we split the remaining 6 to avoid
    // wrong alignment)
    anchors {
        leftMargin: Math.floor(((parent.width + borderWidth) % 7) / 2);
        rightMargin: anchors.leftMargin
        bottomMargin: anchors.leftMargin
    }

    property real borderOpacity: 1.0

    Rectangle {
        id: allAroundFrame
        color: "transparent"
        border.width: borderWidth
        border.color: theme.textColor
        opacity: borderOpacity
        width: (root.cellWidth + borderWidth) * calendarGrid.rows
        height: (root.cellHeight + borderWidth) * calendarGrid.columns
    }

    Repeater {
        id: verticalGridLineRepeater
        model: calendarGrid.columns - 1
        delegate: Rectangle {
            x: root.cellWidth + (index * (root.cellWidth + borderWidth))
            // The first grid row does not have any columns, so start at cellHeight and add borderWidth
            // to not create conjuction points which looks bad as the lines are semi-transparent
            y: root.cellHeight + borderWidth
            width: borderWidth
            // Subtract the most bottom border width to avoid conjuction points
            height: (root.cellHeight + borderWidth) * (calendarGrid.columns - 1) - borderWidth
            color: theme.textColor
            opacity: borderOpacity
        }
    }

    Repeater {
        id: horizontalGridLineRepeater
        model: calendarGrid.rows - 1
        delegate: Rectangle {
            // Start the horizontal line so that it does not cross the leftmost vertical borderline
            // but is rathar adjacent to it
            x: borderWidth
            y: root.cellHeight + (index * (root.cellHeight + borderWidth))
            // To each cell add one border width and then subtract the outer border widths
            // so the lines do not cross
            width: (root.cellWidth + borderWidth) * calendarGrid.rows - (borderWidth * 2)
            height: borderWidth
            color: theme.textColor
            opacity: borderOpacity
        }
    }

    Grid {
        id: calendarGrid
        columns: calendarBackend.days
        rows: calendarBackend.weeks + 1
        spacing: 1
        property Item selectedItem
        property bool containsEventItems: false // FIXME
        property bool containsTodoItems: false // FIXME

        property QtObject selectedDate: root.date
        onSelectedDateChanged: {
            // clear the selection if the root.date is null
            if (calendarGrid.selectedDate == null) {
                calendarGrid.selectedItem = null;
            }
        }

        Repeater {
            id: days
            model: calendarBackend.days
            Item {
                width: root.cellWidth
                height: root.cellHeight
                Components.Label {
                    text: Qt.locale().dayName(calendarBackend.firstDayOfWeek + index, Locale.ShortFormat)
                    font.pixelSize: Math.max(theme.smallestFont.pixelSize, root.cellHeight / 6)
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
            model: calendarBackend.daysModel

            DayDelegate {
                borderOpacity: daysCalendar.borderOpacity
            }
        }
    }
}