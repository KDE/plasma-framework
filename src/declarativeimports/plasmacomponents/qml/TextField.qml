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
import QtQuick.Controls.Styles.Plasma 2.0 as Styles
import org.kde.kconfig 1.0

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

    /*
     * Whether to show a button that allows the user to reveal the password in plain text
     * This only makes sense if the echoMode is set to Password.
     * @since 5.26
     */
    property bool revealPasswordButtonShown: false

    // this takes into account kiosk restriction
    readonly property bool __effectiveRevealPasswordButtonShown: revealPasswordButtonShown
                                                              && KAuthorized.authorize("lineedit_reveal_password")

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

    Row {
        anchors {
            right: textField.right
            rightMargin: 6
            verticalCenter: textField.verticalCenter
        }

        PlasmaCore.IconItem {
            id: showPasswordButton
            source: __effectiveRevealPasswordButtonShown ? (textField.echoMode === TextInput.Normal ? "hint" : "visibility") : ""
            height: Math.max(textField.height * 0.8, units.iconSizes.small)
            width: height
            opacity: (__effectiveRevealPasswordButtonShown && textField.enabled) ? 1 : 0
            visible: opacity > 0
            Behavior on opacity {
                NumberAnimation {
                    duration: units.longDuration
                    easing.type: Easing.InOutQuad
                }
            }
            MouseArea {
                anchors.fill: parent
                onClicked: {
                    textField.echoMode = (textField.echoMode == TextInput.Normal ? TextInput.Password : TextInput.Normal)
                    textField.forceActiveFocus()
                }
            }
        }

        PlasmaCore.IconItem {
            id: clearButton
            //ltr confusingly refers to the direction of the arrow in the icon, not the text direction which it should be used in
            source: clearButtonShown ? (LayoutMirroring.enabled ? "edit-clear-locationbar-ltr" : "edit-clear-locationbar-rtl") : ""
            height: Math.max(textField.height * 0.8, units.iconSizes.small)
            width: height
            opacity: (textField.length > 0 && clearButtonShown && textField.enabled) ? 1 : 0
            visible: opacity > 0
            Behavior on opacity {
                NumberAnimation {
                    duration: units.longDuration
                    easing.type: Easing.InOutQuad
                }
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
}
