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

