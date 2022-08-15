/*
    SPDX-FileCopyrightText: 2016 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2022 ivan (@ratijas) tkachenko <me@ratijas.tk>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.15
import QtQuick.Templates @QQC2_VERSION@ as T
import org.kde.plasma.core 2.1 as PlasmaCore
import "private" as Private

Item {
    id: root

    required property T.AbstractButton control

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
        opacity: root.control.checked ? 1 : 0
        Behavior on opacity {
            OpacityAnimator {
                duration: PlasmaCore.Units.longDuration
                easing.type: Easing.InOutQuad
            }
        }
    }
    PlasmaCore.SvgItem {
        x: root.control.mirrored ? (root.control.checked ? 0 : parent.width - width) : (root.control.checked ? parent.width - width : 0)
        anchors.verticalCenter: parent.verticalCenter
        svg: PlasmaCore.Svg {
            id: buttonSvg
            imagePath: "widgets/actionbutton"
        }
        elementId: "normal"

        implicitWidth: PlasmaCore.Units.gridUnit
        implicitHeight: PlasmaCore.Units.gridUnit

        Private.RoundShadow {
            anchors.fill: parent
            z: -1
            state: root.control.activeFocus ? "focus" : (root.control.hovered ? "hover" : "shadow")
        }
        Behavior on x {
            XAnimator {
                duration: PlasmaCore.Units.longDuration
                easing.type: Easing.InOutQuad
            }
        }
    }
}

