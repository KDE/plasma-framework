/*
    SPDX-FileCopyrightText: 2011 Daker Fernandes Pinheiro <dakerfp@gmail.com>
    SPDX-FileCopyrightText: 2011 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.1
import org.kde.plasma.core 2.0 as PlasmaCore

Item {
    id: main
    state: parent.state

    PlasmaCore.Svg {
        id: lineEditSvg
        property bool hasFocusFrame: lineEditSvg.hasElement("focusframe-center")
        property bool hasFocusOverBase: lineEditSvg.hasElement("hint-focus-over-base")
        imagePath: "widgets/lineedit"
        onRepaintNeeded: {
            if (hasFocusOverBase) {
                main.z = 800
            } else {
                main.z = 0
            }
        }
        Component.onCompleted: {
            if (hasFocusOverBase) {
                main.z = 800
            } else {
                main.z = 0
            }
        }
    }

    PlasmaCore.FrameSvgItem {
        id: hover

        anchors {
            fill: parent
            leftMargin: -margins.left
            topMargin: -margins.top
            rightMargin: -margins.right
            bottomMargin: -margins.bottom
        }
        opacity: 0
        visible: opacity > 0
        imagePath: "widgets/lineedit"
        prefix: "hover"
    }

    states: [
        State {
            name: "hover"
            PropertyChanges {
                target: hover
                opacity: 1
                prefix: "hover"
            }
        },
        State {
            name: "focus"
            PropertyChanges {
                target: hover
                opacity: 1
                prefix: "focus"
            }
        },
        State {
            name: "focusframe"
            PropertyChanges {
                target: hover
                opacity: 1
                prefix: lineEditSvg.hasFocusFrame ? "focusframe" : "focus"
            }
        },
        State {
            name: "hidden"
            PropertyChanges {
                target: hover
                opacity: 0
                prefix: "hover"
            }
        }
    ]

    transitions: [
        Transition {
            PropertyAnimation {
                properties: "opacity"
                duration: PlasmaCore.Units.longDuration
                easing.type: Easing.OutQuad
            }
        }
    ]
}
