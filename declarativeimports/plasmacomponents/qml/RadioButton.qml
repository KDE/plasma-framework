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

/**Documented API
Inherits:
        The private DualStateButton

Imports:
        QtQuick 1.0
        org.kde.plasma.core

Description:
        A radio button component consists of a radio button and a line of text. Only one item in a list may be selected at a time. Once an item is selected, it can be deselected only by selecting another item. Initial item selection may be set at the list creation. If not set, the list is shown without a selection.

Properties:
        bool checked: If the button is checked, its checked property is true; otherwise false. The property is false by default.

        bool pressed: If the button is pressed, its pressed property is true.
            See also clicked.

        string text: The text is shown beside the check box. By default text is an empty string.
Signals:
        clicked():
            Emitted when the user clicked a mouse button over the checkbox (or tapped on the touch screen)
**/

import QtQuick 1.0
import org.kde.plasma.core 0.1 as PlasmaCore


//FIXME: this should be round, DualStateButton shouldn't draw the shadow
DualStateButton {
    id: radioButton
    view: PlasmaCore.SvgItem {
        svg: PlasmaCore.Svg {
            id: buttonSvg
            imagePath: "widgets/actionbutton"
        }
        elementId: "normal"
        width: theme.defaultFont.mSize.height + 6
        height: width

        PlasmaCore.SvgItem {
            svg: PlasmaCore.Svg {
                id: checkmarkSvg
                imagePath: "widgets/checkmarks"
            }
            elementId: "radiobutton"
            opacity: checked ? 1 : 0
            anchors {
                fill: parent
            }
            Behavior on opacity {
                NumberAnimation {
                    duration: 250
                    easing.type: Easing.InOutQuad
                }
            }
        }
    }

    shadow: RoundShadow {}
}