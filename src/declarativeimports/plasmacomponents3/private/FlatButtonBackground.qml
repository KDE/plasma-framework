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

    property real leftMargin: surfaceHover.margins.left
    property real topMargin: surfaceHover.margins.top
    property real rightMargin: surfaceHover.margins.right
    property real bottomMargin: surfaceHover.margins.bottom
    property string usedPrefix: surfaceHover.usedPrefix

    ButtonShadow {
        anchors.fill: parent
        showShadow: !(root.checked || root.pressed) && root.usedPrefix === "normal"
    }

    ButtonFocus {
        anchors.fill: parent
        showFocus: root.focused && !root.pressed
        flat: true
    }

    // TODO: Maybe add a way to customize the look of normal state flat buttons with "toolbutton-normal"? 
    // TODO: Maybe add a way to customize the background of focused flat buttons with "toolbutton-focus-background"? 
    // TODO KF6: "flat" would be a more logical name than "toolbutton" since toolbuttons can be non-flat.
    PlasmaCore.FrameSvgItem {
        id: surfaceHover
        anchors.fill: parent
        imagePath: "widgets/button"
        /* TODO KF6: making "toolbutton-hover" margins work like "hover"
         * and using "hover" as a fallback would make more sense.
         * If that is done, make ButtonHover handle flat button hover effects.
         */
        // The fallback is "normal" to match PC2 behavior. Some 3rd party themes depend on this.
        prefix: ["toolbutton-hover", "normal"]
    }

    PlasmaCore.FrameSvgItem {
        id: surfacePressed
        anchors.fill: parent
        imagePath: "widgets/button"
        prefix: ["toolbutton-pressed", "pressed"]
        opacity: 0
    }

    state: {
        if (root.checked || root.pressed) {
            return "pressed";
        } else if (root.hovered) {
            return "hovered";
        } else {
            return "normal";
        }
    }

    states: [
        State {
            name: "normal"
            PropertyChanges {
                target: root
                leftMargin: surfaceHover.margins.left
                topMargin: surfaceHover.margins.top
                rightMargin: surfaceHover.margins.right
                bottomMargin: surfaceHover.margins.bottom
                usedPrefix: surfaceHover.usedPrefix
            }
            PropertyChanges {
                target: surfaceHover
                visible: false
            }
            PropertyChanges {
                target: surfacePressed
                opacity: 0
            }
        },
        State {
            name: "hovered"
            PropertyChanges {
                target: root
                leftMargin: surfaceHover.margins.left
                topMargin: surfaceHover.margins.top
                rightMargin: surfaceHover.margins.right
                bottomMargin: surfaceHover.margins.bottom
                usedPrefix: surfaceHover.usedPrefix
            }
            PropertyChanges {
                target: surfaceHover
                visible: true
            }
            PropertyChanges {
                target: surfacePressed
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
                target: surfaceHover
                visible: false
            }
            PropertyChanges {
                target: surfacePressed
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
                    targets: [surfaceHover]
                    property: "visible"
                    value: false
                }
                NumberAnimation {
                    property: "opacity"
                    duration: PlasmaCore.Units.shortDuration
                    easing.type: Easing.OutQuad
                }
                PropertyAction {
                    targets: [surfacePressed]
                    property: "visible"
                    value: false
                }
            }
        },
        Transition {
            from: "*"
            to: "hovered"
            SequentialAnimation {
                PropertyAction {
                    target: surfaceHover
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
                    target: surfacePressed
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
                    target: surfaceHover
                    property: "visible"
                    value: false
                }
            }
        }
    ]
}
