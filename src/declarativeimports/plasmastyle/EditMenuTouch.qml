/*
    SPDX-FileCopyrightText: 2015 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.1
import QtQuick.Controls 1.1
import QtQuick.Controls.Styles 1.1
import QtQuick.Controls.Private 1.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents

/**
 * \internal
 */
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
            var par = control
            //heuristic: if a flickable is found in the parents,
            //reparent to it, so it scrolls together
            while (par.parent && par.parent.contentY === undefined) {
                par = par.parent
            }

            popup.parent = par
        }

        function popup(pos) {
            popup.x = pos.x;
            popup.y = pos.y;
            popup.visible = true;
        }
        function dismiss() {
            popup.visible = false;
        }
        Row {
            x: parent.margins.left
            y: parent.margins.top
            property Item checkedButton
            PlasmaComponents.ToolButton {
                iconSource: "text-field"
                flat: false
                visible: input.selectedText == "" && control.echoMode != TextInput.Password
                onClicked: {
                    selectWord();
                    popupTimer.restart();
                }
            }
            PlasmaComponents.ToolButton {
                iconSource: "edit-cut"
                flat: false
                visible: input.selectedText != "" && control.echoMode != TextInput.Password
                onClicked: {
                    control.cut();
                    select(input.cursorPosition, input.cursorPosition);
                }
            }
            PlasmaComponents.ToolButton {
                iconSource: "edit-copy"
                visible: input.selectedText != "" && control.echoMode != TextInput.Password
                flat: false
                onClicked: {
                    control.copy();
                    select(input.cursorPosition, input.cursorPosition);
                }
            }
            PlasmaComponents.ToolButton {
                iconSource: "edit-paste"
                visible: input.canPaste
                flat: false
                onClicked: {
                    control.paste();
                }
            }
        }
    }

    Connections {
        target: mouseArea

        function onClicked(mouse) {
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
        function onPressAndHold(mouse) {
            input.activate();
            var pos = input.positionAt(mouseArea.mouseX, mouseArea.mouseY);
            input.select(pos, pos);
            selectWord();
            popupTimer.restart();
        }
    }

    Connections {
        target: input
        function onSelectionStartChanged() {popupTimer.restart()}
        function onSelectionEndChanged() {popupTimer.restart()}
        function onActiveFocusChanged() {popupTimer.restart()}
    }

    Connections {
        target: flickable
        function onMovingChanged() {popupTimer.restart()}
    }

    Timer {
        id: popupTimer
        interval: 1
        onTriggered: {
            if (control.activeFocus) {
                var startRect = input.positionToRectangle(input.selectionStart);
                var endRect = input.positionToRectangle(input.selectionEnd);

                var pos = getMenuInstance().parent.mapFromItem(input, (startRect.x + endRect.x)/2 - getMenuInstance().width/2, endRect.y);
                pos.y += (pos.y + getMenuInstance().height + PlasmaCore.Units.gridUnit) > getMenuInstance().parent.height ? -PlasmaCore.Units.smallSpacing - getMenuInstance().height : PlasmaCore.Units.gridUnit*2;

                getMenuInstance().dismiss();
                getMenuInstance().popup(pos);
            } else {
                getMenuInstance().dismiss();
            }
        }
    }
}
