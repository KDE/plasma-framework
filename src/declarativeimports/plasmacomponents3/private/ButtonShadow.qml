/*
    SPDX-FileCopyrightText: 2011 Daker Fernandes Pinheiro <dakerfp@gmail.com>
    SPDX-FileCopyrightText: 2011 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2020 Noah Davis <noahadvs@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

/**Documented API
Inherits:
        Item

Imports:
        QtQuick 2.1
        org.kde.plasma.core

Description:
TODO i need more info here


Properties:
**/

import QtQuick 2.12
import org.kde.plasma.core 2.0 as PlasmaCore

Item {
    id: root
    property bool showShadow: true

    property alias enabledBorders: shadowEffect.enabledBorders

    PlasmaCore.FrameSvgItem {
        id: shadowEffect
        anchors {
            fill: parent
            leftMargin: -margins.left
            topMargin: -margins.top
            rightMargin: -margins.right
            bottomMargin: -margins.bottom
        }
        imagePath: "widgets/button"
        prefix: "shadow"
    }

    state: showShadow ? "shown" : "hidden"

    states: [
        State {
            name: "shown"
            PropertyChanges {
                target: shadowEffect
                opacity: 1
            }
        },
        State {
            name: "hidden"
            PropertyChanges {
                target: shadowEffect
                opacity: 0
            }
        }
    ]
    transitions: [
        Transition {
            from: "*"
            to: "shown"
            SequentialAnimation {
                PropertyAction {
                    target: root
                    property: "visible"
                    value: true
                }
                OpacityAnimator {
                    duration: units.veryShortDuration
                    easing.type: Easing.OutQuad
                }
            }
        },
        Transition {
            from: "*"
            to: "hidden"
            SequentialAnimation {
                OpacityAnimator {
                    // Same duration as pressed and flat background hovered state animations
                    duration: units.veryShortDuration
                    easing.type: Easing.OutQuad
                }
                PropertyAction {
                    target: root
                    property: "visible"
                    value: false
                }
            }
        }
    ]
}
