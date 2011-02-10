/*
*   Copyright (C) 2010 by Artur Duque de Souza <asouzakde.org>
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
import org.kde.plasma.core 0.1 as PlasmaCore

Item {
    id: root
    width: 100; height: 100

    property string text: ""
    property string infoText: ""

    property string svg: ""
    property string icon: ""

    property bool drawBackground: true
    property int numDisplayLines: 1
    //property QColor textBackgroundColor: ""

    signal pressed(bool down)
    signal clicked()
    signal doubleClicked()
    signal activated()
    signal changed()

    PlasmaCore.Theme {
        id: theme
    }

    PlasmaCore.FrameSvgItem {
        id: background
        visible: root.drawBackground
        anchors.fill: root

        imagePath: "widgets/viewitem"
        prefix: mainMouseArea.pressed == true ? "selected" : "hover"
    }


    PlasmaCore.SvgItem {
        id: svg
        visible: root.svg != ""
        width: root.width - label.height
        height: root.height - label.height
        anchors.horizontalCenter: root.horizontalCenter
        svg: PlasmaCore.Svg { imagePath: root.svg }

        // if you need to do anything while repainting
        // do it inside this slot
        function update() {
        }
    }

    Image {
        id: icon
        visible: root.icon != ""
        width: root.width - label.height
        height: root.height - label.height
    }

    Text {
        id: label
        text: root.text
        color: theme.textColor
        anchors.top: icon.bottom
        anchors.horizontalCenter: root.horizontalCenter
    }

    MouseArea {
        id: mainMouseArea
        hoverEnabled: true
        anchors.fill: parent

        onEntered: {
        }

        onExited: {
        }
    }
}
