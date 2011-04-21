/*
*   Copyright (C) 2010 by Marco Martin <mart@kde.org>
*
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU Library General Public License as
*   published by the Free Software Foundation; either version 2, or
*   (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details
*
*   You should have received a copy of the GNU Library General Public
*   License along with this program; if not, write to the
*   Free Software Foundation, Inc.,
*   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

import Qt 4.7
import org.kde.plasma.core 0.1 as PlasmaCore

Item {
    width: 50
    height:20

    property string text
    property bool down: false
    signal clicked

    id: button

    PlasmaCore.FrameSvgItem {
        id: surface
        anchors.fill: parent
        imagePath: "widgets/button"
        prefix: down?"pressed":"normal"
        opacity: down?1:0
        Behavior on opacity {
            PropertyAnimation { duration: 250 }
        }
    }

    Text {
        text: button.text
        anchors.fill: parent
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
    }

    MouseArea {
        anchors.fill: parent
        hoverEnabled: true

        onPressed: {
            if (!down) {
                surface.prefix = "pressed"
            }
        }
        onReleased: {
            if (!down) {
                surface.prefix = "normal"
            }
            button.clicked()
        }
        onEntered: {
            if (!down) {
                surface.opacity = 1
            }
        }
        onExited: {
            if (!down) {
                surface.opacity = 0
            }
        }
    }

}

