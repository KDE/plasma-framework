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

    PlasmaComponents.Slider {
        id: slider1
        width: 140
        height: 20
        animated: true
        Component.onCompleted: {
            forceActiveFocus();
        }
    }

    Text {
        text: slider1.value
    }

    PlasmaComponents.Slider {
        id: slider2
        width: 20
        height: 140
        orientation: Qt.Vertical
        minimumValue: 10
        maximumValue: 1000
        stepSize: 50
        inverted: true
        animated: true
    }

    Text {
        text: slider2.value
    }
}
