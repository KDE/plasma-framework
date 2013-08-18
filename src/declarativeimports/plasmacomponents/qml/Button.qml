/*
*   Copyright (C) 2011 by Daker Fernandes Pinheiro <dakerfp@gmail.com>
*   Copyright (C) 2011 by Mark Gaiser <markg85@gmail.com>
*   Copyright (C) 2011 by Marco Martin <mart@kde.org>
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


/**
 * A button with optional label and icon which uses the plasma theme.
 *
 * This button component can also be used as a checkable button by using
 * the checkable and checked properties for that.  Plasma theme is the
 * theme which changes via the systemsetting-workspace appearance -desktop
 * theme.
 */
import QtQuick 2.0
import QtQuick.Controls 1.0 as QtControls
import QtQuick.Controls.Styles 1.0 as QtControlsStyle


import org.kde.plasma.core 2.0 as PlasmaCore
import "private" as Private

QtControls.Button {
    style: QtControlsStyle.ButtonStyle {
        background: Item {
            state: (control.pressed || control.checked) ? "pressed" : "normal"
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
            Private.ButtonShadow {
                id: shadow
                anchors.fill: parent
                state: {
                    if (control.pressed || control.checked) {
                        return "hidden"
                    } else if (control.hovered) {
                        return "hover"
                    } else if (control.activeFocus) {
                        //Surely this is wrong, if it's checked we'd never see if it is focussed. is Copy + Pasta from previous code - Dave
                        return "focus"
                    } else {
                        return "shadow"
                    }
                }
            }
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
                    // Cross fade from pressed to normal
                    ParallelAnimation {
                        NumberAnimation { target: surfaceNormal; property: "opacity"; to: 1; duration: 100 }
                        NumberAnimation { target: surfacePressed; property: "opacity"; to: 0; duration: 100 }
                    }
                }
            ]
        }
    }
}
