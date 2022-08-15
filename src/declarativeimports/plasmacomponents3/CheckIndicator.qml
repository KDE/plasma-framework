/*
    SPDX-FileCopyrightText: 2016 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2022 ivan (@ratijas) tkachenko <me@ratijas.tk>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.15
import QtQuick.Templates @QQC2_VERSION@ as T

import org.kde.plasma.core 2.1 as PlasmaCore
import "private" as Private

PlasmaCore.FrameSvgItem {
    id: root

    required property T.AbstractButton control

    imagePath: "widgets/button"
    prefix: "normal"
    implicitWidth: PlasmaCore.Units.iconSizes.small
    implicitHeight: PlasmaCore.Units.iconSizes.small
    opacity: control.enabled ? 1 : 0.6

    Private.ButtonShadow {
        anchors.fill: parent
        showShadow: !control.pressed
    }

    PlasmaCore.SvgItem {
        anchors.fill: parent
        svg: PlasmaCore.Svg {
            id: checkmarkSvg
            imagePath: "widgets/checkmarks"
        }
        elementId: "checkbox"
        opacity: {
            if (control instanceof T.CheckBox) {
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
        showHover: control.hovered
    }
}
