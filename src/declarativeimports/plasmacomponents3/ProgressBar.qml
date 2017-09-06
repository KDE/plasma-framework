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

T.ProgressBar {
    id: control

    implicitWidth: units.gridUnit * 8
    implicitHeight: background.implicitHeight

    hoverEnabled: true

    contentItem: Item {
        scale: control.mirrored ? -1 : 1
        PlasmaCore.FrameSvgItem {
            id: indicator
            height: parent.height
            width: control.indeterminate ? units.gridUnit * 2 : parent.width * control.position
            imagePath: "widgets/bar_meter_horizontal"
            prefix: "bar-active"
            colorGroup: PlasmaCore.ColorScope.colorGroup
        }
        SequentialAnimation {
            id: anim
            loops: Animation.Infinite

            running: control.indeterminate && control.visible

            PropertyAnimation {
                target: indicator
                property: "x"
                duration: 800
                to: control.width - indicator.width
                onToChanged: {
                    //the animation won't update the boundaries automatically
                    if (anim.running) {
                        anim.restart();
                    }
                }
            }
            PropertyAnimation {
                target: indicator
                property: "x"
                duration: 800
                to: 0
            }
        }
    }

    background: PlasmaCore.FrameSvgItem {
        imagePath: "widgets/bar_meter_horizontal"
        prefix: "bar-inactive"
        colorGroup: PlasmaCore.ColorScope.colorGroup
        onRepaintNeeded: {
            implicitHeight = barSvg.elementSize("hint-bar-size").height
            if (implicitHeight == 0) {
                implicitHeight = barSvg.elementSize("bar-inactive-top").height + barSvg.elementSize("bar-inactive-bottom").height
            }
        }
    }
}
