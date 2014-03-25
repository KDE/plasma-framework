/*
 *  Copyright 2014 Sebastian Kügler <sebas@kde.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  2.010-1301, USA.
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

    spacing: units.gridUnit

    id: themePage
    anchors {
        margins: units.largeSpacing
    }

    Row {
        PlasmaComponents.CheckBox {
            id: boxesCheck
            text: "Show Boxes"
        }
        PlasmaComponents.CheckBox {
            id: fontCheck
            text: checked ? "Oxygen-Sans" : "Comme"
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
        font.family: "Oxygen-Sans"
    }

    FontGizmo {
        id: giz2
        font.family: "Comme"
    }

    FontGizmo {
        id: giz3
        font.family: "Oxygen Mono"

    }

    Rectangle {
        width: parent.width
        height: units.gridUnit * 6

        border.width: boxesCheck.checked ? 1 : 0
        border.color: "red"

        Rectangle {
            id: img
            color: "orange"
            height: units.gridUnit * 2
            width: height
            anchors {
                top: parent.top
                left: parent.left
            }
        }

        PlasmaComponents.Label {

            font.family: fontCheck.text
            font.pointSize: 24
            verticalAlignment: Text.AlignTop

            text: "That's a Headline"

            //height: paintedHeight
            height: units.gridUnit
            font.pixelSize: height
            anchors {
                left: img.right
                leftMargin: units.gridUnit
                top: img.top
            }
        }
    }

}
