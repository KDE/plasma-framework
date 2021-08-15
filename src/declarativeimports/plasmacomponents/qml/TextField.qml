/*
    SPDX-FileCopyrightText: 2011 Daker Fernandes Pinheiro <dakerfp@gmail.com>
    SPDX-FileCopyrightText: 2014 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
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
                                                              && (echoMode == TextInput.Normal || textField.length > 0)

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
            source: __effectiveRevealPasswordButtonShown ? (textField.echoMode === TextInput.Normal ? "visibility": "hint") : ""
            height: PlasmaCore.Units.iconSizes.small
            width: height
            opacity: (__effectiveRevealPasswordButtonShown && textField.enabled) ? 1 : 0
            visible: opacity > 0
            Behavior on opacity {
                NumberAnimation {
                    duration: PlasmaCore.Units.longDuration
                    easing.type: Easing.InOutQuad
                }
            }
            MouseArea {
                anchors.fill: parent
                enabled: __effectiveRevealPasswordButtonShown
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
            height: PlasmaCore.Units.iconSizes.small
            width: height
            opacity: (textField.length > 0 && clearButtonShown && textField.enabled) ? 1 : 0
            visible: opacity > 0
            Behavior on opacity {
                NumberAnimation {
                    duration: PlasmaCore.Units.longDuration
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
