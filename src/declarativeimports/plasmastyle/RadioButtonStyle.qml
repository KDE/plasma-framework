/*
    SPDX-FileCopyrightText: 2014 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.0
import QtQuick.Controls.Styles 1.1 as QtQuickControlStyle

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import "private" as Private

/**
 * \internal
 */
QtQuickControlStyle.RadioButtonStyle {
    id: radiobuttonStyle

    label: PlasmaComponents.Label {
        text: control.text
        Rectangle {
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.bottom
            height: 1 * PlasmaCore.Units.devicePixelRatio
            color: PlasmaCore.Theme.highlightColor
            visible: control.activeFocus
        }
    }
    //Not needed?
    background: Item {}

    spacing: PlasmaCore.Units.smallSpacing

    indicator: PlasmaCore.SvgItem {
        svg: PlasmaCore.Svg {
            id: buttonSvg
            imagePath: "widgets/actionbutton"
        }
        elementId: "normal"
        opacity: control.enabled ? 1 : 0.6

        width: PlasmaCore.Units.iconSizes.small
        height: width

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
}
