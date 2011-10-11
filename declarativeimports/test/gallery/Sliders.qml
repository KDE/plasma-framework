/*
*   Copyright (C) 2011 by Daker Fernandes Pinheiro <dakerfp@gmail.com>
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

import QtQuick 1.0
import org.kde.plasma.components 0.1 as PlasmaComponents

Column {
    spacing: 20

    Text {
        font.pixelSize: 20
        text: "Slider"
    }

    PlasmaComponents.Highlight {
        width: 300
        height: 400
        Column {
            anchors {
                fill: parent
            }
            spacing: 10

            Text { text: "Color Selector"; font.pixelSize: 20 }

            Text { text: "Red" }

            PlasmaComponents.Slider {
                id: redSlider
                height: 20
                width: 255
                orientation: Qt.Horizontal
                minimumValue: 0
                maximumValue: 255
                stepSize: 10
                animated: true
                Keys.onTabPressed: greenSlider.forceActiveFocus()
            }

            Text { text: "Green" }

            PlasmaComponents.Slider {
                id: greenSlider
                height: 20
                width: 255
                orientation: Qt.Horizontal
                minimumValue: 0
                maximumValue: 255
                stepSize: 10
                animated: true
                Keys.onTabPressed: blueSlider.forceActiveFocus()
            }

            Text { text: "Blue" }

            PlasmaComponents.Slider {
                id: blueSlider
                height: 20
                width: 255
                orientation: Qt.Horizontal
                minimumValue: 0
                maximumValue: 255
                stepSize: 10
                animated: true
                Keys.onTabPressed: redSlider.forceActiveFocus()
            }

            Rectangle {
                anchors.horizontalCenter: parent.horizontalCenter
                width: parent.width / 2
                height: width
                color: Qt.rgba(redSlider.value / 255, greenSlider.value / 255, blueSlider.value / 255, 1)
            }
        }
    }

    Text { text: "Disabled Horizontal Slider" }

    PlasmaComponents.Slider {
        id: horizontalSlider
        width: 140
        height: 20
        animated: true
        enabled: false
    }

    Text { text: "Inverted Horizontal Slider" }

    PlasmaComponents.Slider {
        id: invHorizontalSlider
        width: 140
        height: 20
        inverted: true
        animated: true
        enabled: true
    }

    Text { text: "Vertical Slider" }

    Row {
        spacing: 30
        PlasmaComponents.Slider {
            id: verticalSlider
            width: 20
            height: 140
            orientation: Qt.Vertical
            minimumValue: 10
            maximumValue: 1000
            stepSize: 50
            inverted: true
            animated: true
        }
        Text { text: verticalSlider.value }
    }

}
