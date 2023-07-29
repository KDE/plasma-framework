/*
    SPDX-FileCopyrightText: 2011 Daker Fernandes Pinheiro <dakerfp@gmail.com>
    SPDX-FileCopyrightText: 2011 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.1
import org.kde.ksvg 1.0 as KSvg
import org.kde.kirigami 2 as Kirigami

Item {
    id: main
    state: parent.state

    KSvg.Svg {
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

    KSvg.FrameSvgItem {
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
                duration: Kirigami.Units.longDuration
                easing.type: Easing.OutQuad
            }
        }
    ]
}
