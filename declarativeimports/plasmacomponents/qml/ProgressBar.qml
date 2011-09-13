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
import org.kde.plasma.core 0.1 as PlasmaCore

Item {
    id: progressBar

    // Common API
    property alias minimumValue: range.minimumValue
    property alias maximumValue: range.maximumValue
    property alias value: range.value
    property alias indeterminate: indeterminateAnimation.running

    // Plasma API
    property int orientation: Qt.Horizontal

    // Convinience API
    property bool _isVertical: orientation == Qt.Vertical

    width: 100
    height: 20
    opacity: enabled ? 1.0 : 0.5

    RangeModel {
        id: range

        // default values
        minimumValue: 0.0
        maximumValue: 1.0
        value: 0

        positionAtMinimum: 0
        positionAtMaximum: contents.width
    }

    Item {
        id: contents

        width: _isVertical ? progressBar.height : progressBar.width
        height: _isVertical ? progressBar.width : progressBar.height
        rotation: _isVertical ? 90 : 0
        anchors.centerIn: parent

        PlasmaCore.FrameSvgItem {
            id: background

            anchors.fill: parent
            imagePath: "widgets/bar_meter_horizontal"
            prefix: "bar-inactive"
        }

        PlasmaCore.FrameSvgItem {
            id: bar

            width: indeterminate ? contents.width / 4 : range.position
            height: contents.height
            imagePath: "widgets/bar_meter_horizontal"
            prefix: "bar-active"
            visible: indeterminate || value > 0

            SequentialAnimation {
                id: indeterminateAnimation

                loops: Animation.Infinite

                PropertyAnimation {
                    target: bar;
                    property: "x"
                    duration: 800
                    to: 0
                }
                PropertyAnimation {
                    target: bar;
                    property: "x"
                    duration: 800
                    to: background.width - bar.width
                }
            }
        }
    }
}