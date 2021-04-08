/*
    SPDX-FileCopyrightText: 2014 Digia Plc and /or its subsidiary(-ies).
    SPDX-FileCopyrightText: 2014 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.2
import QtQuick.Controls.Styles 1.2 as QtQuickControlStyle
import QtQuick.Controls 1.2
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents

/**
 * \internal
 */
QtQuickControlStyle.TableViewStyle {
    id: styleRoot

    textColor: PlasmaCore.ColorScope.textColor
    backgroundColor: control.backgroundVisible ? PlasmaCore.Theme.viewBackgroundColor : "transparent"
    alternateBackgroundColor: Qt.darker(PlasmaCore.Theme.viewBackgroundColor, 1.05)
    highlightedTextColor: PlasmaCore.Theme.viewTextColor
    activateItemOnSingleClick: false


    property real scrollbarWidthHint: Math.round( (scrollbarSvg.hasElement("hint-scrollbar-size") ? scrollbarSvg.elementSize("hint-scrollbar-size").width : scrollbarSvg.elementSize("arrow-up").width))

    headerDelegate: PlasmaCore.FrameSvgItem {
        imagePath: "widgets/button"
        prefix: "normal"
        enabledBorders: PlasmaCore.FrameSvgItem.TopEdge | PlasmaCore.FrameSvgItem.BottomEdge

        height: textItem.implicitHeight * 1.2

        PlasmaComponents.Label {
            id: textItem
            anchors.fill: parent
            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: styleData.textAlignment
            anchors.leftMargin: 12
            color: PlasmaCore.Theme.buttonTextColor
            text: styleData.value
            elide: Text.ElideRight
        }
        Rectangle {
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 1
            anchors.topMargin: 1
            width: 1
            color: Qt.rgba(PlasmaCore.ColorScope.textColor.r, PlasmaCore.ColorScope.textColor.g, PlasmaCore.ColorScope.textColor.b, 0.2)
        }
    }


    rowDelegate: Rectangle {
        height: Math.round(PlasmaCore.Units.gridUnit * 1.2)
        property color selectedColor: styleData.hasActiveFocus ? PlasmaCore.Theme.viewHoverColor: PlasmaCore.Theme.viewFocusColor

        color: styleData.selected ? selectedColor :
                                    !styleData.alternate ? alternateBackgroundColor : PlasmaCore.Theme.viewBackgroundColor
    }


    itemDelegate: Item {
        height: Math.max(16, label.implicitHeight)
        property int implicitWidth: label.implicitWidth + 20

        PlasmaComponents.Label {
            id: label
            objectName: "label"
            width: parent.width
            anchors.leftMargin: 12
            anchors.left: parent.left
            anchors.right: parent.right
            horizontalAlignment: styleData.textAlignment
            anchors.verticalCenter: parent.verticalCenter
            anchors.verticalCenterOffset: 1
            elide: styleData.elideMode
            text: styleData.value !== undefined ? styleData.value : ""
            color: PlasmaCore.Theme.viewTextColor
        }
    }


    frame: Item {

    }

    scrollBarBackground: PlasmaCore.FrameSvgItem {
        imagePath:"widgets/scrollbar"
        prefix: styleData.horizontal ? "background-horizontal" : "background-vertical"
        implicitWidth: scrollbarWidthHint
    }

    handle: PlasmaCore.FrameSvgItem {
        imagePath:"widgets/scrollbar"
        implicitWidth: scrollbarWidthHint
        implicitHeight: scrollbarWidthHint

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
        implicitWidth: scrollbarSvg.arrowPresent ? scrollbarWidthHint : PlasmaCore.Units.smallSpacing
        implicitHeight: scrollbarSvg.arrowPresent ? scrollbarWidthHint : PlasmaCore.Units.smallSpacing
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
        implicitWidth: scrollbarSvg.arrowPresent ? scrollbarWidthHint : PlasmaCore.Units.smallSpacing
        implicitHeight: scrollbarSvg.arrowPresent ? scrollbarWidthHint : PlasmaCore.Units.smallSpacing
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

