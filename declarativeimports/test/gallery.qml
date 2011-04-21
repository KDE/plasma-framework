/*
 *   Copyright (C) 2010 by Anselmo Lacerda Silveira de Melo <anselmolsm@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

import Qt 4.7
import org.kde.plasma.components 0.1 as PlasmaComponents
import org.kde.plasma.core 0.1 as PlasmaCore
import org.kde.qtextracomponents 0.1 as QtExtraComponents

Rectangle {
    id: test
    width: 400
    height: 500
    color: "tomato"

    Column {
        x: 10
        anchors.fill: parent
        spacing: 5

        PlasmaComponents.FlashingLabel {
            id: label
            font: "Times"
        }

        Row {
            id: busyRow

            Repeater {
                model: 8
                PlasmaComponents.BusyWidget {
                    width: 50
                    height: 50
                }
            }
        }

        PlasmaComponents.PushButton {
            text: "Ok"
            onClicked: {print("Clicked!"); scrollBar.value=35}
        }
        PlasmaComponents.ScrollBar {
            id: scrollBar
        }
        PlasmaComponents.ScrollBar {
            id: scrollBarV
            orientation: Qt.Vertical
        }
        QtExtraComponents.QPixmapItem {
            width:200
            height:100
            id: pixmapTest
        }
    }

    PlasmaCore.Svg {
        id: svgTest
        imagePath: "widgets/clock"
    }
    Component.onCompleted: {
        label.flash("I am a FlashingLabel!!!");
        pixmapTest.pixmap = svgTest.pixmap();
    }
}
