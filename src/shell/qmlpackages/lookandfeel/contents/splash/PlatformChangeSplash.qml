/*   vim:set foldmethod=marker:
 *
 *   Copyright (C) 2013 Ivan Cukic <ivan.cukic(at)kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License version 2,
 *   or (at your option) any later version, as published by the Free
 *   Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

import QtQuick 2.0
import QtGraphicalEffects 1.0

Rectangle {
    id: root

    color: "#1e1e1e"

    Image {
        id: imageGear

        width:  64
        height: 64

        anchors.centerIn: parent
        source: "images/kdegear.png"

        NumberAnimation {
            id: animateRotation

            target: imageGear
            properties: "rotation"

            from: 0
            to: 360
            duration: 5000

            loops: Animation.Infinite
            running: true
        }
    }

    Rectangle {
        id: rectMask
        color: root.color

        anchors {
            top:    imageGear.top
            bottom: imageGear.bottom
            right:  root.right
            left:   imageGear.horizontalCenter
        }
    }

    Rectangle {
        id: rectLine

        width: 1
        height: imageGear.height * 1.5
        anchors.centerIn: imageGear

        gradient: Gradient {
            GradientStop { position: 0.00; color: root.color }
            GradientStop { position: 0.33; color: "#ffffff"  }
            GradientStop { position: 0.67; color: "#ffffff"  }
            GradientStop { position: 1.00; color: root.color }
        }
    }

    Text {
        id: textTitle

        anchors {
            left: imageGear.horizontalCenter
            leftMargin: 8
            verticalCenter:  imageGear.verticalCenter
        }

        text: qsTr("Loading...")
        color: "white"

        horizontalAlignment: Text.AlignHCenter
        font.pixelSize: 14
    }
}
