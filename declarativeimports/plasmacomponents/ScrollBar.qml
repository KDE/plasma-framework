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

    
PlasmaCore.FrameSvgItem {
    id: scrollBar
    width: orientation==Qt.Horizontal?200:22
    height: orientation==Qt.Horizontal?22:200

    property int minimum: 0
    property int maximum: 100
    property int value: 0
    property string orientation: Qt.Horizontal

    onValueChanged: {
        if (drag.state != "dragging") {
            if (orientation == Qt.Horizontal) {
                drag.x = (value/(maximum-minimum))*(scrollBar.width - drag.width)
            } else {
                drag.y = (value/(maximum-minimum))*(scrollBar.height - drag.height)
            }
        }
    }

    imagePath: "widgets/scrollbar"
    prefix: orientation==Qt.Horizontal?"background-horizontal":"background-vertical"

    PlasmaCore.FrameSvgItem {
        id: drag
        anchors.top: orientation==Qt.Horizontal?parent.top:null
        anchors.bottom: orientation==Qt.Horizontal?parent.bottom:null
        anchors.left: orientation==Qt.Horizontal?null:parent.left
        anchors.right: orientation==Qt.Horizontal?null:parent.right
        state: "normal"
        width: (orientation == Qt.Horizontal)?Math.max(12, (parent.width*1/(scrollBar.maximum-scrollBar.minimum))):0
        height: (orientation != Qt.Horizontal)?Math.max(12, (parent.height*1/(scrollBar.maximum-scrollBar.minimum))):0
        x: 0
        y: 0
        onXChanged: {
            if (orientation == Qt.Horizontal && state == "dragging") {
                value = (maximum - minimum)*(x/(scrollBar.width-width))
            }
        }
        onYChanged: {
            if (orientation != Qt.Horizontal && state == "dragging") {
                value = (maximum - minimum)*(x/(scrollBar.height-height))
            }
        }
        
        Behavior on x {
            NumberAnimation {
                duration: 200
            }
        }
        Behavior on y {
            NumberAnimation {
                duration: 200
            }
        }
        
        imagePath: "widgets/scrollbar"
        prefix: "slider"
        MouseArea {
            anchors.fill: parent
            hoverEnabled: true
            
            drag.target: parent;
            drag.axis: orientation == Qt.Horizontal?"XAxis":"YAxis"
            drag.minimumX: 0;
            drag.maximumX: scrollBar.width-drag.width;
            drag.minimumY: 0;
            drag.maximumY: scrollBar.height-drag.height;
            
            onEntered: drag.prefix = "mouseover-slider"
            onExited: drag.prefix = "slider"
            onPressed: {
                drag.prefix = "sunken-slider"
                drag.state = "dragging"
            }
            onReleased: {
                containsMouse?drag.prefix = "mouseover-slider":drag.prefix = "slider"
                drag.state = "normal"
            }
        }
    }
}


