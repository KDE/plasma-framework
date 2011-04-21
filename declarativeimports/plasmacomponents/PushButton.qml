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
    signal clicked

    id: button
    PlasmaCore.FrameSvgItem {
        id: hover
        anchors.fill: parent
        opacity: 0
        anchors.leftMargin: -margins.left
        anchors.topMargin: -margins.top
        anchors.rightMargin: -margins.right
        anchors.bottomMargin: -margins.bottom
        imagePath: "widgets/button"
        prefix: "hover"
        Behavior on opacity {
            PropertyAnimation { duration: 250 }
        }
    }
    PlasmaCore.FrameSvgItem {
        id: shadow
        anchors.fill: parent
        anchors.leftMargin: -margins.left
        anchors.topMargin: -margins.top
        anchors.rightMargin: -margins.right
        anchors.bottomMargin: -margins.bottom
        imagePath: "widgets/button"
        prefix: "shadow"
        Behavior on opacity {
            PropertyAnimation { duration: 250 }
        }
    }
        
    PlasmaCore.FrameSvgItem {
        id: surface
        anchors.fill: parent
        imagePath: "widgets/button"
        prefix: "normal"   
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
            surface.prefix = "pressed"
        }
        onReleased: {
            surface.prefix = "normal"
            button.clicked()
        }
        onEntered: {
            shadow.opacity = 0
            hover.opacity = 1
        }
        onExited: {
            shadow.opacity = 1
            hover.opacity = 0
        }
    }

}

