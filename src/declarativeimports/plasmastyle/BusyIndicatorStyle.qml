/*
    SPDX-FileCopyrightText: 2014 Kai Uwe Broulik <kde@privat.broulik.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.2
import QtQuick.Controls.Styles 1.2

import org.kde.plasma.core 2.0 as PlasmaCore

/**
 * \internal
 */
BusyIndicatorStyle {
    indicator: PlasmaCore.SvgItem {
        id: indicatorItem
        svg: PlasmaCore.Svg {
            imagePath: "widgets/busywidget"
            colorGroup: PlasmaCore.ColorScope.colorGroup
        }
        elementId: "busywidget"

        anchors.centerIn: parent
        width:  Math.min(control.width, control.height)
        height: width
        smooth: !control.running || (control.hasOwnProperty("smoothAnimation") && control.smoothAnimation)

        Connections {
            target: control
            function onRunningChanged() {
                rotationAnimator.from = rotation
                rotationAnimator.to = rotation + 360
            }
        }

        RotationAnimator on rotation {
            id: rotationAnimator
            from: 0
            to: 360
            // Not using a standard duration value because we don't want the
            // animation to spin faster or slower based on the user's animation
            // scaling preferences; it doesn't make sense in this context
            duration: 2000
            // Don't want it to animate at all if the user has disabled animations
            running: control.running && control.visible && control.opacity > 0 && PlasmaCore.Units.longDuration > 1;
            loops: Animation.Infinite
        }
    }
}
