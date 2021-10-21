/*
 * SPDX-FileCopyrightText: 2020 Noah Davis <noahadvs@gmail.com>
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick 2.6
import org.kde.plasma.core 2.0 as PlasmaCore

Item {
    id: root
    required property bool hovered
    required property bool pressed
    required property bool checked
    required property bool focused

    property real leftMargin: surfaceNormal.margins.left
    property real topMargin: surfaceNormal.margins.top
    property real rightMargin: surfaceNormal.margins.right
    property real bottomMargin: surfaceNormal.margins.bottom
    property string usedPrefix: surfaceNormal.usedPrefix

    ButtonShadow {
        anchors.fill: parent
        showShadow: enabled && !root.checked && !root.pressed
    }

    PlasmaCore.FrameSvgItem {
        id: surfaceNormal
        anchors.fill: parent
        imagePath: "widgets/button"
        prefix:  "normal"
        opacity: 1
    }

    // Intentionally lower than surfacePressed, surfaceFocused
    ButtonFocus {
        anchors.fill: parent
        showFocus: root.focused && !root.pressed
    }

    // Intentionally lower than surfacePressed and surfaceFocused
    ButtonHover {
        anchors.fill: parent
        showHover: root.hovered && !root.pressed
    }

    PlasmaCore.FrameSvgItem {
        id: surfacePressed
        anchors.fill: parent
        imagePath: "widgets/button"
        prefix: "pressed"
        opacity: 0
    }

    PlasmaCore.FrameSvgItem {
        id: surfaceFocused
        anchors.fill: parent
        imagePath: "widgets/button"
        prefix: ["focus-background", "normal"]
        opacity: 0
    }

    state: {
        if (root.checked || root.pressed) {
            return "pressed";
        } else if (focused && surfaceFocused.usedPrefix != "normal") {
            return "focused";
        } else {
            return "normal";
        }
    }

    states: [
        State {
            name: "normal"
            PropertyChanges {
                target: root
                leftMargin: surfaceNormal.margins.left
                topMargin: surfaceNormal.margins.top
                rightMargin: surfaceNormal.margins.right
                bottomMargin: surfaceNormal.margins.bottom
                usedPrefix: surfaceNormal.usedPrefix
            }
            PropertyChanges {
                target: surfaceNormal
                opacity: 1
            }
            PropertyChanges {
                target: surfacePressed
                opacity: 0
            }
            PropertyChanges {
                target: surfaceFocused
                opacity: 0
            }
        },
        State {
            name: "pressed"
            PropertyChanges {
                target: root
                leftMargin: surfacePressed.margins.left
                topMargin: surfacePressed.margins.top
                rightMargin: surfacePressed.margins.right
                bottomMargin: surfacePressed.margins.bottom
                usedPrefix: surfacePressed.usedPrefix
            }
            PropertyChanges {
                target: surfaceNormal
                opacity: 0
            }
            PropertyChanges {
                target: surfacePressed
                opacity: 1
            }
            PropertyChanges {
                target: surfaceFocused
                opacity: 0
            }
        },
        State {
            name: "focused"
            PropertyChanges {
                target: root
                leftMargin: surfaceFocused.margins.left
                topMargin: surfaceFocused.margins.top
                rightMargin: surfaceFocused.margins.right
                bottomMargin: surfaceFocused.margins.bottom
                usedPrefix: surfaceFocused.usedPrefix
            }
            PropertyChanges {
                target: surfaceNormal
                opacity: 0
            }
            PropertyChanges {
                target: surfacePressed
                opacity: 0
            }
            PropertyChanges {
                target: surfaceFocused
                opacity: 1
            }
        }
    ]

    transitions: [
        /* FIXME: For some reason, one of the surfaces will stop working when
         * OpacityAnimator is used.
         * OpacityAnimator would be more efficient.
         */
        Transition {
            from: "*"
            to: "normal"
            SequentialAnimation {
                PropertyAction {
                    target: surfaceNormal
                    property: "visible"
                    value: true
                }
                NumberAnimation {
                    property: "opacity"
                    duration: PlasmaCore.Units.shortDuration
                    easing.type: Easing.OutQuad
                }
                PropertyAction {
                    targets: [surfacePressed, surfaceFocused]
                    property: "visible"
                    value: false
                }
            }
        },
        Transition {
            from: "*"
            to: "pressed"
            SequentialAnimation {
                PropertyAction {
                    target: surfacePressed
                    property: "visible"
                    value: true
                }
                NumberAnimation {
                    property: "opacity"
                    // Using a shorter duration here makes things feel more responsive.
                    duration: PlasmaCore.Units.shortDuration/2
                    easing.type: Easing.OutQuad
                }
                PropertyAction {
                    targets: [surfaceNormal, surfaceFocused]
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
                    target: surfaceFocused
                    property: "visible"
                    value: true
                }
                NumberAnimation {
                    property: "opacity"
                    duration: PlasmaCore.Units.shortDuration
                    easing.type: Easing.OutQuad
                }
                PropertyAction {
                    targets: [surfaceNormal, surfacePressed]
                    property: "visible"
                    value: false
                }
            }
        }
    ]
}
