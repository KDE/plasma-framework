/*
    SPDX-FileCopyrightText: 2014 Sebastian Kügler <sebas@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.1

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.extras 2.0 as PlasmaExtras
import org.kde.kquickcontrolsaddons 2.0 as KQuickControlsAddons

// IconTab

Column {

    width: 400
    height: 400

    spacing: PlasmaCore.Units.gridUnit

    id: themePage
    anchors {
        margins: PlasmaCore.Units.largeSpacing
    }

    Row {
        spacing: PlasmaCore.Units.gridUnit/2
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
        font: PlasmaCore.Theme.smallestFont
        //font.family: "Oxygen-Sans"
    }

    FontGizmo {
        id: giz2
        font.pixelSize: PlasmaCore.Units.gridUnit * 1
    }

    FontGizmo {
        id: giz3
        font.pixelSize: PlasmaCore.Units.gridUnit * 2
    }

    FontGizmo {
        id: giz4
        font.pixelSize: PlasmaCore.Units.gridUnit * 2
        text: "AlignTop"
        verticalAlignment: Text.AlignTop
    }

    FontGizmo {
        id: giz5
        font.pixelSize: PlasmaCore.Units.gridUnit * 2
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
        height: PlasmaCore.Units.gridUnit * 6

        color: "transparent"
        border.width: boxesCheck.checked ? 1 : 0
        border.color: "red"

        Rectangle {
            id: img
            color: "orange"
            height: PlasmaCore.Units.gridUnit * 2
            width: height
            anchors {
                top: parent.top
                left: parent.left
            }
        }

        PlasmaExtras.Heading {

            font.weight: lightCheck.checked ? Font.Light : Font.Normal
            font.family: fontCheck.text
            font.pointSize: 24
            verticalAlignment: Text.AlignTop

            text: "That's a Headline"

            //height: paintedHeight
            height: PlasmaCore.Units.gridUnit
            font.pixelSize: height
            anchors {
                left: img.right
                leftMargin: PlasmaCore.Units.gridUnit / 2
                top: img.top
            }
        }
    }

}
