/*
*   Copyright (C) 2011 by Daker Fernandes Pinheiro <dakerfp@gmail.com>
*   Copyright (C) 2011 Marco Martin <mart@kde.org>
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

import QtQuick 1.1
import org.kde.plasma.core 0.1 as PlasmaCore


PlasmaCore.FrameSvgItem {
    id: background
    anchors.fill: parent
    imagePath:"widgets/scrollbar"
    prefix: internalLoader.isVertical ? "background-vertical" : "background-horizontal"

    property int implicitWidth: scrollbarSvg.hasElement("hint-scrollbar-size") ? scrollbarSvg.elementSize("hint-scrollbar-size").width : 12
    property int implicitHeight: scrollbarSvg.hasElement("hint-scrollbar-size") ? scrollbarSvg.elementSize("hint-scrollbar-size").height : 12

    opacity: 0
    Behavior on opacity {
        NumberAnimation {
            duration: 250
            easing.type: Easing.OutQuad
        }
    }

    PlasmaCore.Svg {
        id: scrollbarSvg
        imagePath: "widgets/scrollbar"
    }

    property Item handle: handle

    property Item contents: contents
    Item {
        id: contents
        anchors.fill: parent

        PlasmaCore.FrameSvgItem {
            id: handle
            imagePath:"widgets/scrollbar"
            prefix: "slider"

            function length()
            {
                var nh, ny;

                if (internalLoader.isVertical) {
                    nh = flickableItem.visibleArea.heightRatio * internalLoader.height
                } else {
                    nh = flickableItem.visibleArea.widthRatio * internalLoader.width
                }

                if (internalLoader.isVertical) {
                    ny = flickableItem.visibleArea.yPosition * internalLoader.height
                } else {
                    ny = flickableItem.visibleArea.xPosition * internalLoader.width
                }

                if (ny > 3) {
                    var t

                    if (internalLoader.isVertical) {
                        t = Math.ceil(internalLoader.height - 3 - ny)
                    } else {
                        t = Math.ceil(internalLoader.width - 3 - ny)
                    }

                    if (nh > t) {
                        return t
                    } else {
                        return nh
                    }
                } else {
                    return nh + ny
                }
            }

            width: internalLoader.isVertical ? parent.width : length()
            height: internalLoader.isVertical ? length() : parent.height
        }
    }

    property MouseArea mouseArea: null

    Connections {
        target: flickableItem
        onMovingChanged: {
            if (flickableItem.moving) {
                opacityTimer.running = false
                background.opacity = 1
            } else {
                opacityTimer.restart()
            }
        }
    }
    Timer {
        id: opacityTimer
        interval: 500
        repeat: false
        running: false
        onTriggered: {
            background.opacity = 0
        }
    }
}

