/*
*   Copyright (C) 2011 by Daker Fernandes Pinheiro <dakerfp@gmail.com>
*   Copyright (C) 2014 by Marco Martin <mart@kde.org>
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

import QtQuick 2.1
import QtQuick.Controls 1.2 as QtControls
import org.kde.plasma.core 2.0 as PlasmaCore
import "styles" as Styles


/**
 * A plasma theme based text field widget.
 * @inherit QtQuick.Controls.TextField
 */
QtControls.TextField {
    id: textField



    //Plasma api

    /**
     * Whether the button to clear the text from TextField is visible.
     */
    property bool clearButtonShown: false

    //Deprecated/unsupported api
    /**
     * type: string
     *
     * Allows to set a custom character for password fields
     * @warning Not implemented in Plasma components.
     */
    property string passwordCharacter

    /**
     * Whether the text field is highlighted or not
     *
     * If it is true then the problematic lines will be highlighted.
     *
     * @warning Not implemented in Plasma components.
     */
    property bool errorHighlight: false // TODO

    /**
     * DEPRECATED
     */
    function positionAt(pos) {
        print("DEPRECATED function");
    }

    /**
     * DEPRECATED
     */
    function positionToRectangle(pos) {
        print("DEPRECATED function");
    }


    style: Styles.TextFieldStyle {}

    PlasmaCore.IconItem {
        id: clearButton
        source: "edit-clear-locationbar-rtl"
        height: Math.max(parent.height*0.8, units.iconSizes.small)
        width: height
        opacity: (textField.text != "" && clearButtonShown && textField.enabled) ? 1 : 0
        Behavior on opacity {
            NumberAnimation {
                duration: units.longDuration
                easing.type: Easing.InOutQuad
            }
        }
        anchors {
            right: parent.right
            rightMargin: 6
            verticalCenter: textField.verticalCenter
        }
        MouseArea {
            anchors.fill: parent
            onClicked: {
                textField.text = ""
                textField.forceActiveFocus()
            }
        }
    }
}
