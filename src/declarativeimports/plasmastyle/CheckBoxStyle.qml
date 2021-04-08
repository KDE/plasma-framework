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
QtQuickControlStyle.CheckBoxStyle {
    id: checkboxStyle

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
    background: Item {}

    spacing: PlasmaCore.Units.smallSpacing

    indicator: PlasmaCore.FrameSvgItem {
        imagePath: "widgets/button"
        prefix: "normal"
        width: PlasmaCore.Units.iconSizes.small
        height: width
        opacity: control.enabled ? 1 : 0.6

        PlasmaCore.SvgItem {
            svg: PlasmaCore.Svg {
                id: checkmarkSvg
                imagePath: "widgets/checkmarks"
            }
            elementId: "checkbox"
            opacity: {
                switch (control.checkedState) {
                case Qt.Checked:
                    return 1;
                case Qt.PartiallyChecked:
                    return 0.5;
                default:
                    return 0;
                }
            }
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
        Private.ButtonShadow {
            z: -1
            anchors.fill: parent
            state: control.activeFocus ? "focus" : (control.hovered ? "hover" : "shadow")
        }
    }
}
