/*
 * SPDX-FileCopyrightText: 2020 Noah Davis <noahadvs@gmail.com>
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick 2.12
import org.kde.plasma.core 2.0 as PlasmaCore

Item {
    id: root
    property bool showHover: false
    // TODO KF6: If "toolbutton-hover" is made to behave like "hover",
    // put the logic for flat button hover effects here.

    //used to tell apart this implementation with the touch components one
    property bool hasOverState: true
    property alias enabledBorders: hoverEffect.enabledBorders

    PlasmaCore.FrameSvgItem {
        id: hoverEffect
        anchors {
            fill: parent
            leftMargin: -margins.left
            topMargin: -margins.top
            rightMargin: -margins.right
            bottomMargin: -margins.bottom
        }
        imagePath: "widgets/button"
        prefix: "hover"
    }

    state: root.showHover ? "hovered" : "hidden"

    states: [
        State {
            name: "hovered"
            PropertyChanges {
                target: hoverEffect
                visible: true
            }
        },
        State {
            name: "hidden"
            PropertyChanges {
                target: hoverEffect
                visible: false
            }
        }
    ]
}
