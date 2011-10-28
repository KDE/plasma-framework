/*
*   Copyright (C) 2011 by Daker Fernandes Pinheiro <dakerfp@gmail.com>
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
    property bool interactive

    // Plasma API
    property int orientation: Qt.Vertical
    property bool animated: true
    property bool inverted: false
    property bool updateValueWhileDragging: true
    property alias stepSize: range.stepSize
    property alias pressed: mouseArea.pressed
    property real scrollButtonInterval: 50

    // Convinience API
    property bool _isVertical: orientation == Qt.Vertical
    property bool _showButtons: stepSize != 0
    property bool _inverted: _isVertical ?
        !scrollbar.inverted : scrollbar.inverted

    implicitWidth: _isVertical ? 22 : 200
    implicitHeight: _isVertical ? 200 : 22
    // TODO: needs to define if there will be specific graphics for
    //     disabled scroll bars
    opacity: enabled ? 1.0 : 0.5

    visible: flickableItem && handle.width < contents.width

    anchors {
        right: flickableItem.right
        left: (orientation == Qt.Vertical) ? undefined : flickableItem.left 
        top: (orientation == Qt.Vertical) ? flickableItem.top : undefined
        bottom: flickableItem.bottom
    }

    function incrementValue(increment) {
        if (!flickableItem)
            return;

        if (_isVertical) {
            flickableItem.contentY = Math.min(flickableItem.contentHeight,
                flickableItem.contentY + increment);
        } else {
            flickableItem.contentX = Math.min(flickableItem.contentWidth,
                flickableItem.contentX + increment);
        }
    }

    Keys.onUpPressed: {
        if (!enabled || !_isVertical)
            return;

        if (_inverted)
            incrementValue(-stepSize);
        else
            incrementValue(stepSize);
    }

    Keys.onDownPressed: {
        if (!enabled || !_isVertical)
            return;

        if (_inverted)
            incrementValue(stepSize);
        else
            incrementValue(-stepSize);
    }

    Keys.onLeftPressed: {
        if (!enabled || _isVertical)
            return;

        if (_inverted)
            incrementValue(stepSize);
        else
            incrementValue(-stepSize);
    }

    Keys.onRightPressed: {
        if (!enabled || _isVertical)
            return;

        if (_inverted)
            incrementValue(-stepSize);
        else
            incrementValue(stepSize);
    }


    Item {

        width: _isVertical ? scrollbar.height : scrollbar.width
        height: _isVertical ? scrollbar.width : scrollbar.height
        rotation: _isVertical ? -90 : 0

        anchors.centerIn: parent
        PlasmaCore.Svg {
            id: scrollbarSvg
            imagePath: "widgets/scrollbar"
        }

        PlasmaCore.SvgItem {
            id: leftButton

            anchors {
                left: parent.left
                verticalCenter: parent.verticalCenter
            }
            height: 18
            width: _showButtons ? 18 : 0
            svg: scrollbarSvg
            elementId: {
                if (leftMousArea.pressed)
                    return "sunken-arrow-left";

                if (scrollbar.activeFocus || leftMousArea.containsMouse)
                    return "mouseover-arrow-left";
                else
                    return "arrow-left";
            }

            MouseArea {
                id: leftMousArea

                anchors.fill: parent
                enabled: scrollbar.enabled
                Timer {
                    id: leftTimer
                    interval: scrollbar.scrollButtonInterval;
                    running: parent.pressed
                    repeat: true
                    onTriggered: {
                        scrollbar.forceActiveFocus();
                        if (_inverted)
                            incrementValue(stepSize);
                        else
                            incrementValue(-stepSize);
                    }
                }
            }
        }

        PlasmaCore.SvgItem {
            id: rightButton

            anchors {
                right: parent.right
                verticalCenter: parent.verticalCenter
            }
            height: 18
            width: _showButtons ? 18 : 0
            svg: scrollbarSvg
            elementId: {
                if (rightMousArea.pressed)
                    return "sunken-arrow-right";

                if (scrollbar.activeFocus || rightMousArea.containsMouse)
                    return "mouseover-arrow-right";
                else
                    return "arrow-right";
            }

            MouseArea {
                id: rightMousArea

                anchors.fill: parent
                enabled: scrollbar.enabled
                Timer {
                    id: rightTimer
                    interval: scrollbar.scrollButtonInterval;
                    running: parent.pressed;
                    repeat: true
                    onTriggered: {
                        scrollbar.forceActiveFocus();
                        if (_inverted)
                            incrementValue(-stepSize);
                        else
                            incrementValue(stepSize);
                    }
                }
            }
        }

        Item {
            id: contents

            anchors {
                left: leftButton.right
                top: parent.top
                bottom: parent.bottom
                right: rightButton.left
            }

            RangeModel {
                id: range

                minimumValue: 0
                maximumValue: {
                    var diff;
                    if (_isVertical)
                        diff = flickableItem.contentHeight - flickableItem.height;
                    else
                        diff = flickableItem.contentWidth - flickableItem.width;

                    return Math.max(0, diff);
                }
                stepSize: 0.0
                inverted: _inverted
                positionAtMinimum: 0 + handle.width / 2
                positionAtMaximum: contents.width - handle.width / 2
                value: _isVertical ? flickableItem.contentY : flickableItem.contentX
                onValueChanged: {
                    if (flickableItem.flicking)
                        return;

                    if (_isVertical)
                        flickableItem.contentY = value;
                    else
                        flickableItem.contentX = value;
                }
                position: handle.x
                onPositionChanged: { handle.x = position; }
            }

            PlasmaCore.FrameSvgItem {
                id: groove

                anchors.fill: parent
                imagePath: "widgets/scrollbar"
                prefix: "background-horizontal"
            }

            PlasmaCore.FrameSvgItem {
                id: handle

                transform: Translate { x: - handle.width / 2 }
                x: fakeHandle.x
                anchors.verticalCenter: groove.verticalCenter
                width: {
                    var ratio;
                    if (_isVertical)
                        ratio = flickableItem.visibleArea.heightRatio;
                    else
                        ratio = flickableItem.visibleArea.widthRatio;

                    return ratio * parent.width;
                }
                height: parent.height - margins.top // TODO: check mergin
                imagePath: "widgets/scrollbar"
                prefix: {
                    if (scrollbar.pressed)
                        return "sunken-slider";

                    if (scrollbar.activeFocus || mouseArea.containsMouse)
                        return "mouseover-slider";
                    else
                        return "slider";
                }

                Behavior on x {
                    id: behavior
                    enabled: !mouseArea.drag.active && scrollbar.animated &&
                        !flickableItem.flicking

                    PropertyAnimation {
                        duration: behavior.enabled ? 150 : 0
                        easing.type: Easing.OutSine
                    }
                }
            }

            Item {
                id: fakeHandle
                width: handle.width
                height: handle.height
                transform: Translate { x: - handle.width / 2 }
            }

            MouseArea {
                id: mouseArea

                anchors.fill: parent
                enabled: scrollbar.enabled
                drag {
                    target: fakeHandle
                    axis: Drag.XAxis
                    minimumX: range.positionAtMinimum
                    maximumX: range.positionAtMaximum
                }

                onPressed: {
                    // Clamp the value
                    var newX = Math.max(mouse.x, drag.minimumX);
                    newX = Math.min(newX, drag.maximumX);

                    // Debounce the press: a press event inside the handler will not
                    // change its position, the user needs to drag it.
                    if (Math.abs(newX - fakeHandle.x) > handle.width / 2)
                        range.position = newX;

                    scrollbar.forceActiveFocus();
                }
                onReleased: {
                    // If we don't update while dragging, this is the only
                    // moment that the range is updated.
                    if (!scrollbar.updateValueWhileDragging)
                        range.position = fakeHandle.x;
                }
            }
        }

        // Range position normally follow fakeHandle, except when
        // 'updateValueWhileDragging' is false. In this case it will only follow
        // if the user is not pressing the handle.
        Binding {
            when: updateValueWhileDragging || !mouseArea.pressed
            target: range
            property: "position"
            value: fakeHandle.x
        }

        // During the drag, we simply ignore position set from the range, this
        // means that setting a value while dragging will not "interrupt" the
        // dragging activity.
        Binding {
            when: !mouseArea.drag.active
            target: fakeHandle
            property: "x"
            value: range.position
        }
    }
}
