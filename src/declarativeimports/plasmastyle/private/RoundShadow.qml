/*
    SPDX-FileCopyrightText: 2011 Daker Fernandes Pinheiro <dakerfp@gmail.com>
    SPDX-FileCopyrightText: 2011 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

/**Documented API
Inherits:
        Item

Imports:
        QtQuick 2.1
        org.kde.plasma.core

Description:
        It is a simple Radio button which is using the plasma theme.
        TODO Do we need more info?

Properties:
        TODO needs more info??
**/

import QtQuick 2.1
import org.kde.plasma.core 2.0 as PlasmaCore

Item {
    id: main
    state: parent.state
    property alias imagePath: shadowSvg.imagePath
    property string hoverElement: "hover"
    property string focusElement: "focus"
    property alias shadowElement: shadow.elementId

    //used to tell apart this implementation with the touch components one
    property bool hasOverState: true

    PlasmaCore.Svg {
        id: shadowSvg
        imagePath: "widgets/actionbutton"
    }

    PlasmaCore.SvgItem {
        id: hover
        svg: shadowSvg
        elementId: "hover"

        anchors.fill: parent

        opacity: 0
    }

    PlasmaCore.SvgItem {
        id: shadow
        svg: shadowSvg
        elementId: "shadow"

        anchors.fill: parent
    }

    states: [
        State {
            name: "shadow"
            PropertyChanges {
                target: shadow
                opacity: 1
            }
            PropertyChanges {
                target: hover
                opacity: 0
                elementId: hoverElement
            }
        },
        State {
            name: "hover"
            PropertyChanges {
                target: shadow
                opacity: 0
            }
            PropertyChanges {
                target: hover
                opacity: 1
                elementId: hoverElement
            }
        },
        State {
            name: "focus"
            PropertyChanges {
                target: shadow
                opacity: 0
            }
            PropertyChanges {
                target: hover
                opacity: 1
                elementId: focusElement
            }
        },
        State {
            name: "hidden"
            PropertyChanges {
                target: shadow
                opacity: 0
            }
            PropertyChanges {
                target: hover
                opacity: 0
                elementId: hoverElement
            }
        }
    ]

    transitions: [
        Transition {
            PropertyAnimation {
                properties: "opacity"
                duration: PlasmaCore.Units.veryShortDuration
                easing.type: Easing.OutQuad
            }
        }
    ]
}
