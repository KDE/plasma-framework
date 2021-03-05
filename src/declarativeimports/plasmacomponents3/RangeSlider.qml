/*
    SPDX-FileCopyrightText: 2016 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.6
import QtQuick.Templates @QQC2_VERSION@ as T
import org.kde.plasma.core 2.0 as PlasmaCore
import "private" as Private

T.RangeSlider {
    id: control

    implicitWidth: control.orientation === Qt.Horizontal ? PlasmaCore.Units.gridUnit * 12 : PlasmaCore.Units.gridUnit * 1.6
    implicitHeight: control.orientation === Qt.Horizontal ? PlasmaCore.Units.gridUnit * 1.6 : PlasmaCore.Units.gridUnit * 12

    PlasmaCore.Svg {
        id: grooveSvg
        imagePath: "widgets/slider"
        colorGroup: PlasmaCore.ColorScope.colorGroup

    }
    first.handle: Item {
        property bool horizontal: control.orientation === Qt.Horizontal
        x: Math.round(control.leftPadding + (horizontal ? control.first.visualPosition * (control.availableWidth - width) : (control.availableWidth - width) / 2))
        y: Math.round(control.topPadding + (horizontal ? (control.availableHeight - height) / 2 : control.first.visualPosition * (control.availableHeight - height)))

        width: grooveSvg.hasElement("hint-handle-size") ? grooveSvg.elementSize("hint-handle-size").width : firstHandle.width
        height: grooveSvg.hasElement("hint-handle-size") ? grooveSvg.elementSize("hint-handle-size").height : firstHandle.height

        Private.RoundShadow {
            anchors.fill: firstHandle
            imagePath: "widgets/slider"
            focusElement: parent.horizontal ? "horizontal-slider-focus" : "vertical-slider-focus"
            hoverElement: parent.horizontal ? "horizontal-slider-hover" : "vertical-slider-hover"
            shadowElement: parent.horizontal ? "horizontal-slider-shadow" : "vertical-slider-shadow"
            state: control.activeFocus ? "focus" : (control.hovered ? "hover" : "shadow")
        }
        PlasmaCore.SvgItem {
            id: firstHandle
            anchors.centerIn: parent
            width: naturalSize.width
            height: naturalSize.height
            svg: grooveSvg
            elementId: parent.horizontal ? "horizontal-slider-handle" : "vertical-slider-handle"
        }
    }

    second.handle: Item {
        property bool horizontal: control.orientation === Qt.Horizontal
        x: control.leftPadding + (horizontal ? control.second.visualPosition * (control.availableWidth - width) : (control.availableWidth - width) / 2)
        y: control.topPadding + (horizontal ? (control.availableHeight - height) / 2 : control.second.visualPosition * (control.availableHeight - height))

        width: grooveSvg.hasElement("hint-handle-size") ? grooveSvg.elementSize("hint-handle-size").width : secondHandle.width
        height: grooveSvg.hasElement("hint-handle-size") ? grooveSvg.elementSize("hint-handle-size").height : secondHandle.height

        Private.RoundShadow {
            anchors.fill: secondHandle
            imagePath: "widgets/slider"
            focusElement: parent.horizontal ? "horizontal-slider-focus" : "vertical-slider-focus"
            hoverElement: parent.horizontal ? "horizontal-slider-hover" : "vertical-slider-hover"
            shadowElement: parent.horizontal ? "horizontal-slider-shadow" : "vertical-slider-shadow"
            state: control.activeFocus ? "focus" : (control.hovered ? "hover" : "shadow")
        }
        PlasmaCore.SvgItem {
            id: secondHandle
            anchors.centerIn: parent
            width: naturalSize.width
            height: naturalSize.height
            svg: grooveSvg
            elementId: parent.horizontal ? "horizontal-slider-handle" : "vertical-slider-handle"
        }
    }

    background: PlasmaCore.FrameSvgItem {
        imagePath: "widgets/slider"
        prefix: "groove"
        colorGroup: PlasmaCore.ColorScope.colorGroup
        readonly property bool horizontal: control.orientation === Qt.Horizontal
        implicitWidth: horizontal ? PlasmaCore.Units.gridUnit * 8 : margins.left + margins.right
        implicitHeight: horizontal ? margins.top + margins.bottom : PlasmaCore.Units.gridUnit * 8
        width: horizontal ? control.availableWidth : implicitWidth
        height: horizontal ? implicitHeight : control.availableHeight
        anchors.centerIn: parent
        scale: horizontal && control.mirrored ? -1 : 1
        opacity: control.enabled ? 1 : 0.6

        PlasmaCore.FrameSvgItem {
            imagePath: "widgets/slider"
            prefix: "groove-highlight"
            colorGroup: PlasmaCore.ColorScope.colorGroup
            x: parent.horizontal ? control.first.position * parent.width : 0
            y: parent.horizontal ? 0 : control.second.visualPosition * parent.height
            width: parent.horizontal ? control.second.position * parent.width - control.first.position * parent.width : parent.width
            height: parent.horizontal ? parent.height : control.second.position * parent.height - control.first.position * parent.height
        }
    }
}
