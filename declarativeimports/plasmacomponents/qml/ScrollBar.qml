/*
*   Copyright (C) 2011 by Daker Fernandes Pinheiro <dakerfp@gmail.com>
*   Copyright 2011 Marco Martin <mart@kde.org>
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

// TODO: add support mouse wheel and key events
Item {
    id: scrollbar

    // Common API
    property Flickable flickableItem: null
    property int orientation: Qt.Vertical
    property bool interactive: true

    // Plasma API
    property bool animated: true
    property bool inverted: false
    property bool updateValueWhileDragging: true
    property alias stepSize: range.stepSize
    //property alias pressed: mouseArea.pressed
    property real scrollButtonInterval: 50

    // Convinience API
    property bool _isVertical: orientation == Qt.Vertical
    property bool _showButtons: stepSize != 0

    implicitWidth: _isVertical ? 22 : 200
    implicitHeight: _isVertical ? 200 : 22
    // TODO: needs to define if there will be specific graphics for
    //     disabled scroll bars
    opacity: enabled ? 1.0 : 0.5

    visible: flickableItem && internalLoader.handleEnabled

    anchors {
        right: flickableItem.right
        left: (orientation == Qt.Vertical) ? undefined : flickableItem.left 
        top: (orientation == Qt.Vertical) ? flickableItem.top : undefined
        bottom: flickableItem.bottom
    }

    Keys.onUpPressed: {
        if (!enabled || !_isVertical)
            return;

        if (inverted)
            internalLoader.incrementValue(-stepSize);
        else
            internalLoader.incrementValue(stepSize);
    }

    Keys.onDownPressed: {
        if (!enabled || !_isVertical)
            return;

        if (inverted)
            internalLoader.incrementValue(stepSize);
        else
            internalLoader.incrementValue(-stepSize);
    }

    Keys.onLeftPressed: {
        if (!enabled || _isVertical)
            return;

        if (inverted)
            internalLoader.incrementValue(stepSize);
        else
            internalLoader.incrementValue(-stepSize);
    }

    Keys.onRightPressed: {
        if (!enabled || _isVertical)
            return;

        if (inverted)
            internalLoader.incrementValue(-stepSize);
        else
            internalLoader.incrementValue(stepSize);
    }

    Loader {
        id: internalLoader
        anchors.fill: parent
        //property bool handleEnabled: _isVertical ? item.handle.height < item.contents.height : item.handle.width < item.contents.width
        property bool handleEnabled: _isVertical ? flickableItem.contentHeight > flickableItem.height : flickableItem.contentWidth > flickableItem.width

        function incrementValue(increment)
        {
            if (!flickableItem)
                return;

            if (_isVertical) {
                flickableItem.contentY = Math.max(0, Math.min(flickableItem.contentHeight,
                    flickableItem.contentY + increment))
            } else {
                flickableItem.contentX = Math.max(0, Math.min(flickableItem.contentWidth,
                    flickableItem.contentX + increment))
            }
        }

        RangeModel {
            id: range

            minimumValue: 0
            maximumValue: {
                var diff;
                if (_isVertical) {
                    diff = flickableItem.contentHeight - flickableItem.height
                } else {
                    diff = flickableItem.contentWidth - flickableItem.width
                }

                return Math.max(0, diff)
            }

            stepSize: 10
            inverted: scrollbar.inverted
            positionAtMinimum: 0
            positionAtMaximum: {
                if (_isVertical) {
                    internalLoader.item.contents.height - internalLoader.item.handle.height
                } else {
                    internalLoader.item.contents.width - internalLoader.item.handle.width
                }
            }
            value: _isVertical ? flickableItem.contentY : flickableItem.contentX
            onValueChanged: {
                if (flickableItem.moving) {
                    return
                }

                if (_isVertical) {
                    flickableItem.contentY = value
                } else {
                    flickableItem.contentX = value
                }
            }

            position: _isVertical ? internalLoader.item.handle.y : internalLoader.item.handle.x

            onPositionChanged: {
                if (internalLoader.item.mouseArea.pressed) {
                    return
                }

                if (_isVertical) {
                    internalLoader.item.handle.y = position
                } else {
                    internalLoader.item.handle.x = position
                }
            }
        }

        sourceComponent: Component {
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
        }
    }
}
