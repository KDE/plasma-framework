/*
 *   Copyright (C) 2014 by Marco Martin <mart@kde.org>
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


import QtQuick 2.1
import QtQuick.Controls.Styles 1.1 as QtQuickControlStyle
import QtQuick.Controls 1.1

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents

import "private" as Private

QtQuickControlStyle.SliderStyle {
    id: styleItem

    property Item handleItem
    padding { top: 0 ; left: 0 ; right: 0 ; bottom: 0 }

    PlasmaCore.Svg {
        id: grooveSvg
        imagePath: "widgets/slider"
        colorGroup: PlasmaCore.ColorScope.colorGroup

    }
    handle: Item {
        width: handle.naturalSize.width
        height: handle.naturalSize.height
        //keep this opaque to not show half highlight beneath
        Private.RoundShadow {
            id: shadow
            anchors.fill: parent
            imagePath: "widgets/slider"
            focusElement: "horizontal-slider-focus"
            hoverElement: "horizontal-slider-hover"
            shadowElement: "horizontal-slider-shadow"
            state: control.activeFocus ? "focus" : (control.hovered ? "hover" : "shadow")
        }
        PlasmaCore.SvgItem {
            id: handle
            anchors.fill: parent
            svg: grooveSvg
            elementId: "horizontal-slider-handle"
        }
    }

    groove: PlasmaCore.FrameSvgItem {
        id: groove
        imagePath: "widgets/slider"
        prefix: "groove"
        height: implicitHeight
        colorGroup: PlasmaCore.ColorScope.colorGroup
        opacity: control.enabled ? 1 : 0.6

        PlasmaCore.FrameSvgItem {
            id: highlight
            imagePath: "widgets/slider"
            prefix: "groove-highlight"
            height: groove.height

            width: styleData.handlePosition
            anchors.verticalCenter: parent.verticalCenter
            colorGroup: PlasmaCore.ColorScope.colorGroup

            visible: value > 0
        }
    }


   tickmarks: Repeater {
        id: repeater
        model: control.stepSize > 0 ? 1 + (control.maximumValue - control.minimumValue) / control.stepSize : 0
        width: control.orientation == Qt.Vertical ? control.height : control.width
        height: control.orientation == Qt.Vertical ? control.width : control.height
        Rectangle {
            color: PlasmaCore.ColorScope.textColor
            width: 1 ; height: 3
            y: control.orientation == Qt.Vertical ? control.width : control.height
            //Position ticklines from styleData.handleWidth to width - styleData.handleWidth/2
            //position them at an half handle width increment
            x: styleData.handleWidth / 2 + index * ((repeater.width - styleData.handleWidth) / (repeater.count>1 ? repeater.count-1 : 1))
            opacity: control.enabled ? 1 : 0.6
        }
    }
}
