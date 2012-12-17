/*
*   Copyright (C) 2011 by Daker Fernandes Pinheiro <dakerfp@gmail.com>
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
*   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

import QtQuick 1.0
import org.kde.plasma.core 0.1 as PlasmaCore
import "private" as Private

/**
 * A boolean toggle button with the visual representation of a "realistic"
 * switch with a movable toggle showing the state of the Switch. Generally
 * easier to use on touch devices than a CheckBox due to the larger surface
 * space and more evident state visualization.
 *
 * You can bind the Switch component, for example, to a feature that the
 * application has to enable or disable depending on the user's input.
 *
 * All elements of this component are defined in DualStateButton, its base component.
 */
Private.DualStateButton {
    id: switchItem

    view: PlasmaCore.FrameSvgItem {
        imagePath: "widgets/slider"
        prefix: "groove"
        width: height * 2
        height: Math.max(theme.defaultFont.mSize.height + margins.top + margins.bottom,
                         button.margins.top + button.margins.bottom)

        PlasmaCore.FrameSvgItem {
            id: highlight
            imagePath: "widgets/slider"
            prefix: "groove-highlight"
            anchors.fill: parent

            opacity: checked ? 1 : 0
            Behavior on opacity {
                PropertyAnimation { duration: 100 }
            }
        }

        PlasmaCore.FrameSvgItem {
            imagePath: "widgets/button"
            prefix: "shadow"
            anchors {
                fill: button
                leftMargin: -margins.left
                topMargin: -margins.top
                rightMargin: -margins.right
                bottomMargin: -margins.bottom
            }
        }

        PlasmaCore.FrameSvgItem {
            id: button
            imagePath: "widgets/button"
            prefix: "normal"
            anchors {
                top: parent.top
                bottom: parent.bottom
            }
            width: height
            x: checked ? width : 0
            Behavior on x {
                PropertyAnimation { duration: 100 }
            }
        }
    }
}

