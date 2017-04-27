/*
 *   Copyright 2016 Marco Martin <mart@kde.org>
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
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

import QtQuick 2.6
import QtQuick.Templates 2.0 as T
import org.kde.plasma.core 2.0 as PlasmaCore


T.ScrollBar {
    id: control

    implicitWidth: background.implicitWidth
    implicitHeight: background.implicitHeight

    hoverEnabled: true

    visible: control.size < 1.0

    background: PlasmaCore.FrameSvgItem {
        imagePath:"widgets/scrollbar"
        implicitWidth: scrollbarSvg.elementSize("hint-scrollbar-size").width 
        implicitHeight: implicitWidth
        colorGroup: PlasmaCore.ColorScope.colorGroup
        visible: control.size < 1.0

        prefix: control.horizontal ? "background-horizontal" : "background-vertical"
        opacity: control.hovered ? 1 : 0
        Behavior on opacity {
            OpacityAnimator {
                duration: 250
            }
        }
    }

    contentItem: PlasmaCore.FrameSvgItem {
        imagePath:"widgets/scrollbar"
        implicitWidth: scrollbarSvg.elementSize("hint-scrollbar-size").width 
        implicitHeight: implicitWidth
        colorGroup: PlasmaCore.ColorScope.colorGroup
        visible: control.size < 1.0

        prefix: control.hovered ? "mouseover-slider" : "slider"
    }

    PlasmaCore.Svg {
        id: scrollbarSvg
        imagePath: "widgets/scrollbar"
        //TODO: support arrows?
        property bool arrowPresent: scrollbarSvg.hasElement("arrow-up")
        //new theme may be different
        onRepaintNeeded: arrowPresent = scrollbarSvg.hasElement("arrow-up")
    }
}
