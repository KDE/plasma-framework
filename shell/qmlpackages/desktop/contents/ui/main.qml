/*
 *  Copyright 2012 Marco Martin <mart@kde.org>
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
//import org.kde.plasma 2.0

import org.kde.plasma.core 0.1 as PlasmaCore

Rectangle {
    color: "darkblue"
    width: 640
    height: 480
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

        Text {
            id: txt
            text: "Click or Drag";
            anchors.centerIn: parent
        }
    }

//     Svg {
//         x: 200
//         y: 200
//         width: 100
//         height: 100
//         MouseArea {
//             anchors.fill: parent
//             drag.target: parent
//         }
//     }
}