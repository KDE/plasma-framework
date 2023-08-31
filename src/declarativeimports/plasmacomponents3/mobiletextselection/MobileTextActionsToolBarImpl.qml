/*
    SPDX-FileCopyrightText: 2018 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import org.kde.kirigami as Kirigami

Popup {
    id: root

    parent: controlRoot.Window.window.contentItem
    modal: false
    focus: false
    closePolicy: Popup.NoAutoClose

    x: parent ? Math.min(Math.max(0, controlRoot.mapToItem(root.parent, controlRoot.positionToRectangle(controlRoot.selectionStart).x, 0).x - root.width/2), parent.width - root.width) : 0

    y: {
        if (!parent) {
            return false;
        }
        var desiredY = controlRoot.mapToItem(root.parent, 0, controlRoot.positionToRectangle(controlRoot.selectionStart).y).y  - root.height;

        if (desiredY >= 0) {
            return Math.min(desiredY, parent.height - root.height);
        } else {
            return Math.min(Math.max(0, controlRoot.mapToItem(root.parent, 0, controlRoot.positionToRectangle(controlRoot.selectionEnd).y + Math.round(Kirigami.Units.gridUnit * 1.5)).y), parent.height - root.height);
        }
    }

    width: contentItem.implicitWidth + leftPadding + rightPadding
    visible: true

    contentItem: RowLayout {
        ToolButton {
            focusPolicy: Qt.NoFocus
            icon.name: "edit-cut"
            visible: controlRoot.selectedText.length > 0 && (!controlRoot.hasOwnProperty("echoMode") || controlRoot.echoMode === TextInput.Normal)
            onClicked: {
                controlRoot.cut();
            }
        }
        ToolButton {
            focusPolicy: Qt.NoFocus
            icon.name: "edit-copy"
            visible: controlRoot.selectedText.length > 0 && (!controlRoot.hasOwnProperty("echoMode") || controlRoot.echoMode === TextInput.Normal)
            onClicked: {
                controlRoot.copy();
            }
        }
        ToolButton {
            focusPolicy: Qt.NoFocus
            icon.name: "edit-paste"
            visible: controlRoot.canPaste
            onClicked: {
                controlRoot.paste();
            }
        }
    }
}
