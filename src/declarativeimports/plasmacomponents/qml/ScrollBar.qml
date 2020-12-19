/*
    SPDX-FileCopyrightText: 2011 Daker Fernandes Pinheiro <dakerfp@gmail.com>
    SPDX-FileCopyrightText: 2011 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.1
import org.kde.plasma.core 2.0 as PlasmaCore
import "private" as Private
import org.kde.plasma.components 2.0 as PlasmaComponents

/**
 * A simple Scroll Bar using the plasma theme.
 *
 * This component does not belong to the QtComponents API specification but it
 * was based on the ScrollDecorator component. You should not use it for touch
 * interfaces, use a flickable and a ScrollDecorator instead.
 *
 * By default, this component looks and behaves like a scroll decorator
 * on touchscreens.
 * 
 * @inherit QtQuick.Item
 */
// TODO: add support mouse wheel events
Item {
    id: scrollbar

    // Common API
    /**
     * The Flickable component which the ScrollBar will interact with.
     */
    property Flickable flickableItem: null
    /**
     * The orientation where the ScrollBar will scroll.
     * The orientation * can be either Qt.Horizontal or Qt.Vertical.
     *
     * The default value is Qt.Vertical.
     */
    property int orientation: Qt.Vertical

    /**
     * Whether the ScrollBar is interactive.
     *
     * The default value is true.
     */
    property bool interactive: true

    // Plasma API
    /**
     * Whether the ScrollBar will increase the Flickable content in the normal
     * direction (Left to Right or Top to Bottom) or if this will be inverted.
     *
     * The default value is false.
     */
    property bool inverted: false

    /**
     * type:bool
     * How many steps exist while moving the handler. If you want the
     * ScrollBar buttons to appear you must set this property to a value
     * bigger than 0.
     *
     * The default value is 0.
     */
    property alias stepSize: range.stepSize

    /**
     * Indicates if the ScrollBar is pressed.
     */
    property bool pressed: internalLoader.item.mouseArea?internalLoader.item.mouseArea.pressed:false

    /**
     * The interval time used by the ScrollBar button to increase or decrease
     * steps.
     */
    property real scrollButtonInterval: 50

    implicitWidth: internalLoader.isVertical ? (internalLoader.item ? internalLoader.item.implicitWidth : 12) : 200
    implicitHeight: internalLoader.isVertical ? 200 : (internalLoader.item ? internalLoader.item.implicitHeight : 12)
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
                flickableItem.contentY = Math.max(0, Math.min(flickableItem.contentHeight - flickableItem.height,
                    flickableItem.contentY + increment))
            } else {
                flickableItem.contentX = Math.max(0, Math.min(flickableItem.contentWidth - flickableItem.width,
                    flickableItem.contentX + increment))
            }
        }


        Connections {
            target: flickableItem
            function onContentHeightChanged() {
                range.value = flickableItem.contentY
            }
            function onContentYChanged() {
                if (internalLoader.isVertical) {
                    range.value = flickableItem.contentY
                }
            }
            function onContentXChanged() {
                if (!internalLoader.isVertical) {
                    range.value = flickableItem.contentX
                }
            }
        }
        Connections {
            target: internalLoader.item.handle
            function onYChanged() {
                updateFromHandleTimer.running = true
            }
            function onXChanged() {
                updateFromHandleTimer.running = true
            }
        }
        PlasmaComponents.RangeModel {
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

        Timer {
            id: updateFromHandleTimer
            interval: 10
            onTriggered: {
                if(!enabled || !interactive)
                    return;
                
                if (internalLoader.isVertical) {
                    range.position = internalLoader.item.handle.y
                } else {
                    range.position = internalLoader.item.handle.x
                }
            }
        }

        source: interactive ? "private/ScrollBarDelegate.qml" : "private/ScrollDecoratorDelegate.qml"
    }
    Accessible.role: Accessible.ScrollBar
}
