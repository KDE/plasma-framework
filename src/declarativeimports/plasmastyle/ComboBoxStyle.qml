/*
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
import QtQuick.Window 2.2
import QtQuick.Controls.Styles 1.1 as QtQuickControlStyle

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents

import "private" as Private

QtQuickControlStyle.ComboBoxStyle {
    drowDownButtonWidth: units.iconSizes.small

    label: PlasmaComponents.Label {
        text: control.currentText
        elide: Text.ElideRight
        color: theme.buttonTextColor
        verticalAlignment: Text.AlignTop
        horizontalAlignment: Text.AlignLeft
    }

    background: Item {

        //size copied from Plasma Button
        //for some reason the logic there is:
        // maximum of
        //   the calculated height + frame margins
        //   Or 1.6 letters tall no matter how big the margins are

        //QtQuickControls tries to be helpful and adds on the margin sizes for us
        //to compensate, we have to subtract our margins here in order to do the  max 1.6 lines high tall feature
        implicitHeight: Math.max(theme.mSize(theme.defaultFont).height*1.6 - surfaceNormal.margins.top - surfaceNormal.margins.bottom,
                                    theme.mSize(theme.defaultFont).height)

        implicitWidth: theme.mSize(theme.defaultFont).width*12

        Private.ButtonShadow {
            anchors.fill: parent
            state: {
                if (control.pressed) {
                    return "hidden"
                } else if (control.hovered) {
                    return "hover"
                } else if (control.activeFocus) {
                    return "focus"
                } else {
                    return "shadow"
                }
            }
        }


        //This code is duplicated here and Button and ToolButton
        //maybe we can make an AbstractButton class?
        PlasmaCore.FrameSvgItem {
            id: surfaceNormal
            anchors.fill: parent
            imagePath: "widgets/button"
            prefix: "normal"
        }

        PlasmaCore.FrameSvgItem {
            id: surfacePressed
            anchors.fill: parent
            imagePath: "widgets/button"
            prefix: "pressed"
            opacity: 0
        }

        state: control.pressed ? "pressed" : "normal"

        states: [
            State { name: "normal" },
            State { name: "pressed"
                    PropertyChanges {
                        target: surfaceNormal
                        opacity: 0
                    }
                    PropertyChanges {
                        target: surfacePressed
                        opacity: 1
                    }
            }
        ]

        transitions: [
            Transition {
                to: "normal"
                //Cross fade from pressed to normal
                ParallelAnimation {
                    NumberAnimation { target: surfaceNormal; property: "opacity"; to: 1; duration: 100 }
                    NumberAnimation { target: surfacePressed; property: "opacity"; to: 0; duration: 100 }
                }
            }
        ]

        PlasmaCore.SvgItem {
            width: drowDownButtonWidth
            height: drowDownButtonWidth
            anchors {
                right: parent.right
                rightMargin: surfaceNormal.margins.right
                verticalCenter: parent.verticalCenter
            }
            svg: PlasmaCore.Svg {
                imagePath: "widgets/arrows"
                colorGroup: PlasmaCore.Theme.ButtonColorGroup
            }
            elementId: "down-arrow"
        }

        Component.onCompleted: {
            padding.top = surfaceNormal.margins.top
            padding.left = surfaceNormal.margins.left
            padding.right = surfaceNormal.margins.right
            padding.bottom = surfaceNormal.margins.bottom
        }
    }
}
