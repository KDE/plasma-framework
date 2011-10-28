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
    property Flickable flickableItem

    // Plasma API
    property int orientation: Qt.Vertical
    property bool inverted: false

    // Convinience API
    property bool _isVertical: orientation == Qt.Vertical
    property bool _inverted: _isVertical ?
        !scrollDecorator.inverted : scrollDecorator.inverted
    property alias _value: range.value

    implicitWidth: _isVertical ? 16 : 200
    implicitHeight: _isVertical ? 200 : 16

    visible: flickableItem && handle.width < contents.width

    anchors {
        right: flickableItem.right
        left: (orientation == Qt.Vertical) ? undefined : flickableItem.left 
        top: (orientation == Qt.Vertical) ? flickableItem.top : undefined
        bottom: flickableItem.bottom
    }

    Item {
        width: _isVertical ? scrollDecorator.height : scrollDecorator.width
        height: _isVertical ? scrollDecorator.width : scrollDecorator.height
        rotation: _isVertical ? -90 : 0

        anchors.centerIn: parent

        PlasmaCore.FrameSvgItem {
            id: contents
            imagePath: "widgets/scrollbar"
            prefix: "background-horizontal"

            anchors.fill: parent
            opacity: flickableItem && (flickableItem.flicking || flickableItem.moving)  ? 1 : 0
            Behavior on opacity {
                NumberAnimation {
                    duration: 250
                    easing.type: Easing.OutQuad
                }
            }

            RangeModel {
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
            }
        }
    }
}