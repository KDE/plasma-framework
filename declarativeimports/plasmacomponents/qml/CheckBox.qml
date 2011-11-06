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

/**Documentanted API
Inherits:
        DualStateButton
Imports:
        QtQuick 1.0
        org.kde.plasma.core

Description:
 TODO i need more info here

Properties:
**/

import QtQuick 1.0
import org.kde.plasma.core 0.1 as PlasmaCore

DualStateButton {
    id: checkBox
    view: PlasmaCore.FrameSvgItem {
        imagePath: "widgets/button"
        prefix: "normal"
        width: fontMetricText.height + margins.left
        height: fontMetricText.height + margins.top
        //FIXME: an hack to have font metrics: can we have a proper binding?
        Text {
            id: fontMetricText
            text: "M"
            visible: false
            font.capitalization: theme.defaultFont.capitalization
            font.family: theme.defaultFont.family
            font.italic: theme.defaultFont.italic
            font.letterSpacing: theme.defaultFont.letterSpacing
            font.pointSize: theme.defaultFont.pointSize
            font.strikeout: theme.defaultFont.strikeout
            font.underline: theme.defaultFont.underline
            font.weight: theme.defaultFont.weight
            font.wordSpacing: theme.defaultFont.wordSpacing
            color: theme.textColor
        }
        PlasmaCore.SvgItem {
            svg: PlasmaCore.Svg {
                id: checkmarkSvg
                imagePath: "widgets/checkmarks"
            }
            elementId: "checkbox"
            opacity: checked ? 1 : 0
            anchors {
                fill: parent
                margins: parent.margins.left/2
            }
            Behavior on opacity {
                NumberAnimation {
                    duration: 250
                    easing.type: Easing.InOutQuad
                }
            }
        }
    }

    shadow: ButtonShadow {}
}