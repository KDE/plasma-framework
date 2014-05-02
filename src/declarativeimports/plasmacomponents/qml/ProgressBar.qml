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
*   51 Franklin Street, Fifth Floor, Boston, MA  2.010-1301, USA.
*/

import QtQuick 2.1
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.kquickcontrolsaddons 2.0

/**
 * Simple progressBar using the plasma theme.
 *
 * Some operations take a period of time to be performed and the user needs a
 * confirmation that the operation is still ongoing. If the user does not get
 * any confirmation, they might suspect that they did something wrong or that
 * the device has broken. A progress bar is one of the available mechanisms for
 * providing this reassurance to the user.
 */
Item {
    id: progressBar

    // Common API
    /** type:real Minimum value for the progressBar */
    property alias minimumValue: range.minimumValue
    /** type:real Maximum value for the progressBar */
    property alias maximumValue: range.maximumValue
    /** type:real Current value of the progressBar */
    property alias value: range.value
    /**
     * type:bool
     * Indicates whether the operation's duration is known or not. The property
     * can have the following values:
     *
     * - true: the operation's duration is unknown, so the progress bar is
     *   animated. The value, minimum, and maximum properties are ignored.
     * - false: the operation's duration is known, so the progress bar is drawn
     *   to indicate progress between the minimum and maximum values.
     *
     * The default value is false.
     */
    property alias indeterminate: indeterminateAnimation.running

    // Plasma API
    /**
     * Orientation of the progressBar: Qt.Horizontal or Qt.Vertical
     */
    property int orientation: Qt.Horizontal

    width: 100
    height: 20 * Math.max(1.0, Math.floor(units.devicePixelRatio))
    opacity: enabled ? 1.0 : 0.5

    PlasmaComponents.RangeModel {
        id: range

        // default values
        minimumValue: 0.0
        maximumValue: 1.0
        value: 0

        positionAtMinimum: 0
        positionAtMaximum: background._isVertical ? background.height : background.width
    }

    PlasmaCore.FrameSvgItem {
        id: background
        anchors.fill: parent
        imagePath: _isVertical ? "widgets/bar_meter_vertical" : "widgets/bar_meter_horizontal"
        prefix: "bar-inactive"
        property bool _isVertical: orientation == Qt.Vertical

        PlasmaCore.FrameSvgItem {
            id: bar
            anchors {
                left: indeterminate && !background._isVertical ? undefined : parent.left
                bottom: indeterminate && background._isVertical ? undefined : parent.bottom
                right: background._isVertical ? parent.right : undefined
                top: background._isVertical ? undefined : parent.top
            }
            imagePath: background.imagePath
            prefix: "bar-active"

            width: indeterminate ? units.gridUnit*2 : range.position
            height: indeterminate ? units.gridUnit*2 : range.position
        }
    }

    PlasmaCore.Svg {
        id: barSvg
        imagePath: background.imagePath
    }

    SequentialAnimation {
        id: indeterminateAnimation

        loops: Animation.Infinite

        onRunningChanged: {
            if (!running) {
                barPixmapItem.x = 0
                barPixmapItem.y = 0
            }
        }

        PropertyAnimation {
            target: bar
            property: background._isVertical ? "y" : "x"
            duration: 800
            to: 0
        }
        PropertyAnimation {
            target: bar
            property: background._isVertical ? "y" : "x"
            duration: 800
            to: background._isVertical ? background.height - bar.height : background.width - bar.width
        }
    }
}
