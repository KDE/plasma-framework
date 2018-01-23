/*
 *   Copyright 2014 Digia Plc and/or its subsidiary(-ies).
 *   Copyright 2014 Marco Martin <mart@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU Library General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  2.010-1301, USA.
 */

import QtQuick 2.2
import QtQuick.Controls.Styles 1.2 as QtQuickControlStyle
import QtQuick.Controls 1.2
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents

QtQuickControlStyle.TableViewStyle {
    id: styleRoot

    textColor: PlasmaCore.ColorScope.textColor
    backgroundColor: control.backgroundVisible ? theme.viewBackgroundColor : "transparent"
    alternateBackgroundColor: Qt.darker(theme.viewBackgroundColor, 1.05)
    highlightedTextColor: theme.viewTextColor
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
            color: theme.buttonTextColor
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
        height: Math.round(units.gridUnit * 1.2)
        property color selectedColor: styleData.hasActiveFocus ? theme.viewHoverColor: theme.viewFocusColor

        color: styleData.selected ? selectedColor :
                                    !styleData.alternate ? alternateBackgroundColor : theme.viewBackgroundColor
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
            color: theme.viewTextColor
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
        implicitWidth: scrollbarSvg.arrowPresent ? scrollbarWidthHint : units.smallSpacing
        implicitHeight: scrollbarSvg.arrowPresent ? scrollbarWidthHint : units.smallSpacing
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
        implicitWidth: scrollbarSvg.arrowPresent ? scrollbarWidthHint : units.smallSpacing
        implicitHeight: scrollbarSvg.arrowPresent ? scrollbarWidthHint : units.smallSpacing
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

