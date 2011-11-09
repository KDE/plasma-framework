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
    prefix: _isVertical ? "background-vertical" : "background-horizontal"

    property Item handle: handle

    property Item contents: contents
    Item {
        id: contents
        anchors {
            fill: parent
            leftMargin: _isVertical || stepSize <= 0 ? 0 : leftButton.width
            rightMargin: _isVertical || stepSize <= 0 ? 0 : rightButton.width
            topMargin: _isVertical && stepSize > 0 ? leftButton.height : 0
            bottomMargin: _isVertical && stepSize > 0 ? rightButton.height : 0
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

            property int length: _isVertical? flickableItem.visibleArea.heightRatio * parent.height :  flickableItem.visibleArea.widthRatio * parent.width

            width: _isVertical ? parent.width : length
            height: _isVertical ? length : parent.height
        }
    }

    PlasmaCore.Svg {
        id: scrollbarSvg
        imagePath: "widgets/scrollbar"
    }

    PlasmaCore.SvgItem {
        id: leftButton
        visible: stepSize > 0

        anchors {
            left: _isVertical ? undefined : parent.left
            verticalCenter: _isVertical ? undefined : parent.verticalCenter
            top: _isVertical ? 0 : undefined
            horizontalCenter: _isVertical ? parent.horizontalCenter : undefined
        }
        height: 18
        width: _showButtons ? 18 : 0
        svg: scrollbarSvg
        elementId: {
            if (leftMouseArea.pressed) {
                return _isVertical ? "sunken-arrow-up" : "sunken-arrow-left"
            }

            if (scrollbar.activeFocus || leftMouseArea.containsMouse) {
                return _isVertical ? "mouseover-arrow-up" : "mouseover-arrow-left"
            } else {
                return _isVertical ? "mouseover-arrow-up" : "arrow-left"
            }
        }

        MouseArea {
            id: leftMouseArea

            anchors.fill: parent
            enabled: scrollbar.enabled
            Timer {
                id: leftTimer
                interval: scrollbar.scrollButtonInterval;
                running: parent.pressed
                repeat: true
                onTriggered: {
                    scrollbar.forceActiveFocus()
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
        visible: stepSize > 0

        anchors {
            right: _isVertical ? undefined : parent.right
            verticalCenter: _isVertical ? undefined : parent.verticalCenter
            bottom: _isVertical ? parent.bottom : undefined
            horizontalCenter: _isVertical ? parent.horizontalCenter : undefined
        }
        height: 18
        width: _showButtons ? 18 : 0
        svg: scrollbarSvg
        elementId: {
            if (leftMouseArea.pressed) {
                return _isVertical ? "sunken-arrow-down" : "sunken-arrow-right"
            }

            if (scrollbar.activeFocus || leftMouseArea.containsMouse) {
                return _isVertical ? "mouseover-arrow-down" : "mouseover-arrow-right"
            } else {
                return _isVertical ? "mouseover-arrow-down" : "arrow-right"
            }
        }

        MouseArea {
            id: rightMouseArea

            anchors.fill: parent
            enabled: scrollbar.enabled
            Timer {
                id: rightTimer
                interval: scrollbar.scrollButtonInterval;
                running: parent.pressed;
                repeat: true
                onTriggered: {
                    scrollbar.forceActiveFocus();
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
            axis: _isVertical ? Drag.YAxis : Drag.XAxis
            minimumX: range.positionAtMinimum
            maximumX: range.positionAtMaximum
            minimumY: range.positionAtMinimum
            maximumY: range.positionAtMaximum
        }

        onPressed: {
            if (_isVertical) {
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

            scrollbar.forceActiveFocus();
        }
    }
}

