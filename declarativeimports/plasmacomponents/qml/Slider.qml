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

/**Documentanted API
Inherits:
        Item

Imports:
        QtQuick 1.0
        org.kde.plasma.core

Description:
        A simple slider.

Properties:
        alias stepSize: range.stepSize
        Sets the step with which the slider will move.

        alias minimumValue:
        TODO
        alias maximumValue:
        TODO
        alias value:
        TODO

        int orientation:
        Sets the orientation for the slider.

        alias pressed:
        If it is true then the slider has been pressed.

        bool valueIndicatorVisible:

        string valueIndicatorText:

        bool animated:
        Sets if the slider will animate or not.

        alias inverted:

        bool updateValueWhileDragging:

        real handleSize:
**/

import QtQuick 1.0
import org.kde.plasma.core 0.1 as PlasmaCore

// TODO: create a value indicator for plasma?
Item {
    id: slider

    // Common API
    property alias stepSize: range.stepSize
    property alias minimumValue: range.minimumValue
    property alias maximumValue: range.maximumValue
    property alias value: range.value
    property int orientation: Qt.Horizontal
    property alias pressed: mouseArea.pressed
    property bool valueIndicatorVisible: false
    property string valueIndicatorText: value

    // Plasma API
    property bool animated: false
    property alias inverted: range.inverted
    property bool updateValueWhileDragging: true
    property real handleSize: 22

    // Convenience API
    property bool _isVertical: orientation == Qt.Vertical

    width: _isVertical ? theme.defaultFont.mSize.height*1.8 : 200
    height: _isVertical ? 200 : theme.defaultFont.mSize.height*1.8
    // TODO: needs to define if there will be specific graphics for
    //     disabled sliders
    opacity: enabled ? 1.0 : 0.5

    Keys.onUpPressed: {
        if (!enabled || !_isVertical)
            return;

        if (inverted)
            value -= stepSize;
        else
            value += stepSize;
    }

    Keys.onDownPressed: {
        if (!enabled || !enabled)
            return;

        if (!_isVertical)
            return;

        if (inverted)
            value += stepSize;
        else
            value -= stepSize;
    }

    Keys.onLeftPressed: {
        if (!enabled || _isVertical)
            return;

        if (inverted)
            value += stepSize;
        else
            value -= stepSize;
    }

    Keys.onRightPressed: {
        if (!enabled || _isVertical)
            return;

        if (inverted)
            value -= stepSize;
        else
            value += stepSize;
    }

    Item {
        id: contents

        width: _isVertical ? slider.height : slider.width
        height: _isVertical ? slider.width : slider.height
        rotation: _isVertical ? -90 : 0

        anchors.centerIn: parent

        RangeModel {
            id: range

            minimumValue: 0.0
            maximumValue: 1.0
            value: 0
            stepSize: 0.0
            inverted: false
            positionAtMinimum: 0
            positionAtMaximum: contents.width - handle.width
        }

        PlasmaCore.Svg {
            id: grooveSvg
            imagePath: "widgets/slider"
        }
        PlasmaCore.FrameSvgItem {
            id: groove
            imagePath: "widgets/slider"
            prefix: "groove"
            //FIXME: frameSvg should have a minimumSize attribute, could be added to kdelibs 4.7(maybe just the qml binding is enough)?
            height: grooveSvg.elementSize("groove-topleft").height + grooveSvg.elementSize("groove-bottomleft").height
            anchors {
                left: parent.left
                right: parent.right
                verticalCenter: parent.verticalCenter
            }
        }
        PlasmaCore.FrameSvgItem {
            id: highlight
            imagePath: "widgets/slider"
            prefix: "groove-highlight"
            height: groove.height
            width: inverted ? groove.width - handle.x : fakeHandle.x + handle.width/2
            x: inverted ? handle.x : 0
            anchors.verticalCenter: parent.verticalCenter

            visible: range.position > 0 && slider.enabled
        }

        RoundShadow {
            id: shadow
            imagePath: "widgets/slider"
            focusElement: "horizontal-slider-focus"
            hoverElement: "horizontal-slider-hover"
            shadowElement: "horizontal-slider-shadow"
            state: slider.activeFocus ? "focus" : (mouseArea.containsMouse ? "hover" : "shadow")
            anchors.fill: handle
        }

        PlasmaCore.SvgItem {
            id: handle

            x: fakeHandle.x
            anchors {
                verticalCenter: groove.verticalCenter
            }
            width: handleSize
            height: handleSize
            svg: PlasmaCore.Svg { imagePath: "widgets/slider" }
            elementId: "horizontal-slider-handle"

            Behavior on x {
                id: behavior
                enabled: !mouseArea.drag.active && slider.animated

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
        }

        MouseArea {
            id: mouseArea

            anchors.fill: parent
            enabled: slider.enabled
            drag {
                target: fakeHandle
                axis: Drag.XAxis
                minimumX: range.positionAtMinimum
                maximumX: range.positionAtMaximum
            }
            hoverEnabled: true

            onPressed: {
                // Clamp the value
                var newX = Math.max(mouse.x, drag.minimumX)
                newX = Math.min(newX, drag.maximumX)

                // Debounce the press: a press event inside the handler will not
                // change its position, the user needs to drag it.
                if (Math.abs(newX - fakeHandle.x) > handle.width / 2) {
                    range.position = newX
                }

                slider.forceActiveFocus()
            }
            onReleased: {
                // If we don't update while dragging, this is the only
                // moment that the range is updated.
                if (!slider.updateValueWhileDragging) {
                    range.position = fakeHandle.x
                }
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