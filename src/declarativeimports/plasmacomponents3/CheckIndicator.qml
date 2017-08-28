/*
 *   Copyright 2016 Marco Martin <mart@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Library General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

import QtQuick 2.6
import org.kde.plasma.core 2.0 as PlasmaCore
import "private" as Private

PlasmaCore.FrameSvgItem {
    id: root
    property Item control
    imagePath: "widgets/button"
    prefix: "normal"
    implicitWidth: units.gridUnit
    implicitHeight: units.gridUnit
    opacity: control.enabled ? 1 : 0.6

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
                duration: units.longDuration
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
