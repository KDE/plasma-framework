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
            id: bar

            // Indeterminate progress bar is a 1/4 chunk of available control
            // width, bouncing from half of its width to the left and up to
            // the other half of its width to the right. So it travels the
            // across full availableWidth, partially clipping at the ends.

            property real indeterminateProgress: 0 // Range: [0..1]

            // "Normal" means normalized to 0..1 scale. Everything else is in pixels here.
            readonly property real indeterminateNormalWidth: 1 / 3
            readonly property real indeterminateNormalPosition: indeterminateProgress - indeterminateNormalWidth / 2

            function indeterminateUnclippedPosition(): int {
                return Math.round(control.availableWidth * indeterminateNormalPosition);
            }

            function indeterminateClippedPosition(): int {
                return Math.max(0, indeterminateUnclippedPosition());
            }

            function indeterminateUnclippedWidth(): real {
                return Math.round(control.availableWidth * indeterminateNormalWidth);
            }

            function indeterminateClippedWidth(): int {
                // Convert to pixels first, so we don't get rounding errors at the right edge.
                const unclippedPosition = Math.round(control.availableWidth * indeterminateNormalPosition)
                const preferredWidth = indeterminateUnclippedWidth();

                return clipWidth(unclippedPosition, preferredWidth);
            }

            // Non-indeterminate progress bar follows control.position percent.
            // The two states are glued by an interpolation animation.

            function determinateWidth(): int {
                return Math.round(control.availableWidth * control.position);
            }

            // 0 -> follow percent value.  1 -> follow indeterminate state.
            property real indeterminateMix: 0

            onIndeterminateMixChanged: print("III", Math.round(indeterminateMix * 1000) / 1000);


            function mix(a: real, b: real, t: real): real {
                return lerp(a, b, t);
            }

            function lerp(a: real, b: real, t: real): real {
                return a + t * (b - a);
            }

            function interpolatedUnclippedPosition(): int {
                // Use switch case for edge cases to avoid extra bindings
                switch (indeterminateMix) {
                case 0:
                    return 0;
                case 1:
                    return indeterminateUnclippedPosition();
                default:
                    return Math.round(mix(0, indeterminateUnclippedPosition(), indeterminateMix));
                }
            }

            function interpolatedClippedPosition(): int {
                return Math.max(0, interpolatedUnclippedPosition());
            }

            function interpolatedUnclippedWidth(): int {
                // Use switch case for edge cases to avoid extra bindings
                switch (indeterminateMix) {
                case 0:
                    return determinateWidth();
                case 1:
                    return indeterminateUnclippedWidth();
                default:
                    return Math.round(mix(determinateWidth(), indeterminateUnclippedWidth(), indeterminateMix));
                }
            }

            function interpolatedClippedWidth(): int {
                return clipWidth(interpolatedUnclippedPosition(), interpolatedUnclippedWidth());
            }

            function clipWidth(position: int, preferredWidth: int): int {
                const availableWidth = control.availableWidth - interpolatedClippedPosition();
                let width = preferredWidth;
                if (position < 0) {
                    width += position;
                }
                if (width > availableWidth) {
                    width = availableWidth;
                }
                return width;
            }

            // property bool overridePosition: false

            imagePath: "widgets/bar_meter_horizontal"
            prefix: "bar-active"
            colorGroup: PlasmaCore.ColorScope.colorGroup

            LayoutMirroring.enabled: control.mirrored
            anchors.left: parent.left
            anchors.leftMargin: interpolatedClippedPosition()
            anchors.verticalCenter: parent.verticalCenter

            // unlike Slider, this width is allowed to be less than its minimum (margins) size, in which case it would not render at all.
            width: interpolatedClippedWidth()
            height: barSvg.hasElement("hint-bar-size")
                ? barSvg.elementSize("hint-bar-size").height
                : fixedMargins.top + fixedMargins.bottom

            visible: width >= fixedMargins.left + fixedMargins.right ? 1 : 0

            SequentialAnimation on indeterminateProgress {
                id: progressAnimation
                loops: Animation.Infinite
                running: false

                onRunningChanged: print("P running", control, running)

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

            NumberAnimation on indeterminateMix {
                id: mixAnimation
                running: false
                duration: PlasmaCore.Units.humanMoment / 4
                easing.type: Easing.Linear

                onRunningChanged: print("M running", control, running)

                onFinished: {
                    if (!control.indeterminate) {
                        progressAnimation.stop();
                    }
                }
            }

            function targetAnimationInterpolationMix(): real {
                return control.indeterminate ? 1 : 0;
            }

            Connections {
                target: control

                function onVisibleChanged() {
                    mixAnimation.stop();
                    progressAnimation.stop();
                    bar.indeterminateMix = bar.targetAnimationInterpolationMix();
                    if (control.indeterminate) {
                        progressAnimation.start();
                    }
                }

                function onIndeterminateChanged() {
                    if (control.visible) {
                        mixAnimation.stop();
                        mixAnimation.to = bar.targetAnimationInterpolationMix();
                        mixAnimation.start();

                        if (control.indeterminate) {
                            progressAnimation.start();
                        } else {
                            progressAnimation.stop();
                        }

                        // progressAnimation.start();
                    }
                }
            }

            Component.onCompleted: {
                mixAnimation.stop();
                indeterminateMix = bar.targetAnimationInterpolationMix();
                if (control.visible && control.indeterminate) {
                    progressAnimation.start();
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
