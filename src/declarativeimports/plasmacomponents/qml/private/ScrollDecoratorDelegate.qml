/*
    SPDX-FileCopyrightText: 2011 Daker Fernandes Pinheiro <dakerfp@gmail.com>
    SPDX-FileCopyrightText: 2011 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.1
import org.kde.plasma.core 2.0 as PlasmaCore


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
            duration: PlasmaCore.Units.longDuration
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
        function onMovingChanged() {
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

