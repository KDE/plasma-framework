/*
    SPDX-FileCopyrightText: 2016 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.6
import org.kde.plasma.core 2.0 as PlasmaCore
import "private" as Private

PlasmaCore.SvgItem {
    property Item control
    svg: PlasmaCore.Svg {
        id: buttonSvg
        imagePath: "widgets/actionbutton"
    }
    elementId: "normal"
    opacity: control.enabled ? 1 : 0.6

    implicitWidth: implicitHeight
    implicitHeight: PlasmaCore.Units.iconSizes.small

    PlasmaCore.SvgItem {
        id: checkmark
        svg: PlasmaCore.Svg {
            id: checkmarkSvg
            imagePath: "widgets/checkmarks"
        }
        elementId: "radiobutton"
        opacity: control.checked ? 1 : 0
        anchors {
            fill: parent
        }
        Behavior on opacity {
            NumberAnimation {
                duration: PlasmaCore.Units.longDuration
                easing.type: Easing.InOutQuad
            }
        }
    }
    Private.RoundShadow {
        anchors.fill: parent
        z: -1
        state: control.activeFocus ? "focus" : (control.hovered ? "hover" : "shadow")
    }
}
