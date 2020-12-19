/*
    SPDX-FileCopyrightText: 2014 Kai Uwe Broulik <kde@privat.broulik.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.2
import QtQuick.Controls.Styles 1.2

import org.kde.plasma.core 2.0 as PlasmaCore

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
            duration: 2000
            running: control.running && control.visible && control.opacity > 0;
            loops: Animation.Infinite
        }
    }
}
