/*
    SPDX-FileCopyrightText: 2016 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.6
import org.kde.plasma.core 2.0 as PlasmaCore
import "private" as Private

Item {
    property Item control
    implicitWidth: Math.round(PlasmaCore.Units.gridUnit * 1.5)
    implicitHeight : PlasmaCore.Units.gridUnit

    opacity: control.enabled ? 1 : 0.6

    PlasmaCore.FrameSvgItem {
        anchors {
            left: parent.left
            right: parent.right
            verticalCenter: parent.verticalCenter
        }
        imagePath: "widgets/bar_meter_horizontal"
        prefix: "bar-inactive"
    }
    PlasmaCore.FrameSvgItem {
        anchors {
            left: parent.left
            right: parent.right
            verticalCenter: parent.verticalCenter
        }
        imagePath: "widgets/bar_meter_horizontal"
        prefix: "bar-active"
        opacity: control.checked ? 1 : 0
        Behavior on opacity {
            OpacityAnimator {
                duration: PlasmaCore.Units.longDuration
                easing.type: Easing.InOutQuad
            }
        }
    }
    PlasmaCore.SvgItem {
        x: control.mirrored ? (control.checked ? 0 : parent.width - width) : (control.checked ? parent.width - width : 0)
        anchors.verticalCenter: parent.verticalCenter
        svg: PlasmaCore.Svg {
            id: buttonSvg
            imagePath: "widgets/actionbutton"
        }
        elementId: "normal"

        implicitWidth: implicitHeight
        implicitHeight: PlasmaCore.Units.gridUnit

        Private.RoundShadow {
            anchors.fill: parent
            z: -1
            state: control.activeFocus ? "focus" : (control.hovered ? "hover" : "shadow")
        }
        Behavior on x {
            XAnimator {
                duration: PlasmaCore.Units.longDuration
                easing.type: Easing.InOutQuad
            }
        }
    }
}

