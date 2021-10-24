/*
    SPDX-FileCopyrightText: 2012 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2014 David Edmundson <davidedmundson@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.0
import QtQuick.Controls.Styles 1.1 as QtQuickControlStyle
import QtQuick.Controls.Private 1.0

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents

/**
 * \internal
 */
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
        control.flickableItem.flickDeceleration = Math.round(1500 * PlasmaCore.Units.devicePixelRatio);
        // double maximum speed so it feels better
        control.flickableItem.maximumFlickVelocity = Math.round(2500 * PlasmaCore.Units.devicePixelRatio);

        control.flickableItem.interactive = true;
    }

    Component.onCompleted: syncVelocity()
    Connections {
        target: control
        function onContentItemChanged() {
            syncVelocity()
        }
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
                    duration: PlasmaCore.Units.longDuration
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
                    duration: PlasmaCore.Units.longDuration
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
                    duration: PlasmaCore.Units.longDuration
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
                    duration: PlasmaCore.Units.longDuration
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

    scrollBarBackground: Item {
        implicitWidth: Math.max(widthHint, bgFrame.fixedMargins.left + bgFrame.fixedMargins.right)
            + bgFrame.anchors.leftMargin + bgFrame.anchors.rightMargin
        implicitHeight: Math.max(widthHint, bgFrame.fixedMargins.top + bgFrame.fixedMargins.bottom)
            + bgFrame.anchors.topMargin + bgFrame.anchors.bottomMargin
        PlasmaCore.FrameSvgItem {
            id: bgFrame
            anchors {
                fill: parent
                leftMargin: scrollbarSvg.hasElement(`${bgFrame.usedPrefix}-hint-left-inset`) ? bgFrame.inset.left : 0
                rightMargin: scrollbarSvg.hasElement(`${bgFrame.usedPrefix}-hint-right-inset`) ? bgFrame.inset.right : 0
                topMargin: scrollbarSvg.hasElement(`${bgFrame.usedPrefix}-hint-top-inset`) ? bgFrame.inset.top : 0
                bottomMargin: scrollbarSvg.hasElement(`${bgFrame.usedPrefix}-hint-bottom-inset`) ? bgFrame.inset.bottom : 0
            }
            imagePath:"widgets/scrollbar"
            colorGroup: PlasmaCore.ColorScope.colorGroup
            prefix: styleData.horizontal ? "background-horizontal" : "background-vertical"
            opacity: styleData.hovered
            visible: opacity > 0
            Behavior on opacity {
                NumberAnimation {
                    duration: PlasmaCore.Units.longDuration
                    easing.type: Easing.OutCubic
                }
            }
        }
    }

    handle: Item {
        property bool hovered: __activeControl !== "none"
        implicitWidth: Math.max(widthHint, handleFrame.fixedMargins.left + handleFrame.fixedMargins.right)
            + handleFrame.anchors.leftMargin + handleFrame.anchors.rightMargin
        implicitHeight: Math.max(widthHint, handleFrame.fixedMargins.top + handleFrame.fixedMargins.bottom)
            + handleFrame.anchors.topMargin + handleFrame.anchors.bottomMargin
        PlasmaCore.FrameSvgItem {
            id: handleFrame
            anchors {
                fill: parent
                leftMargin: scrollbarSvg.hasElement(`${handleFrame.usedPrefix}-hint-left-inset`) ? handleFrame.inset.left : 0
                rightMargin: scrollbarSvg.hasElement(`${handleFrame.usedPrefix}-hint-right-inset`) ? handleFrame.inset.right : 0
                topMargin: scrollbarSvg.hasElement(`${handleFrame.usedPrefix}-hint-top-inset`) ? handleFrame.inset.top : 0
                bottomMargin: scrollbarSvg.hasElement(`${handleFrame.usedPrefix}-hint-bottom-inset`) ? handleFrame.inset.bottom : 0
            }
            imagePath:"widgets/scrollbar"
            implicitWidth: Math.max(scrollbarSvg.elementSize("hint-scrollbar-size").width, fixedMargins.left + fixedMargins.right)
            implicitHeight: Math.max(scrollbarSvg.elementSize("hint-scrollbar-size").height, fixedMargins.top + fixedMargins.bottom)
            colorGroup: PlasmaCore.ColorScope.colorGroup
            prefix: hovered && enabled ? "mouseover-slider" : "slider"
            opacity: enabled ? 1 : 0.5
        }
    }

    incrementControl: PlasmaCore.SvgItem {
        svg: scrollbarSvg
        visible: scrollbarSvg.arrowPresent
        //if there is no arrow we don't want to waste space, a tiny margin does look better though
        implicitWidth: scrollbarSvg.arrowPresent ? widthHint : PlasmaCore.Units.smallSpacing
        implicitHeight: scrollbarSvg.arrowPresent ? widthHint : PlasmaCore.Units.smallSpacing
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
        implicitWidth: scrollbarSvg.arrowPresent ? widthHint : PlasmaCore.Units.smallSpacing
        implicitHeight: scrollbarSvg.arrowPresent ? widthHint : PlasmaCore.Units.smallSpacing
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
