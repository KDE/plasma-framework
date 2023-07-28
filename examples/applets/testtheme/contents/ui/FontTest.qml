/*
    SPDX-FileCopyrightText: 2014 Sebastian Kügler <sebas@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.1

import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.kquickcontrolsaddons 2.0 as KQuickControlsAddons
import org.kde.kirigami 2.20 as Kirigami

// IconTab

Column {

    width: 400
    height: 400

    spacing: Kirigami.Units.gridUnit

    id: themePage
    anchors {
        margins: Kirigami.Units.gridUnit
    }

    Row {
        spacing: Kirigami.Units.largeSpacing
        PlasmaComponents.CheckBox {
            id: boxesCheck
            text: "Show Boxes"
        }
        PlasmaComponents.CheckBox {
            id: fontCheck
            text: checked ? "Oxygen-Sans" : "Comme"
        }
        PlasmaComponents.CheckBox {
            id: lightCheck
            //tristate: true
            text: checked ? "Light" : "Regular"
        }
        PlasmaComponents.CheckBox {
            id: paintedHeightCheck
            text: "Painted Height"
            checked: true
            visible: false
        }
    }

    FontGizmo {
        id: giz1
        font: Kirigami.Theme.smallFont
        //font.family: "Oxygen-Sans"
    }

    FontGizmo {
        id: giz2
        font.pixelSize: Kirigami.Units.gridUnit * 1
    }

    FontGizmo {
        id: giz3
        font.pixelSize: Kirigami.Units.gridUnit * 2
    }

    FontGizmo {
        id: giz4
        font.pixelSize: Kirigami.Units.gridUnit * 2
        text: "AlignTop"
        verticalAlignment: Text.AlignTop
    }

    FontGizmo {
        id: giz5
        font.pixelSize: Kirigami.Units.gridUnit * 2
        text: "AlignVCenter"
        verticalAlignment: Text.AlignVCenter
    }

//     FontGizmo {
//         id: giz3
//         font.family: "Oxygen Mono"
//
//     }
//
    Rectangle {
        width: parent.width
        height: Kirigami.Units.gridUnit * 6

        color: "transparent"
        border.width: boxesCheck.checked ? 1 : 0
        border.color: "red"

        Rectangle {
            id: img
            color: "orange"
            height: Kirigami.Units.gridUnit * 2
            width: height
            anchors {
                top: parent.top
                left: parent.left
            }
        }

        Kirigami.Heading {

            font.weight: lightCheck.checked ? Font.Light : Font.Normal
            font.family: fontCheck.text
            font.pointSize: 24
            verticalAlignment: Text.AlignTop

            text: "That's a Headline"

            //height: paintedHeight
            height: Kirigami.Units.gridUnit
            font.pixelSize: height
            anchors {
                left: img.right
                leftMargin: Kirigami.Units.largeSpacing
                top: img.top
            }
        }
    }

}
