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

    property int implicitWidth: (scrollbarSvg.hasElement("hint-scrollbar-size") ? scrollbarSvg.elementSize("hint-scrollbar-size").width : scrollbarSvg.elementSize("arrow-up").width)
    property int implicitHeight: (scrollbarSvg.hasElement("hint-scrollbar-size") ? scrollbarSvg.elementSize("hint-scrollbar-size").height : scrollbarSvg.elementSize("arrow-left").height)

     Keys.onUpPressed: {
        if (!enabled || !internalLoader.isVertical)
            return;

        if (inverted)
            internalLoader.incrementValue(stepSize);
        else
            internalLoader.incrementValue(-stepSize);
    }

    Keys.onDownPressed: {
        if (!enabled || !internalLoader.isVertical)
            return;

        if (inverted)
            internalLoader.incrementValue(-stepSize);
        else
            internalLoader.incrementValue(stepSize);
    }

    Keys.onLeftPressed: {
        if (!enabled || internalLoader.isVertical)
            return;

        if (inverted)
            internalLoader.incrementValue(stepSize);
        else
            internalLoader.incrementValue(-stepSize);
    }

    Keys.onRightPressed: {
        if (!enabled || internalLoader.isVertical)
            return;

        if (inverted)
            internalLoader.incrementValue(-stepSize);
        else
            internalLoader.incrementValue(stepSize);
    }

    property Item handle: handle

    property Item contents: contents
    Item {
        id: contents
        anchors {
            fill: parent
            leftMargin: (!internalLoader.isVertical && leftButton.visible) ? leftButton.width : 0
            rightMargin: (!internalLoader.isVertical && rightButton.visible) ? rightButton.width : 0
            topMargin: (internalLoader.isVertical && leftButton.visible) ? leftButton.height : 0
            bottomMargin: (internalLoader.isVertical && rightButton.visible) ? rightButton.height : 0
        }

        PlasmaCore.FrameSvgItem {
            id: handle
            imagePath:"widgets/scrollbar"
            prefix: {
                if (mouseArea.pressed) {
                    return "sunken-slider"
                }

                if (scrollbar.activeFocus || mouseArea.containsMouse) {
                    return "mouseover-slider"
                } else {
                    return "slider"
                }
            }

            property int length: internalLoader.isVertical? flickableItem.visibleArea.heightRatio * parent.height :  flickableItem.visibleArea.widthRatio * parent.width

            width: internalLoader.isVertical ? parent.width : length
            height: internalLoader.isVertical ? length : parent.height
        }
    }

    PlasmaCore.Svg {
        id: scrollbarSvg
        imagePath: "widgets/scrollbar"
        property bool arrowPresent: scrollbarSvg.hasElement("arrow-up")
        //new theme may be different
        onRepaintNeeded: arrowPresent = scrollbarSvg.hasElement("arrow-up")
    }

    PlasmaCore.SvgItem {
        id: leftButton
        visible: stepSize > 0 && scrollbarSvg.arrowPresent

        anchors {
            left: internalLoader.isVertical ? undefined : parent.left
            verticalCenter: internalLoader.isVertical ? undefined : parent.verticalCenter
            top: internalLoader.isVertical ? parent.top : undefined
            horizontalCenter: internalLoader.isVertical ? parent.horizontalCenter : undefined
        }
        width: 18
        height: 18
        svg: scrollbarSvg
        elementId: {
            if (leftMouseArea.pressed) {
                return internalLoader.isVertical ? "sunken-arrow-up" : "sunken-arrow-left"
            }

            if (scrollbar.activeFocus || leftMouseArea.containsMouse) {
                return internalLoader.isVertical ? "mouseover-arrow-up" : "mouseover-arrow-left"
            } else {
                return internalLoader.isVertical ? "arrow-up" : "arrow-left"
            }
        }

        MouseArea {
            id: leftMouseArea

            anchors.fill: parent
            enabled: scrollbar.enabled
            hoverEnabled: true

            Timer {
                id: leftTimer
                interval: scrollbar.scrollButtonInterval;
                running: parent.pressed
                repeat: true
                triggeredOnStart: true
                onTriggered: {
                    background.forceActiveFocus()
                    if (inverted) {
                        internalLoader.incrementValue(stepSize);
                    } else {
                        internalLoader.incrementValue(-stepSize);
                    }
                }
            }
        }
    }

    PlasmaCore.SvgItem {
        id: rightButton
        visible: leftButton.visible

        anchors {
            right: internalLoader.isVertical ? undefined : parent.right
            verticalCenter: internalLoader.isVertical ? undefined : parent.verticalCenter
            bottom: internalLoader.isVertical ? parent.bottom : undefined
            horizontalCenter: internalLoader.isVertical ? parent.horizontalCenter : undefined
        }
        width: 18
        height: 18
        svg: scrollbarSvg
        elementId: {
            if (rightMouseArea.pressed) {
                return internalLoader.isVertical ? "sunken-arrow-down" : "sunken-arrow-right"
            }

            if (scrollbar.activeFocus || rightMouseArea.containsMouse) {
                return internalLoader.isVertical ? "mouseover-arrow-down" : "mouseover-arrow-right"
            } else {
                return internalLoader.isVertical ? "arrow-down" : "arrow-right"
            }
        }

        MouseArea {
            id: rightMouseArea

            anchors.fill: parent
            enabled: scrollbar.enabled
            hoverEnabled: true

            Timer {
                id: rightTimer
                interval: scrollbar.scrollButtonInterval;
                running: parent.pressed;
                repeat: true
                triggeredOnStart: true
                onTriggered: {
                    background.forceActiveFocus();
                    if (inverted)
                        internalLoader.incrementValue(-stepSize);
                    else
                        internalLoader.incrementValue(stepSize);
                }
            }
        }
    }

    property MouseArea mouseArea: mouseArea
    MouseArea {
        id: mouseArea

        anchors.fill: contents
        enabled: scrollbar.enabled
        hoverEnabled: true
        drag {
            target: handle
            axis: internalLoader.isVertical ? Drag.YAxis : Drag.XAxis
            minimumX: range.positionAtMinimum
            maximumX: range.positionAtMaximum
            minimumY: range.positionAtMinimum
            maximumY: range.positionAtMaximum
        }

        onPressed: {
            if (internalLoader.isVertical) {
                // Clamp the value
                var newY = Math.max(mouse.y, drag.minimumY);
                newY = Math.min(newY, drag.maximumY);

                // Debounce the press: a press event inside the handler will not
                // change its position, the user needs to drag it.
                if (newY > handle.y + handle.height) {
                    handle.y = mouse.y - handle.height
                } else if (newY < handle.y) {
                    handle.y = mouse.y
                }
            } else {
                // Clamp the value
                var newX = Math.max(mouse.x, drag.minimumX);
                newX = Math.min(newX, drag.maximumX);

                // Debounce the press: a press event inside the handler will not
                // change its position, the user needs to drag it.
                if (newX > handle.x + handle.width) {
                    handle.x = mouse.x - handle.width
                } else if (newX < handle.x) {
                    handle.x = mouse.x
                }
            }

            background.forceActiveFocus();
        }

        Component.onCompleted: {
            acceptedButtons |= Qt.MiddleButton
        }
    }
}

