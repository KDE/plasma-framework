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

    // Paints the inner grid and the outer frame
    Canvas {
        id: canvas

        width: (root.cellWidth + root.borderWidth) * gridColumns + root.borderWidth
        height: (root.cellHeight + root.borderWidth) * calendarGrid.rows + root.borderWidth
        anchors.bottom: parent.bottom

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

            // When line is more wide than 1px, it is painted with 1px line at the actual coords
            // and then 1px lines are added first to the left of the middle then right (then left again)
            // So all the lines need to be offset a bit to have their middle point in the center
            // of the grid spacing rather than on the left most pixel, otherwise they will be painted
            // over the days grid which will be visible on eg. mouse hover
            var lineBasePoint = Math.floor(root.borderWidth / 2)

            // horizontal lines
            for (var i = 0; i < calendarGrid.rows + 1; i++) {
                var lineY = lineBasePoint + (root.cellHeight + root.borderWidth) * (i);

                if (i == 0 || i == calendarGrid.rows) {
                    ctx.moveTo(0, lineY);
                } else {
                    ctx.moveTo(root.showWeekNumbers ? root.cellWidth + root.borderWidth : root.borderWidth, lineY);
                }
                ctx.lineTo(width, lineY);
            }

            // vertical lines
            for (var i = 0; i < gridColumns + 1; i++) {
                var lineX = lineBasePoint + (root.cellWidth + root.borderWidth) * (i);

                // Draw the outer vertical lines in full height so that it closes
                // the outer rectangle
                if (i == 0 || i == gridColumns) {
                    ctx.moveTo(lineX, 0);
                } else {
                    ctx.moveTo(lineX, root.borderWidth + root.cellHeight);
                }
                ctx.lineTo(lineX, height);
            }

            ctx.closePath();
            ctx.stroke();
            ctx.restore();
        }
    }

    Connections {
        target: root
        onShowWeekNumbersChanged: {
            canvas.requestPaint();
        }
    }

    Connections {
        target: theme
        onTextColorChanged: {
            canvas.requestPaint();
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

        anchors {
            bottom: parent.bottom
            bottomMargin: root.borderWidth
        }

        columns: calendarBackend.days
        rows: calendarBackend.weeks + 1
        spacing: root.borderWidth
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
                    anchors.bottomMargin: units.smallSpacing
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
