/*
    SPDX-FileCopyrightText: 2011 Daker Fernandes Pinheiro <dakerfp@gmail.com>
    SPDX-FileCopyrightText: 2014 Kai Uwe Broulik <kde@privat.broulik.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.2
import QtQuick.Controls.Styles 1.1

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents

/**
 * \internal
 */
ProgressBarStyle {
    id: style

    readonly property string imagePath: "widgets/bar_meter_horizontal"

    readonly property real implicitHeight: barSvg.preferredHeight

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
        implicitWidth: PlasmaCore.Units.gridUnit * 14
        implicitHeight: PlasmaCore.Theme.mSize(PlasmaCore.Theme.defaultFont).height * 1.6
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

            width: control.indeterminate ? PlasmaCore.Units.gridUnit * 2 : Math.max(margins.left + margins.right, parent.width)
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
                    duration: PlasmaCore.Units.veryLongDuration * 2
                    // The progress Item's width might be left over from the previous state
                    // so we use the control.width/height directly instead
                    to: control.width - PlasmaCore.Units.gridUnit * 2
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
                    duration: PlasmaCore.Units.veryLongDuration * 2
                    to: 0
                }
            }
        }
    }
}
