/*
    SPDX-FileCopyrightText: 2016 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.6
import QtQuick.Templates @QQC2_VERSION@ as T
import org.kde.plasma.core 2.0 as PlasmaCore
import "private" as P

T.Slider {
    id: control

    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                            implicitHandleWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             implicitHandleHeight + topPadding + bottomPadding)

    wheelEnabled: true
    snapMode: T.Slider.SnapOnRelease

    layer.enabled: opacity < 1
    opacity: control.enabled ? 1 : 0.5

    PlasmaCore.Svg {
        id: sliderSvg
        imagePath: "widgets/slider"
        colorGroup: PlasmaCore.ColorScope.colorGroup

    }

    handle: Item {
        x: Math.round(control.leftPadding + (horizontal ? control.visualPosition * (control.availableWidth - width) : (control.availableWidth - width) / 2))
        y: Math.round(control.topPadding + (horizontal ? (control.availableHeight - height) / 2 : control.visualPosition * (control.availableHeight - height)))

        implicitWidth: sliderSvg.hasElement("hint-handle-size") ? sliderSvg.elementSize("hint-handle-size").width : firstHandle.implicitWidth
        implicitHeight: sliderSvg.hasElement("hint-handle-size") ? sliderSvg.elementSize("hint-handle-size").height : firstHandle.implicitHeight

        PlasmaCore.SvgItem {
            id: shadow
            z: -1
            anchors.centerIn: parent
            implicitWidth: naturalSize.width
            implicitHeight: naturalSize.height
            svg: sliderSvg
            elementId: control.horizontal ? "horizontal-slider-shadow" : "vertical-slider-shadow"
            visible: enabled && !control.pressed
        }
        PlasmaCore.SvgItem {
            id: firstHandle
            anchors.centerIn: parent
            implicitWidth: naturalSize.width
            implicitHeight: naturalSize.height
            svg: sliderSvg
            elementId: control.horizontal ? "horizontal-slider-handle" : "vertical-slider-handle"
        }
        P.HoverFocusSvgItem {
            id: hoverFocus
            anchors.centerIn: parent
            svg: sliderSvg
            focusElement: control.horizontal ? "horizontal-slider-focus" : "vertical-slider-focus"
            hoverElement: control.horizontal ? "horizontal-slider-hover" : "vertical-slider-hover"
            elementId: if (control.visualFocus) {
                hoverFocus.focusElement
            } else {
                hoverFocus.hoverElement
            }
            // HoverFocusSvgItem contains a behavior animation
            opacity: control.hovered || control.visualFocus
        }
    }

    background: PlasmaCore.FrameSvgItem {
        imagePath: "widgets/slider"
        prefix: "groove"
        colorGroup: PlasmaCore.ColorScope.colorGroup
        implicitWidth: control.horizontal ? PlasmaCore.Units.gridUnit * 12 : fixedMargins.left + fixedMargins.right
        implicitHeight: control.vertical ? PlasmaCore.Units.gridUnit * 12 : fixedMargins.top + fixedMargins.bottom
        width: control.horizontal ? control.availableWidth : implicitWidth
        height: control.vertical ? control.availableHeight : implicitHeight
        x: control.leftPadding + (control.horizontal ? 0 : (control.availableWidth - width) / 2)
        y: control.topPadding + (control.vertical ? 0 : (control.availableHeight - height) / 2)

        PlasmaCore.FrameSvgItem {
            imagePath: "widgets/slider"
            prefix: "groove-highlight"
            colorGroup: PlasmaCore.ColorScope.colorGroup
            implicitWidth: fixedMargins.left + fixedMargins.right
            implicitHeight: fixedMargins.top + fixedMargins.bottom
            LayoutMirroring.enabled: control.mirrored
            anchors {
                fill: parent
                leftMargin: control.horizontal ? /*startPosition*/0 * parent.width - (/*startPosition*/0 * implicitWidth) : 0
                rightMargin: control.horizontal ? (1-/*endPosition*/control.position) * parent.width - ((1-/*endPosition*/control.position) * implicitWidth) : 0
                topMargin: control.vertical ? (1-/*endPosition*/control.position) * parent.height - ((1-/*endPosition*/control.position) * implicitHeight) : 0
                bottomMargin: control.vertical ? /*startPosition*/0 * parent.height - (/*startPosition*/0 * implicitHeight) : 0
            }
        }

        Repeater {
            id: repeater
            readonly property int stepCount: (control.to - control.from) / control.stepSize
            model: control.stepSize && stepCount < 20 ? 1 + stepCount : 0
            anchors.fill: parent

            Rectangle {
                color: PlasmaCore.ColorScope.textColor
                opacity: 0.3
                width: background.horizontal ? PlasmaCore.Units.devicePixelRatio : PlasmaCore.Units.gridUnit/2
                height: background.horizontal ? PlasmaCore.Units.gridUnit/2 : PlasmaCore.Units.devicePixelRatio
                y: background.horizontal ? background.height + PlasmaCore.Units.devicePixelRatio : handle.height / 2 + index * ((repeater.height - handle.height) / (repeater.count > 1 ? repeater.count - 1 : 1))
                x: background.horizontal ? handle.width / 2 + index * ((repeater.width - handle.width) / (repeater.count > 1 ? repeater.count - 1 : 1)) : background.width
            }
        }
    }
}
