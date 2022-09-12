/*
    SPDX-FileCopyrightText: 2016 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.15
import QtQuick.Templates @QQC2_VERSION@ as T
import org.kde.plasma.core 2.0 as PlasmaCore

T.ProgressBar {
    id: control

    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                            implicitContentWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             implicitContentHeight + topPadding + bottomPadding)

    hoverEnabled: false

    PlasmaCore.Svg {
        id: barSvg
        imagePath: "widgets/bar_meter_horizontal"
        colorGroup: PlasmaCore.ColorScope.colorGroup
    }

    contentItem: Item {
        implicitWidth: PlasmaCore.Units.gridUnit * 8
        implicitHeight: children[0].height

        PlasmaCore.FrameSvgItem {
            readonly property real indeterminateWidth: Math.round(control.availableWidth / 4)
            property real indeterminateProgress: 0

            imagePath: "widgets/bar_meter_horizontal"
            prefix: "bar-active"
            colorGroup: PlasmaCore.ColorScope.colorGroup

            LayoutMirroring.enabled: control.mirrored
            anchors.left: parent.left
            anchors.leftMargin: control.indeterminate ? indeterminateProgress * (control.availableWidth - indeterminateWidth) : 0
            anchors.verticalCenter: parent.verticalCenter

            // unlike Slider, this width is allowed to be less than its minimum (margins) size, in which case it would not render at all.
            width: control.indeterminate ? indeterminateWidth : Math.round(control.position * control.availableWidth)
            height: barSvg.hasElement("hint-bar-size")
                ? barSvg.elementSize("hint-bar-size").height
                : fixedMargins.top + fixedMargins.bottom

            visible: width >= fixedMargins.left + fixedMargins.right

            SequentialAnimation on indeterminateProgress {
                loops: Animation.Infinite
                running: control.indeterminate && control.contentItem.visible

                NumberAnimation {
                    duration: PlasmaCore.Units.humanMoment / 2
                    easing.type: Easing.InOutSine
                    to: 1
                }
                NumberAnimation {
                    duration: PlasmaCore.Units.humanMoment / 2
                    easing.type: Easing.InOutSine
                    to: 0
                }
            }
        }
    }

    background: Item {
        implicitWidth: PlasmaCore.Units.gridUnit * 8
        implicitHeight: children[0].height

        PlasmaCore.FrameSvgItem {
            imagePath: "widgets/bar_meter_horizontal"
            prefix: "bar-inactive"
            colorGroup: PlasmaCore.ColorScope.colorGroup

            anchors.centerIn: parent
            width: Math.max(parent.width, fixedMargins.left + fixedMargins.right)
            height: barSvg.hasElement("hint-bar-size")
                ? barSvg.elementSize("hint-bar-size").height
                : fixedMargins.top + fixedMargins.bottom
        }
    }
}
