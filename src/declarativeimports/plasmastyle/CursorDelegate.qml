/*
    SPDX-FileCopyrightText: 2015 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.2
import org.kde.plasma.core 2.0 as PlasmaCore

/**
 * \internal
 */
Item {
    id: root

    property Item input: parent

    width: PlasmaCore.Units.smallSpacing
    height: input.cursorRectangle.height
    visible: input.activeFocus && input.selectionStart === input.selectionEnd


    Rectangle {
        width: PlasmaCore.Units.smallSpacing
        height: parent.height + PlasmaCore.Units.gridUnit
        radius: width
        color: PlasmaCore.Theme.highlightColor
    }

    Rectangle {
        id: handle
        x: -width/2 + parent.width/2
        width: PlasmaCore.Units.gridUnit
        height: width
        radius: width
        color: PlasmaCore.Theme.highlightColor
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

