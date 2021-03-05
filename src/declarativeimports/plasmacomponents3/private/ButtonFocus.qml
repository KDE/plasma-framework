/*
 * SPDX-FileCopyrightText: 2020 Noah Davis <noahadvs@gmail.com>
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick 2.12
import org.kde.plasma.core 2.0 as PlasmaCore

Item {
    id: root
    property bool showFocus: false
    property bool flat: false

    property alias enabledBorders: focusEffect.enabledBorders

    PlasmaCore.FrameSvgItem {
        id: focusEffect
        anchors {
            fill: parent
            leftMargin: -margins.left
            topMargin: -margins.top
            rightMargin: -margins.right
            bottomMargin: -margins.bottom
        }
        opacity: 0
        imagePath: "widgets/button"
        prefix: flat ? ["toolbutton-focus", "focus"] : "focus"
    }

    state: root.showFocus ? "focused" : "hidden"

    states: [
        State {
            name: "focused"
            PropertyChanges {
                target: focusEffect
                opacity: 1
            }
        },
        State {
            name: "hidden"
            PropertyChanges {
                target: focusEffect
                opacity: 0
            }
        }
    ]

    transitions: [
        Transition {
            from: "*"
            to: "hidden"
            SequentialAnimation {
                OpacityAnimator {
                    duration: PlasmaCore.Units.shortDuration
                    easing.type: Easing.OutQuad
                }
                PropertyAction {
                    target: root
                    property: "visible"
                    value: false
                }
            }
        },
        Transition {
            from: "*"
            to: "focused"
            SequentialAnimation {
                PropertyAction {
                    target: root
                    property: "visible"
                    value: true
                }
                OpacityAnimator {
                    duration: PlasmaCore.Units.shortDuration
                    easing.type: Easing.OutQuad
                }
            }
        }
    ]
}
