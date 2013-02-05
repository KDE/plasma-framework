/*
 *  Copyright 2013 Marco Martin <mart@kde.org>
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

import QtQuick 2.0

import org.kde.plasma.core 0.1 as PlasmaCore

Rectangle {
    id: root
    color: "darkblue"
    width: 640
    height: 480

    Connections {
        target: plasmoid
        onAppletAdded: {
            print("Applet added: " + applet)
            applet.parent = root
            applet.visible = true
        }
    }

    PlasmaCore.Svg {
        id: actionssvg
        imagePath: "widgets/configuration-icons"
    }

    PlasmaCore.FrameSvgItem {
        id: frame
        x: 50
        y: 50
        width: txt.paintedWidth + 80
        height: txt.paintedHeight + 80 * 2
        property int small: 100
        property int large: parent.width - width - 150
        imagePath: "widgets/background"
        MouseArea {
            anchors.fill: parent
            drag.target: parent
            onClicked: {
                var s = (frame.x == frame.large) ? frame.small : frame.large;
                frame.x = s
                frame.height = s
            }
        }
        Behavior on x { PropertyAnimation { easing.type: Easing.OutElastic; duration: 800 } }
        //Behavior on y { PropertyAnimation { easing.type: Easing.OutElastic; duration: 800 } }
        Behavior on height { PropertyAnimation { easing.type: Easing.InOutDouble; duration: 300 } }

        Column {
            anchors.centerIn: parent
            Text {
                id: txt
                text: "Click or Drag";
            }
            Text {
                text: plasmoid
            }
        }
        PlasmaCore.SvgItem {
            svg: actionssvg
            elementId: "rotate"
            width: 16
            height: width
            anchors.margins: frame.margins.left
            anchors {
                top: parent.top
                left: parent.left
            }
            //Rectangle { color: "white"; opacity: 0.2; anchors.fill: parent; }
        }
    }
//     PlasmaCore.SvgItem {
//         svg: actionssvg
//         elementId: "rotate"
//         width: 128
//         height: width
//     }
    PlasmaCore.IconItem {
        source: "accessories-dictionary"
        x: 50
        y: 350
        width: 48
        height: 48
        Rectangle { color: "white"; opacity: 0.2; anchors.fill: parent; }
    }

    Component.onCompleted: {
        print("Test Containment loaded")
        print(plasmoid)
        for (var i in plasmoid) {
            print(i+" "+plasmoid[i])
        }
    }
}