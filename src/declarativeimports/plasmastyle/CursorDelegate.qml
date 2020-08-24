/*
    SPDX-FileCopyrightText: 2015 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.2
import org.kde.plasma.core 2.0 as PlasmaCore

Item {
    id: root

    property Item input: parent

    width: units.smallSpacing
    height: input.cursorRectangle.height
    visible: input.activeFocus && input.selectionStart === input.selectionEnd


    Rectangle {
        width: units.smallSpacing
        height: parent.height + units.gridUnit
        radius: width
        color: theme.highlightColor
    }

    Rectangle {
        id: handle
        x: -width/2 + parent.width/2
        width: units.gridUnit
        height: width
        radius: width
        color: theme.highlightColor
        anchors.top: parent.bottom
    }
    MouseArea {
        drag {
            target: root
            minimumX: 0
            minimumY: 0
            maximumX: input.width
            maximumY: input.height - root.height
        }
        width: handle.width * 2
        height: parent.height + handle.height
        x: -width/2
        onReleased: {
            var pos = mapToItem(input, mouse.x, mouse.y);
            input.cursorPosition = input.positionAt(pos.x, pos.y);
        }
    }
}

