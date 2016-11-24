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

PlasmaCore.SvgItem {
    property Item control
    svg: PlasmaCore.Svg {
        id: buttonSvg
        imagePath: "widgets/actionbutton"
    }
    elementId: "normal"
    opacity: control.enabled ? 1 : 0.6

    implicitWidth: implicitHeight
    implicitHeight: units.gridUnit

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
                duration: units.longDuration
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
