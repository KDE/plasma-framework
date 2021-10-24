/*
    SPDX-FileCopyrightText: 2016 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.6
import org.kde.plasma.core 2.0 as PlasmaCore
import "private" as Private

PlasmaCore.FrameSvgItem {
    id: root
    property Item control
    imagePath: "widgets/button"
    prefix: "normal"
    implicitWidth: PlasmaCore.Units.iconSizes.small
    implicitHeight: implicitWidth
    opacity: control.enabled ? 1 : 0.6

    Private.ButtonShadow {
        anchors.fill: parent
        showShadow: !control.pressed
    }

    PlasmaCore.SvgItem {
        svg: PlasmaCore.Svg {
            id: checkmarkSvg
            imagePath: "widgets/checkmarks"
        }
        elementId: "checkbox"
        opacity: {
            if (typeof control.checkState !== "undefined") {
                switch (control.checkState) {
                case Qt.Checked:
                    return 1;
                case Qt.PartiallyChecked:
                    return 0.5;
                default:
                    return 0;
                }
            } else {
                return control.checked ? 1 : 0;
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

    Private.ButtonFocus {
        anchors.fill: parent
        showFocus: control.visualFocus && !control.pressed
    }

    Private.ButtonHover {
        anchors.fill: parent
        showHover: control.hovered && !control.pressed
    }
}
