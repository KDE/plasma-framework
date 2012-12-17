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

import QtQuick 1.0
import org.kde.plasma.core 0.1 as PlasmaCore
import "private" as Private

// TODO: create a value indicator for plasma?
/**
 * An interactive slider component with Plasma look and feel.
 */
Item {
    id: slider

    // Common API
    /**
     * type:real
     * How many steps the slider value can be selected within its range value.
     */
    property alias stepSize: range.stepSize

    /**
     * type:real
     * Minimum value that the slider's value can assume.
     *
     * The default value is 0.
     */
    property alias minimumValue: range.minimumValue

    /**
     * type:real
     * Maximum value that the slider's value can assume.
     *
     * The default value is 1.
     */
    property alias maximumValue: range.maximumValue

    /**
     * type:real
     * This property holds the value selected inside the minimum to maximum
     * range of value.
     *
     * The default value is 0.
     */
    property alias value: range.value

    /**
     * Orientation for this component.
     *
     * The orientation can be either Qt.Horizontal or Qt.Vertical.
     *
     * The default is Qt.Horizontal.
     */
    property int orientation: Qt.Horizontal

    /**
     * type:bool
     *
     * True if the Slider is being pressed.
     */
    property alias pressed: mouseArea.pressed

    /**
     * This property holds if a value indicator element will be shown while is
     * dragged or not.
     *
     * @warning The value indicator is not implemented in the Plasma Slider.
     *
     * The default value is false.
     */
    property bool valueIndicatorVisible: false

    /**
     * This property holds the text being displayed in the value indicator.
     *
     * @warning The value indicator is not implemented in the Plasma Slider.
     */
    property string valueIndicatorText: value

    // Plasma API
    /**
     * type:bool
     * This property holds if the slider visualizations has an inverted
     * direction.
     *
     * The default value is false.
     */
    property alias inverted: range.inverted

    width: contents.isVertical ? theme.defaultFont.mSize.height*1.6 : 200
    height: contents.isVertical ? 200 : theme.defaultFont.mSize.height*1.6
    // TODO: needs to define if there will be specific graphics for
    //     disabled sliders
    opacity: enabled ? 1.0 : 0.5

    Keys.onUpPressed: {
        if (!enabled || !contents.isVertical)
            return;

        if (inverted)
            value -= stepSize;
        else
            value += stepSize;
    }

    Keys.onDownPressed: {
        if (!enabled || !enabled)
            return;

        if (!contents.isVertical)
            return;

        if (inverted)
            value += stepSize;
        else
            value -= stepSize;
    }

    Keys.onLeftPressed: {
        if (!enabled || contents.isVertical)
            return;

        if (inverted)
            value += stepSize;
        else
            value -= stepSize;
    }

    Keys.onRightPressed: {
        if (!enabled || contents.isVertical)
            return;

        if (inverted)
            value -= stepSize;
        else
            value += stepSize;
    }

    Item {
        id: contents

        // Plasma API
        property bool animated: true
        property real handleWidth: contents.isVertical ? grooveSvg.elementSize("vertical-slider-handle").width : grooveSvg.elementSize("horizontal-slider-handle").width
        property real handleHeight: contents.isVertical ? grooveSvg.elementSize("vertical-slider-handle").height : grooveSvg.elementSize("horizontal-slider-handle").height

        // Convenience API
        property bool isVertical: orientation == Qt.Vertical

        width: contents.isVertical ? slider.height : slider.width
        height: contents.isVertical ? slider.width : slider.height
        rotation: contents.isVertical ? -90 : 0

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

            //use the same animation when resizing a slider as moving the slider this keeps it in line when using key shortcuts
            Behavior on width {
                enabled: !mouseArea.drag.active && contents.animated
                PropertyAnimation {
                    duration: behavior.enabled ? 150 : 0
                    easing.type: Easing.OutSine
                }
            }

            visible: range.position > 0 && slider.enabled
        }

        Private.RoundShadow {
            id: shadow
            imagePath: "widgets/slider"
            focusElement: contents.isVertical ? "vertical-slider-focus" : "horizontal-slider-focus"
            hoverElement: contents.isVertical ? "vertical-slider-hover" : "horizontal-slider-hover"
            shadowElement: contents.isVertical ? "vertical-slider-shadow" : "horizontal-slider-shadow"
            state: slider.activeFocus ? "focus" : (mouseArea.containsMouse ? "hover" : "shadow")
            anchors.fill: handle
            //We rotate the handle below, we need to rotate the shadow back as well
            rotation: contents.isVertical ? 90 : 0
        }

        PlasmaCore.SvgItem {
            id: handle

            x: fakeHandle.x
            anchors {
                verticalCenter: groove.verticalCenter
            }
            width: contents.handleWidth
            height: contents.handleHeight
            //Rotate the handle back for vertical slider so all the handles have the same shadow effect
            rotation: contents.isVertical ? 90 : 0
            svg: PlasmaCore.Svg { imagePath: "widgets/slider" }
            elementId: contents.isVertical ?  "vertical-slider-handle" : "horizontal-slider-handle"

            Behavior on x {
                id: behavior
                enabled: !mouseArea.drag.active && contents.animated

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
                    range.position = newX - handle.width / 2
                }

                slider.forceActiveFocus()
            }
        }
    }

    Binding {
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
