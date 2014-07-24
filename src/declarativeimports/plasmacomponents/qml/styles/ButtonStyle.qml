/*
 *   Copyright 2014 by Marco Martin <mart@kde.org>
 *   Copyright 2014 by David Edmundson <davidedmundson@kde.org>
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

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents

import "../private" as Private

QtQuickControlStyle.ButtonStyle {
    id: style

    label: Row {
        id: buttonContent
        spacing: icon.valid ? units.smallSpacing : 0

        PlasmaCore.IconItem {
            id: icon
            source: control.iconSource
            anchors.verticalCenter: parent.verticalCenter
            width: valid ? parent.height: 0
            visible: valid
            height: width
            active: shadow.hasOverState && mouse.containsMouse
            colorGroup: PlasmaCore.Theme.ButtonColorGroup
        }

        PlasmaComponents.Label {
            id: label
            text: control.text
            font: control.font
            width: parent.width - icon.width - parent.spacing
            height: parent.height
            color: theme.buttonTextColor
            horizontalAlignment: icon.valid ? Text.AlignLeft : Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            elide: Text.ElideRight
        }
    }

    background: Item {


        implicitHeight: Math.floor(Math.max(theme.mSize(theme.defaultFont).height*1.6, control.minimumHeight))

        implicitWidth: {
            if (control.text.length == 0) {
                height;
            } else {
                Math.max(theme.mSize(theme.defaultFont).width*12, control.minimumWidth);
            }
        }

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

        //TODO: create on demand?
        PlasmaCore.SvgItem {
            visible: control.menu !== null
            width: drowDownButtonWidth
            height: drowDownButtonWidth
            anchors {
                right: parent.right
                rightMargin: surfaceNormal.margins.right
                verticalCenter: parent.verticalCenter
            }
            svg: PlasmaCore.Svg { imagePath: "widgets/arrows" }
            elementId: "down-arrow"
        }
    }
}
