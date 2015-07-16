/*
 *   Copyright (C) 2015 by Marco Martin <mart@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Library General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  2.010-1301, USA.
 */

import QtQuick 2.1
import QtQuick.Controls 1.1
import QtQuick.Controls.Styles 1.1
import QtQuick.Controls.Private 1.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents

Item {
    anchors.fill: parent

    property Component defaultMenu: PlasmaCore.FrameSvgItem {
        id: popup
        imagePath: "widgets/background"
        visible: false
        width: childrenRect.width + margins.left + margins.right
        height: childrenRect.height + margins.top + margins.bottom
        z: 9999
        Component.onCompleted: {
            var par = popup.parent
            while (par) {
                popup.parent = par
                par = parent.parent
            }
        }

        function popup(pos) {
            popup.x = pos.x;
            popup.y = pos.y;
            popup.visible = true;
            popup.z = 9999
        }
        function dismiss() {
            popup.visible = false;
            input.z = 0
        }
        Row {
            x: parent.margins.left
            y: parent.margins.top
            property Item checkedButton
            PlasmaComponents.ToolButton {
                iconSource: "edit-cut"
                flat: false
                onClicked: {
                    cut();
                    select(input.cursorPosition, input.cursorPosition);
                }
            }
            PlasmaComponents.ToolButton {
                iconSource: "edit-copy"
                flat: false
                onClicked: {
                    copy();
                    select(input.cursorPosition, input.cursorPosition);
                }
            }
            PlasmaComponents.ToolButton {
                iconSource: "edit-paste"
                flat: false
                onClicked: {
                    paste();
                }
            }
        }
    }

    Connections {
        target: mouseArea

        onClicked: {
            if (control.menu && getMenuInstance().__popupVisible) {
                select(input.cursorPosition, input.cursorPosition);
            } else {
                input.activate();
            }

            if (input.activeFocus) {
                var pos = input.positionAt(mouse.x, mouse.y)
                input.moveHandles(pos, pos)
            }
            popupTimer.restart();
        }
        onPressAndHold: {
            var pos = input.positionAt(mouseArea.mouseX, mouseArea.mouseY);
            input.select(pos, pos);
            var hasSelection = selectionStart != selectionEnd;
            selectWord();
            popupTimer.restart();
        }
    }

    Connections {
        target: input
        onSelectionStartChanged: popupTimer.restart()
        onSelectionEndChanged: popupTimer.restart()
        onActiveFocusChanged: popupTimer.restart()
    }

    Timer {
        id: popupTimer
        interval: 1
        onTriggered: {
            if (selectionStart !== selectionEnd && control.activeFocus) {
                var startRect = input.positionToRectangle(input.selectionStart);
                var endRect = input.positionToRectangle(input.selectionEnd);

                var pos = getMenuInstance().mapFromItem(input, startRect.x, startRect.y + units.gridUnit);
                getMenuInstance().dismiss();
                getMenuInstance().popup(pos);
            } else {
                getMenuInstance().dismiss();
            }
        }
    }
}
