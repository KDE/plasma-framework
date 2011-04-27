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

import QtQuick 1.0
import org.kde.plasma.core 0.1 as PlasmaCore

Item {
    id: root

    // Common API
    property bool running: false

    // Plasma API
    property alias text: label.text
    property QtObject theme: PlasmaCore.Theme { }

    width: 52; height: 52

    PlasmaCore.SvgItem {
        id: widget

        anchors.centerIn: parent
        width: root.width
        height: root.height
        smooth: true
        svg: PlasmaCore.Svg { imagePath: ("widgets/busywidget") }

        RotationAnimation on rotation {
            from: 0
            to: 360
            target: widget
            duration: 1500
            running: root.running
            loops: Animation.Infinite
        }
    }

    Text {
        id: label

        anchors {
            verticalCenter: root.verticalCenter
            horizontalCenter: root.horizontalCenter
        }
        color: theme.textColor
    }
}
