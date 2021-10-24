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
        PlasmaCore.SvgItem {
            anchors.centerIn: parent
            implicitWidth: naturalSize.width
            implicitHeight: naturalSize.height
            svg: sliderSvg
            elementId: control.horizontal ? "horizontal-slider-focus" : "vertical-slider-focus"
            visible: opacity > 0
            opacity: control.visualFocus
            Behavior on opacity {
                NumberAnimation {
                    duration: PlasmaCore.Units.longDuration
                    easing.type: Easing.OutCubic
                }
            }
        }
        PlasmaCore.SvgItem {
            anchors.centerIn: parent
            implicitWidth: naturalSize.width
            implicitHeight: naturalSize.height
            svg: sliderSvg
            elementId: control.horizontal ? "horizontal-slider-hover" : "vertical-slider-hover"
            visible: opacity > 0
            opacity: control.hovered
            Behavior on opacity {
                enabled: control.hovered
                NumberAnimation {
                    duration: PlasmaCore.Units.longDuration
                    easing.type: Easing.OutCubic
                }
            }
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
            id: grooveFill
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

        Loader {
            id: tickLoader
            readonly property int stepCount: (control.to - control.from) / control.stepSize
            visible: stepCount > 0 && stepCount <= 20
            active: visible
            LayoutMirroring.enabled: control.mirrored
            anchors {
                left: control.horizontal ? parent.left : parent.right
                top: control.vertical ? parent.top : parent.bottom
                leftMargin: control.horizontal ? control.handle.width/2 : PlasmaCore.Units.devicePixelRatio
                topMargin: control.vertical ? control.handle.height/2 : PlasmaCore.Units.devicePixelRatio
            }
            width: control.horizontal ? parent.width - control.handle.width : control.background.x
            height: control.vertical ? parent.height - control.handle.height : control.background.y
            sourceComponent: Grid {
                anchors.fill: parent
                rows: control.vertical ? tickLoader.stepCount + 1 : 1
                columns: control.horizontal ? tickLoader.stepCount + 1 : 1
                spacing: (control.vertical ? height : width) / tickLoader.stepCount - PlasmaCore.Units.devicePixelRatio
                Repeater {
                    model: tickLoader.stepCount
                    delegate: Rectangle {
                        property bool withinFill: (control.horizontal && x >= grooveFill.x && x <= grooveFill.x + grooveFill.width)
                            || (control.vertical && y >= grooveFill.y && y <= grooveFill.y + grooveFill.height)
                        width: control.vertical ? parent.width : PlasmaCore.Units.devicePixelRatio
                        height: control.horizontal ? parent.height : PlasmaCore.Units.devicePixelRatio
                        opacity: withinFill ? 1 : 0.3
                        color: withinFill ? PlasmaCore.ColorScope.highlightColor : PlasmaCore.ColorScope.textColor
                    }
                }
                Rectangle {
                    property bool withinFill: (control.horizontal && x >= grooveFill.x && x <= grooveFill.x + grooveFill.width)
                        || (control.vertical && y >= grooveFill.y && y <= grooveFill.y + grooveFill.height)
                    width: control.vertical ? parent.width : PlasmaCore.Units.devicePixelRatio
                    height: control.horizontal ? parent.height : PlasmaCore.Units.devicePixelRatio
                    opacity: withinFill ? 1 : 0.3
                    color: withinFill ? PlasmaCore.ColorScope.highlightColor : PlasmaCore.ColorScope.textColor
                }
            }
        }
    }
}
