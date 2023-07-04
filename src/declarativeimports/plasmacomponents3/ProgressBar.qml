/*
    SPDX-FileCopyrightText: 2016 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2023 ivan tkachenko <me@ratijas.tk>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.15
import QtQuick.Templates @QQC2_VERSION@ as T
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.ksvg 1.0 as KSvg

T.ProgressBar {
    id: control

    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                            implicitContentWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             implicitContentHeight + topPadding + bottomPadding)

    hoverEnabled: false

    KSvg.Svg {
        id: barSvg
        imagePath: "widgets/bar_meter_horizontal"
        colorGroup: PlasmaCore.ColorScope.colorGroup
    }

    contentItem: Item {
        implicitWidth: PlasmaCore.Units.gridUnit * 8
        implicitHeight: children[0].height

        KSvg.FrameSvgItem {
            // Indeterminate progress bar is a 1/4 chunk of available control
            // width, bouncing from half of its width to the left and up to
            // the other half of its width to the right. So it travels the
            // across full availableWidth, partially clipping at the ends.

            property real indeterminateProgress: 0 // Range: [0..1]

            readonly property real indeterminateNormalWidth: 1 / 4
            readonly property real indeterminateNormalPosition: indeterminateProgress - indeterminateNormalWidth / 2
            readonly property int indeterminateClippedPosition: {
                const normalClippedPosition = Math.max(0, indeterminateNormalPosition);
                return Math.round(control.availableWidth * normalClippedPosition);
            }

            function indeterminateClippedWidth(): int {
                // Convert to pixels first, so we don't get rounding errors at the right edge.
                const defaultWidth = Math.round(control.availableWidth * indeterminateNormalWidth);
                const unclippedPosition = Math.round(control.availableWidth * indeterminateNormalPosition)
                const availableWidth = control.availableWidth - indeterminateClippedPosition;

                if (unclippedPosition < 0) {
                    return defaultWidth + unclippedPosition;
                } else if (defaultWidth > availableWidth) {
                    return availableWidth;
                } else {
                    return defaultWidth;
                }
            }

            imagePath: "widgets/bar_meter_horizontal"
            prefix: "bar-active"
            colorGroup: PlasmaCore.ColorScope.colorGroup

            LayoutMirroring.enabled: control.mirrored
            anchors.left: parent.left
            anchors.leftMargin: control.indeterminate ? indeterminateClippedPosition : 0
            anchors.verticalCenter: parent.verticalCenter

            // unlike Slider, this width is allowed to be less than its minimum (margins) size, in which case it would not render at all.
            width: control.indeterminate ? indeterminateClippedWidth() : Math.round(control.position * control.availableWidth)
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

        KSvg.FrameSvgItem {
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
