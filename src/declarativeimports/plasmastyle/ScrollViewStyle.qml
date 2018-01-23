/*
*   Copyright 2012 Marco Martin <mart@kde.org>
*   Copyright (C) 2014 by David Edmundson <davidedmundson@kde.org>
*
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU Library General Public License as
*   published by the Free Software Foundation; either version 2, or
*   (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU Library General Public License for more details
*
*   You should have received a copy of the GNU Library General Public
*   License along with this program; if not, write to the
*   Free Software Foundation, Inc.,
*   51 Franklin Street, Fifth Floor, Boston, MA  2.010-1301, USA.
*/

import QtQuick 2.0
import QtQuick.Controls.Styles 1.1 as QtQuickControlStyle
import QtQuick.Controls.Private 1.0

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents

QtQuickControlStyle.ScrollViewStyle {
    property Flickable flickableItem: control.flickableItem

    property real widthHint: Math.round( (scrollbarSvg.hasElement("hint-scrollbar-size") ? scrollbarSvg.elementSize("hint-scrollbar-size").width : scrollbarSvg.elementSize("arrow-up").width))

    property StyleItem __styleitem: StyleItem { elementType: "frame" }
    readonly property int __wheelScrollLines: __styleitem.styleHint("wheelScrollLines")

    transientScrollBars: Settings.hasTouchScreen && Settings.isMobile

    padding {
        top: 0
        left: 0
        right: 0
        bottom: 0
    }

    function syncVelocity() {
        if (!control.flickableItem) {
            return;
        }

        // QTBUG-35608
        // default values are hardcoded in qtdeclarative/src/quick/items/qquickflickablebehavior_p.h
        control.flickableItem.flickDeceleration = Math.round(1500 * units.devicePixelRatio);
        // double maximum speed so it feels better
        control.flickableItem.maximumFlickVelocity = Math.round(2500 * units.devicePixelRatio);

        control.flickableItem.interactive = true;
    }

    Component.onCompleted: syncVelocity()
    Connections {
        target: control
        onContentItemChanged: syncVelocity()
    }

    frame: Item {
        visible: frameVisible

        PlasmaCore.Svg {
            id: borderSvg
            imagePath: "widgets/scrollwidget"
            colorGroup: PlasmaCore.ColorScope.colorGroup
        }

        PlasmaCore.SvgItem {
            visible: {
                if (flickableItem.hasOwnProperty("orientation")) {
                    return (flickableItem.orientation === Qt.Vertical);
                }

                return false;
            }
            svg: borderSvg
            z: 1000
            elementId: "border-top"
            width: 100
            height: naturalSize.height
            opacity: !visible || flickableItem.atYBeginning ? 0 : 1
            Behavior on opacity {
                NumberAnimation {
                    duration: units.longDuration
                    easing.type: Easing.InOutQuad
                }
            }
            anchors {
                left: parent.left
                top: parent.top
                right: parent.right
            }
        }
        PlasmaCore.SvgItem {
            visible: {
                if (flickableItem.hasOwnProperty("orientation")) {
                    return (flickableItem.orientation === Qt.Vertical);
                }

                return false;
            }
            svg: borderSvg
            z: 1000
            elementId: "border-bottom"
            width: 100
            height: naturalSize.height
            opacity: !visible || flickableItem.atYEnd ? 0 : 1
            Behavior on opacity {
                NumberAnimation {
                    duration: units.longDuration
                    easing.type: Easing.InOutQuad
                }
            }
            anchors {
                left: parent.left
                bottom: parent.bottom
                right: parent.right
            }
        }
        PlasmaCore.SvgItem {
            visible: {
                if (flickableItem.hasOwnProperty("orientation")) {
                    return (flickableItem.orientation === Qt.Horizontal);
                }

                return false;
            }
            svg: borderSvg
            z: 1000
            elementId: "border-left"
            width: naturalSize.width
            opacity: !visible || flickableItem.atXBeginning ? 0 : 1
            Behavior on opacity {
                NumberAnimation {
                    duration: units.longDuration
                    easing.type: Easing.InOutQuad
                }
            }
            anchors {
                left: parent.left
                top: parent.top
                bottom: parent.bottom
            }
        }
        PlasmaCore.SvgItem {
            visible: {
                if (flickableItem.hasOwnProperty("orientation")) {
                    return (flickableItem.orientation === Qt.Horizontal);
                }

                return false;
            }
            svg: borderSvg
            z: 1000
            elementId: "border-right"
            width: naturalSize.width
            opacity: !visible || flickableItem.atXEnd ? 0 : 1
            Behavior on opacity {
                NumberAnimation {
                    duration: units.longDuration
                    easing.type: Easing.InOutQuad
                }
            }
            anchors {
                top: parent.top
                bottom: parent.bottom
                right: parent.right
            }
        }
    }

    scrollBarBackground: PlasmaCore.FrameSvgItem {
        imagePath:"widgets/scrollbar"
        prefix: styleData.horizontal ? "background-horizontal" : "background-vertical"
        implicitWidth: widthHint
        colorGroup: PlasmaCore.ColorScope.colorGroup
        opacity: styleData.hovered ? 1 : 0
        Behavior on opacity {
            NumberAnimation {
                duration: units.longDuration
                easing.type: Easing.InOutQuad
            }
        }
    }

    handle: PlasmaCore.FrameSvgItem {
        imagePath:"widgets/scrollbar"
        implicitWidth: widthHint
        implicitHeight: widthHint
        colorGroup: PlasmaCore.ColorScope.colorGroup

        prefix: {
            if (styleData.hovered) {
                return "sunken-slider"
            }
            if (styleData.pressed) {
                return "mouseover-slider"
            } else {
                return "slider"
            }
        }
    }

    incrementControl: PlasmaCore.SvgItem {
        svg: scrollbarSvg
        visible: scrollbarSvg.arrowPresent
        //if there is no arrow we don't want to waste space, a tiny margin does look better though
        implicitWidth: scrollbarSvg.arrowPresent ? widthHint : units.smallSpacing
        implicitHeight: scrollbarSvg.arrowPresent ? widthHint : units.smallSpacing
        elementId: {
            if (styleData.pressed) {
                return styleData.horizontal ? "sunken-arrow-right" : "sunken-arrow-down"
            }
            if (styleData.hovered) {
                return styleData.horizontal ? "mouseover-arrow-right" : "mouseover-arrow-down"
            } else {
                return styleData.horizontal ? "arrow-right" : "arrow-down"
            }
        }
    }

    decrementControl: PlasmaCore.SvgItem {
        svg: scrollbarSvg
        visible: scrollbarSvg.arrowPresent
        implicitWidth: scrollbarSvg.arrowPresent ? widthHint : units.smallSpacing
        implicitHeight: scrollbarSvg.arrowPresent ? widthHint : units.smallSpacing
        elementId: {
            if (styleData.pressed) {
                return styleData.horizontal ? "sunken-arrow-left" : "sunken-arrow-up"
            }
            if (styleData.hovered) {
                return styleData.horizontal ? "mouseover-arrow-left" : "mouseover-arrow-up"
            } else {
                return styleData.horizontal ? "arrow-left" : "arrow-up"
            }
        }
    }

    PlasmaCore.Svg {
        id: scrollbarSvg
        imagePath: "widgets/scrollbar"
        property bool arrowPresent: scrollbarSvg.hasElement("arrow-up")
        //new theme may be different
        onRepaintNeeded: arrowPresent = scrollbarSvg.hasElement("arrow-up")
    }
}
