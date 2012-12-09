/*
*   Copyright (C) 2011 by Daker Fernandes Pinheiro <dakerfp@gmail.com>
*   Copyright (C) 2011 Marco Martin <mart@kde.org>
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
import "private" as Private


/**
 * A generic ScrollBar/ScrollDecorator component:
 * Always prefer this to ScrollDecorator that is not available on desktop.
 * By default, this component will look and behave like a scroll decorator on touchscreens
 */
// TODO: add support mouse wheel events
Item {
    id: scrollbar

    // Common API
    property Flickable flickableItem: null
    property int orientation: Qt.Vertical
    property bool interactive: false

    // Plasma API
    property bool inverted: false
    property alias stepSize: range.stepSize
    property bool pressed: internalLoader.item.mouseArea?internalLoader.item.mouseArea.pressed:false
    property real scrollButtonInterval: 50

    implicitWidth: internalLoader.isVertical ? (interactive ? 22 : 12) : 200
    implicitHeight: internalLoader.isVertical ? 200 : (interactive ? 22 : 12)
    // TODO: needs to define if there will be specific graphics for
    //     disabled scroll bars
    opacity: enabled ? 1.0 : 0.5

    visible: flickableItem && internalLoader.handleEnabled

    anchors {
        right: internalLoader.anchorableWithFlickable ? flickableItem.right : undefined
        left: (orientation == Qt.Vertical) ? undefined : (internalLoader.anchorableWithFlickable ? flickableItem.left : undefined)
        top: (orientation == Qt.Vertical) ? (internalLoader.anchorableWithFlickable ? flickableItem.top : undefined) : undefined
        bottom: internalLoader.anchorableWithFlickable ? flickableItem.bottom : undefined
    }

    Loader {
        id: internalLoader
        anchors.fill: parent
        //property bool handleEnabled: internalLoader.isVertical ? item.handle.height < item.contents.height : item.handle.width < item.contents.width
        property bool handleEnabled: internalLoader.isVertical ? flickableItem.contentHeight > flickableItem.height : flickableItem.contentWidth > flickableItem.width
        property bool isVertical: orientation == Qt.Vertical
        property bool anchorableWithFlickable: scrollbar.parent == flickableItem || scrollbar.parent == flickableItem.parent

        function incrementValue(increment)
        {
            if (!flickableItem)
                return;

            if (internalLoader.isVertical) {
                flickableItem.contentY = Math.max(0, Math.min(flickableItem.contentHeight,
                    flickableItem.contentY + increment))
            } else {
                flickableItem.contentX = Math.max(0, Math.min(flickableItem.contentWidth,
                    flickableItem.contentX + increment))
            }
        }

        RangeModel {
            id: range

            minimumValue: 0
            maximumValue: {
                var diff;
                if (internalLoader.isVertical) {
                    diff = flickableItem.contentHeight - flickableItem.height
                } else {
                    diff = flickableItem.contentWidth - flickableItem.width
                }

                return Math.max(0, diff)
            }

            stepSize: 10
            inverted: scrollbar.inverted
            positionAtMinimum: 0
            positionAtMaximum: {
                if (internalLoader.isVertical) {
                    internalLoader.item.contents.height - internalLoader.item.handle.height
                } else {
                    internalLoader.item.contents.width - internalLoader.item.handle.width
                }
            }
            value: internalLoader.isVertical ? flickableItem.contentY : flickableItem.contentX
            onValueChanged: {
                if (flickableItem.moving) {
                    return
                }

                if (internalLoader.isVertical) {
                    flickableItem.contentY = value
                } else {
                    flickableItem.contentX = value
                }
            }

            position: internalLoader.isVertical ? internalLoader.item.handle.y : internalLoader.item.handle.x

            onPositionChanged: {
                if (internalLoader.item.mouseArea && internalLoader.item.mouseArea.pressed) {
                    return
                }

                if (internalLoader.isVertical) {
                    internalLoader.item.handle.y = position
                } else {
                    internalLoader.item.handle.x = position
                }
            }
        }

        source: interactive ? "private/ScrollBarDelegate.qml" : "private/ScrollDecoratorDelegate.qml"
    }
}
