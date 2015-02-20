/*
 * Copyright 2013  Heena Mahour <heena393@gmail.com>
 * Copyright 2013 Sebastian Kügler <sebas@kde.org>
 * Copyright 2015 Kai Uwe Broulik <kde@privat.broulik.de>
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

FocusScope {
    id: daysCalendar

    signal headerClicked
    signal previous
    signal next

    signal activated(int index, var date, var item)
    signal selected(int index, var date, var item)

    // whether to show prev/next buttons
    property bool headerVisible: true

    property int rows
    property int columns

    // how precise date matching should be, 3 = day+month+year, 2 = month+year, 1 = just year
    property int precision

    property alias headerModel: days.model
    property alias gridModel: repeater.model

    property alias title: heading.text

    function ensureSelectionVisible() {
        // HACK check whether the selection is on the current screen
        var selectionFound = false
        var calendarItems = calendarGrid.children
        for (var i = days.count + 1, j = calendarItems.length; i < j; ++i) {
            var item = calendarItems[i]
            if (item.isSelected) {
                selectionFound = true
                break
            }
        }

        if (!selectionFound) {
            calendarGrid.children[days.count + 1 + (gridModel.daysBeforeCurrent || 0)].selected()
        }
    }

    Keys.onPressed: {
        if (event.key === Qt.Key_Backspace) {
            daysCalendar.headerClicked()
            event.accepted = true
        } else if (event.key === Qt.Key_PageUp ||
                   (event.modifiers & Qt.ShiftModifier && (event.key === Qt.Key_Up || event.key === Qt.Key_Left))) {
            daysCalendar.previous()
            event.accepted = true
        } else if (event.key === Qt.Key_PageDown ||
                   (event.modifiers & Qt.ShiftModifier && (event.key === Qt.Key_Down || event.key === Qt.Key_Right))) {
            daysCalendar.next()
            event.accepted = true
        } else if (event.key === Qt.Key_Up) {
            ensureSelectionVisible()
        } else if (event.key === Qt.Key_Down) {
            ensureSelectionVisible()
        } else if (event.key === Qt.Key_Left) {
            ensureSelectionVisible()
        } else if (event.key === Qt.Key_Right) {
            ensureSelectionVisible()
        } else if (event.key === Qt.Key_Return) {
            var calendarItems = calendarGrid.children
            for (var i = days.count + 1, j = calendarItems.length; i < j; ++i) {
                var item = calendarItems[i]
                if (item.isSelected) {
                    item.activated()
                    break
                }
            }
        }

        console.log("KALENDAR", event.key)
    }

    readonly property int cellWidth: {
        return Math.min(
            Math.max(
                mWidth * 3,
                // Take the calendar width, subtract the inner and outer spacings and divide by number of columns (==days in week)
                Math.floor((canvas.width - (daysCalendar.columns + 1) * root.borderWidth) / daysCalendar.columns)
            ),
            mWidth * 100
        )
    }
    readonly property int cellHeight: {
        return Math.min(
            Math.max(
                mHeight * 1.5,
                // Take the calendar height, subtract the inner spacings and divide by number of rows (root.weeks + one row for day names)
                Math.floor((canvas.height - (daysCalendar.rows + 1) * root.borderWidth) / (daysCalendar.rows + 1))
            ),
            mHeight * 40
        )
    }

    PlasmaExtras.Heading {
        id: heading

        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
        }

        focus: true
        level: 1
        elide: Text.ElideRight
        font.capitalization: Font.Capitalize
        opacity: activeFocusChanged ? 1 : 0.8 // 0.8 is default opacity for Heading item
        Behavior on opacity { NumberAnimation { duration: units.shortDuration } }

        MouseArea {
            id: monthMouse
            property int previousPixelDelta

            width: heading.paintedWidth
            anchors {
                left: parent.left
                top: parent.top
                bottom: parent.bottom
            }
            onClicked: {
                if (!stack.busy) {
                    daysCalendar.headerClicked()
                }
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
                    daysCalendar.previous()
                } else if (delta <= -15) {
                    daysCalendar.next()
                }
                previousPixelDelta = 0
            }
        }
    }

    FocusScope {
        id: previousButton
        anchors {
            top: heading.bottom
            left: parent.left
            leftMargin: Math.floor(units.largeSpacing / 2)
        }
        KeyNavigation.tab: nextButton

        Components.Label {
            text: Qt.application.layoutDirection === Qt.RightToLeft ? "▶" : "◀"
            opacity: leftmouse.containsMouse || activeFocus ? 1 : 0.4
            focus: true
            Behavior on opacity { NumberAnimation { duration: units.shortDuration } }

            MouseArea {
                id: leftmouse
                anchors.fill: parent
                anchors.margins: -units.largeSpacing / 3
                hoverEnabled: true
                onClicked: daysCalendar.previous()
            }
        }
    }

    Components.Label {
        id: nextButton
        text: Qt.application.layoutDirection === Qt.RightToLeft ? "◀" : "▶"
        focus: true
        opacity: rightmouse.containsMouse || activeFocus ? 1 : 0.4
        Behavior on opacity { NumberAnimation { duration: units.shortDuration } }
        anchors {
            top: heading.bottom
            right: parent.right
            rightMargin: Math.floor(units.largeSpacing / 2)
        }
        MouseArea {
            id: rightmouse
            anchors.fill: parent
            anchors.margins: -units.largeSpacing / 3
            hoverEnabled: true
            onClicked: daysCalendar.next()
        }
    }

    // Paints the inner grid and the outer frame
    Canvas {
        id: canvas
        anchors {
            top: heading.bottom
            left: parent.left
            right: parent.right
            bottom: parent.bottom
        }
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
            var rectWidth = (daysCalendar.cellWidth + root.borderWidth) * calendarGrid.columns + root.borderWidth - 1
            var rectHeight = (daysCalendar.cellHeight + root.borderWidth) * calendarGrid.rows + root.borderWidth - 1

            // the outer frame
            ctx.strokeRect(0, 0, rectWidth, rectHeight);

            // horizontal lines
            for (var i = 1; i < calendarGrid.rows; ++i) {
                var lineY = (rectHeight / calendarGrid.rows) * i;

                ctx.moveTo(0, lineY);
                ctx.lineTo(rectWidth, lineY);
            }

            // vertical lines
            for (var i = 1; i < calendarGrid.columns; ++i) {
                var lineX = (rectWidth / calendarGrid.columns) * i;

                ctx.moveTo(lineX, root.borderWidth + daysCalendar.cellHeight);
                ctx.lineTo(lineX, rectHeight);
            }

            ctx.closePath();
            ctx.stroke();
            ctx.restore();
        }
    }

    Grid {
        id: calendarGrid
        focus: true
        // Pad the grid to not overlap with the top and left frame
        x: root.borderWidth
        y: heading.height + root.borderWidth + (!days.model ? daysCalendar.cellHeight + root.borderWidth : 0)
        columns: daysCalendar.columns
        rows: daysCalendar.rows + 1
        spacing: 1
        property bool containsEventItems: false // FIXME
        property bool containsTodoItems: false // FIXME

        Keys.onPressed: {
            console.log("wefhiwefhiwe", event.key)
        }

        Repeater {
            id: days
            Item {
                width: daysCalendar.cellWidth
                height: daysCalendar.cellHeight
                Components.Label {
                    text: Qt.locale().dayName(calendarBackend.firstDayOfWeek + index, Locale.ShortFormat)
                    font.pixelSize: Math.max(theme.smallestFont.pixelSize, daysCalendar.cellHeight / 6)
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

            DayDelegate {
                id: delegate
                width: daysCalendar.cellWidth
                height: daysCalendar.cellHeight

                onActivated: daysCalendar.activated(index, model, delegate)
                onSelected: daysCalendar.selected(index, model, delegate)
            }
        }
    }
}
