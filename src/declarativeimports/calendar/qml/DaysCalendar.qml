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

    readonly property int gridColumns: root.showWeekNumbers ? calendarGrid.columns + 1 : calendarGrid.columns

    // This is to ensure that the inner grid.width is always aligned to be divisible by 7,
    // fixes wrong side margins because of the rounding of cell size
    // (consider the parent.width to be 404, the cell width would be 56,
    // but 56*7 + 6 (the inner spacing) is 398, so we split the remaining 6 to avoid
    // wrong alignment)
    anchors {
        leftMargin: Math.floor(((parent.width - (gridColumns + 1) * borderWidth) % gridColumns) / 2)
        rightMargin: anchors.leftMargin
        bottomMargin: anchors.leftMargin
    }

    // Paints the inner grid and the outer frame
    Canvas {
        id: canvas
        anchors.fill: parent
        opacity: root.borderOpacity
        antialiasing: false
        clip: false
        onPaint: {
            var ctx = getContext("2d");
            // this is needed as otherwise the canvas seems to have some sort of
            // inner clip region which does not update on size changes
            ctx.reset();
            ctx.save();
            ctx.clearRect(0, 0, canvas.width, canvas.height);
            ctx.strokeStyle = theme.textColor;
            ctx.lineWidth = root.borderWidth
            ctx.globalAlpha = 1.0;

            ctx.beginPath();

            // This isn't the real width/height, but rather the X coord where the line will stop
            // and as the coord system starts from (0,0), we need to do "-1" to not get off-by-1 errors
            var rectWidth = (root.cellWidth + root.borderWidth) * gridColumns + root.borderWidth - 1
            var rectHeight = (root.cellHeight + root.borderWidth) * calendarGrid.rows + root.borderWidth - 1

            // the outer frame
            ctx.strokeRect(0, 0, rectWidth, rectHeight);

            // horizontal lines
            for (var i = 0; i < calendarGrid.rows - 1; i++) {
                var lineY = (rectHeight / calendarGrid.rows) * (i + 1);

                ctx.moveTo(root.showWeekNumbers ? root.cellWidth + root.borderWidth : root.borderWidth, lineY);
                ctx.lineTo(rectWidth, lineY);
            }

            // vertical lines
            for (var i = 0; i < gridColumns - 1; i++) {
                var lineX = (rectWidth / gridColumns) * (i + 1);

                ctx.moveTo(lineX, root.borderWidth + root.cellHeight);
                ctx.lineTo(lineX, rectHeight);
            }

            ctx.closePath();
            ctx.stroke();
            ctx.restore();
        }
    }

    Column {
        id: weeksColumn
        visible: root.showWeekNumbers
        anchors {
            top: canvas.top
            left: parent.left
            bottom: canvas.bottom
            // The borderWidth needs to be counted twice here because it goes
            // in fact through two lines - the topmost one (the outer edge)
            // and then the one below weekday strings
            topMargin: root.cellHeight + root.borderWidth + root.borderWidth
        }
        spacing: root.borderWidth

        Repeater {
            model: root.showWeekNumbers ? calendarBackend.weeksModel : []

            Components.Label {
                height: root.cellHeight
                width: root.cellWidth
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                opacity: 0.4
                text: modelData
                font.pixelSize: Math.max(theme.smallestFont.pixelSize, root.cellHeight / 6)
            }
        }
    }

    Grid {
        id: calendarGrid
        // Pad the grid to not overlap with the top and left frame
        // When week numbers are shown, the border needs to be counted twice
        // because there's one more cell to count with and therefore also
        // another border to add
        x: root.showWeekNumbers ? 2 * root.borderWidth + root.cellWidth: root.borderWidth
        y: root.borderWidth
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

            DayDelegate {}
        }
    }
}
