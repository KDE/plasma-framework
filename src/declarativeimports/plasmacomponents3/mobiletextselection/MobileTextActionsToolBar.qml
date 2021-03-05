/*
    SPDX-FileCopyrightText: 2018 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

pragma Singleton

import QtQuick 2.1
import QtQuick.Layouts 1.2
import QtQuick.Window 2.2
import QtQuick.Controls @QQC2_VERSION@
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.kirigami 2.5 as Kirigami

Popup {
    id: root

    property Item controlRoot
    parent: controlRoot ? controlRoot.Window.contentItem : undefined
    modal: false
    focus: false
    closePolicy: Popup.NoAutoClose
    property bool shouldBeVisible: false

    x: parent ? Math.min(Math.max(0, controlRoot.mapToItem(root.parent, controlRoot.positionToRectangle(controlRoot.selectionStart).x, 0).x - root.width/2), parent.width - root.width) : 0

    y: {
        if (!parent) {
            return false;
        }
        var desiredY = controlRoot.mapToItem(root.parent, 0, controlRoot.positionToRectangle(controlRoot.selectionStart).y).y  - root.height;

        if (desiredY >= 0) {
            return Math.min(desiredY, parent.height - root.height);
        } else {
            return Math.min(Math.max(0, controlRoot.mapToItem(root.parent, 0, controlRoot.positionToRectangle(controlRoot.selectionEnd).y + Math.round(PlasmaCore.Units.gridUnit*1.5)).y), parent.height - root.height);
        }
    }


    visible: controlRoot ? shouldBeVisible && Kirigami.Settings.tabletMode && (controlRoot.selectedText.length > 0 || controlRoot.canPaste) : false

    width: contentItem.implicitWidth + leftPadding + rightPadding

    contentItem: RowLayout {
        ToolButton {
            focusPolicy: Qt.NoFocus
            icon.name: "edit-cut"
            visible: controlRoot && controlRoot.selectedText.length > 0 && (!controlRoot.hasOwnProperty("echoMode") || controlRoot.echoMode === TextInput.Normal)
            onClicked: {
                controlRoot.cut();
            }
        }
        ToolButton {
            focusPolicy: Qt.NoFocus
            icon.name: "edit-copy"
            visible: controlRoot && controlRoot.selectedText.length > 0 && (!controlRoot.hasOwnProperty("echoMode") || controlRoot.echoMode === TextInput.Normal)
            onClicked: {
                controlRoot.copy();
            }
        }
        ToolButton {
            focusPolicy: Qt.NoFocus
            icon.name: "edit-paste"
            visible: controlRoot && controlRoot.canPaste
            onClicked: {
                controlRoot.paste();
            }
        }
    }
}

