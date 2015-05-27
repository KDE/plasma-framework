/*
*   Copyright (C) 2011 by Daker Fernandes Pinheiro <dakerfp@gmail.com>
*   Copyright (C) 2014 by Kai Uwe Broulik <kde@privat.broulik.de>
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
*   51 Franklin Street, Fifth Floor, Boston, MA  2.010-1301, USA.
*/

import QtQuick 2.2
import QtQuick.Controls.Styles 1.1

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents

ProgressBarStyle {
    id: style

    readonly property string imagePath: "widgets/bar_meter_horizontal"

    readonly property real implicitHeight: barSvg.preferredHeight * Math.max(1, Math.floor(units.devicePixelRatio))

    PlasmaCore.Svg {
        id: barSvg
        imagePath: style.imagePath
        property int preferredWidth
        property int preferredHeight
        onRepaintNeeded: {
            preferredWidth = barSvg.elementSize("hint-bar-size").width
            preferredHeight = barSvg.elementSize("hint-bar-size").height
            if (preferredWidth == 0) {
                preferredWidth = barSvg.elementSize("bar-inactive-left").width + barSvg.elementSize("bar-inactive-right").width 
            }
            if (preferredHeight == 0) {
                preferredHeight = barSvg.elementSize("bar-inactive-top").height + barSvg.elementSize("bar-inactive-bottom").height
            }
        }
    }

    background: Item {
        implicitWidth: units.gridUnit * 14
        implicitHeight: theme.mSize(theme.defaultFont).height * 1.6
        PlasmaCore.FrameSvgItem {
            anchors.centerIn: parent
            width: parent.width
            height: style.implicitHeight
            imagePath: style.imagePath
            prefix: "bar-inactive"
            colorGroup: PlasmaCore.ColorScope.colorGroup
        }
    }

    progress: Item {
        PlasmaCore.FrameSvgItem {
            id: progressSvg
            anchors.verticalCenter: parent.verticalCenter
            imagePath: style.imagePath
            prefix: "bar-active"
            colorGroup: PlasmaCore.ColorScope.colorGroup

            width: control.indeterminate ? units.gridUnit * 2 : Math.max(margins.left + margins.right, parent.width)
            height: style.implicitHeight
            visible: control.indeterminate || currentProgress > 0

            SequentialAnimation {
                id: anim
                loops: Animation.Infinite

                running: control.indeterminate && control.visible
                onRunningChanged: {
                    if (!running) {
                        progressSvg.x = 0
                    }
                }

                PropertyAnimation {
                    target: progressSvg
                    property: "x"
                    duration: 800
                    // The progress Item's width might be left over from the previous state
                    // so we use the control.width/height directly instead
                    to: control.width - units.gridUnit * 2
                    onToChanged: {
                        //the animation won't update the boundaries automatically
                        if (anim.running) {
                            anim.restart();
                        }
                    }
                }
                PropertyAnimation {
                    target: progressSvg
                    property: "x"
                    duration: 800
                    to: 0
                }
            }
        }
    }
}
