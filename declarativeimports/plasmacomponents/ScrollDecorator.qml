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

import QtQuick 1.1
import org.kde.plasma.core 0.1 as PlasmaCore

Item {
    id: scrollDecorator

    // Common API
    property Flickable flickableItem: null

    // Plasma API
    property int orientation: Qt.Horizontal
    property bool inverted: false

    // Convinience API
    property bool _isVertical: orientation == Qt.Vertical
    property bool _inverted: _isVertical ?
        !scrollDecorator.inverted : scrollDecorator.inverted
    property alias _value: range.value

    implicitWidth: _isVertical ? 22 : 200
    implicitHeight: _isVertical ? 200 : 22

    visible: flickableItem && handle.width < contents.width

    Item {
        width: _isVertical ? scrollDecorator.height : scrollDecorator.width
        height: _isVertical ? scrollDecorator.width : scrollDecorator.height
        rotation: _isVertical ? -90 : 0

        anchors.centerIn: parent

        PlasmaCore.Svg {
            id: scrollDecoratorSvg
            imagePath: "widgets/scrollDecorator"
        }

        Item {
            id: contents

            anchors.fill: parent

            PlasmaCore.RangeModel {
                id: range

                minimumValue: 0
                maximumValue: {
                    var diff;
                    if (_isVertical)
                        diff = flickableItem.contentHeight - flickableItem.height;
                    else
                        diff = flickableItem.contentWidth - flickableItem.width;

                    return Math.max(0, diff);
                }
                stepSize: 0.0
                inverted: _inverted
                positionAtMinimum: 0 + handle.width / 2
                positionAtMaximum: contents.width - handle.width / 2
                value: _isVertical ? flickableItem.contentY : flickableItem.contentX
            }

            PlasmaCore.FrameSvgItem {
                id: handle

                x: range.position
                transform: Translate { x: - handle.width / 2 }
                anchors.verticalCenter: parent.verticalCenter
                width: {
                    var ratio;
                    if (_isVertical)
                        ratio = flickableItem.visibleArea.heightRatio;
                    else
                        ratio = flickableItem.visibleArea.widthRatio;

                    return ratio * parent.width;
                }
                height: parent.height - margins.top // TODO: check mergin
                imagePath: "widgets/scrollbar"
                prefix: "slider"
                opacity: flickableItem && flickableItem.flicking ? 1 : 0

                Connections {
                    target: flickableItem
                    onMovementStarted: handle.opacity = 1
                    onMovementEnded: opacityAnimation.start()
                }

                Behavior on x {
                    PropertyAnimation {
                        duration: 150
                        easing.type: Easing.OutSine
                    }
                }

                PropertyAnimation {
                    id: opacityAnimation

                    target: handle
                    property: "opacity"
                    from: 1; to: 0
                    duration: 500
                    easing.type: Easing.Linear
                }
            }
        }
    }
}