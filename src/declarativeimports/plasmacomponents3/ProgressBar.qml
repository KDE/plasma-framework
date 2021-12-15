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
        implicitWidth: barSvg.elementSize("hint-bar-size").width || fixedMargins.left + fixedMargins.right
        implicitHeight: barSvg.elementSize("hint-bar-size").height || fixedMargins.top + fixedMargins.bottom
        PlasmaCore.FrameSvgItem {
            id: barFill
            property bool visibleIndeterminate: control.indeterminate && barFill.parent.visible
            property real indeterminateWidth: Math.round(parent.width / 4)
            height: parent.height
            width: anim.actuallyRunning ? indeterminateWidth : parent.width * control.position
            x: control.mirrored ? parent.width - width : 0
            imagePath: "widgets/bar_meter_horizontal"
            prefix: "bar-active"
            colorGroup: PlasmaCore.ColorScope.colorGroup
            visible: width >= fixedMargins.left + fixedMargins.right
        }
        SequentialAnimation {
            id: anim
            // `running` is set to false before the animation actually stops.
            // We need an extra property to see if it's actually still running so
            // the fill doesn't go out of bounds when `running` switches to false.
            property bool actuallyRunning: barFill.visibleIndeterminate
            loops: Animation.Infinite

            alwaysRunToEnd: true
            running: barFill.visibleIndeterminate

            NumberAnimation {
                target: barFill
                property: "x"
                to: !control.mirrored ? control.availableWidth - barFill.indeterminateWidth : 0
                duration: PlasmaCore.Units.humanMoment/2
                easing.type: Easing.InOutSine
            }
            NumberAnimation {
                target: barFill
                property: "x"
                to: control.mirrored ? control.availableWidth - barFill.indeterminateWidth : 0
                duration: PlasmaCore.Units.humanMoment/2
                easing.type: Easing.InOutSine
            }
            onStarted: actuallyRunning = true
            onStopped: actuallyRunning = false
        }
    }

    background: PlasmaCore.FrameSvgItem {
        implicitWidth: PlasmaCore.Units.gridUnit * 8
        implicitHeight: barSvg.elementSize("hint-bar-size").height || fixedMargins.top + fixedMargins.bottom
        imagePath: "widgets/bar_meter_horizontal"
        prefix: "bar-inactive"
        colorGroup: PlasmaCore.ColorScope.colorGroup
    }
}
