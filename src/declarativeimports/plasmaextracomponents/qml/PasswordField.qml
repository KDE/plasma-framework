// SPDX-FileCopyrightText: 2019 Carl-Lucien Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: LGPL-2.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import org.kde.plasma.extras 2.0 as PlasmaExtras

/**
 * This is a standard password text field.
 *
 * Example usage for the password field component:
 *
 * @code{.qml}
 * import org.kde.plasma.extras 2.0 as PlasmaExtras
 *
 * PlasmaExtras.PasswordField {
 *     id: passwordField
 *     onAccepted: {
 *         // check if passwordField.text is valid
 *     }
 * }
 * @endcode
 *
 * @since 5.93
 * @inherit org::kde::plasma::extras::ActionTextField
 * @author Carl Schwan <carl@carlschwan.eu>
 */
PlasmaExtras.ActionTextField {
    id: root

    /**
     * This property holds whether we show the clear text password.
     *
     * By default, it's false.
     * @since 5.93
     */
    property bool showPassword: false

    readonly property var p0: Shortcut {
        // Let's consider this shortcut a standard, it's also supported at least by su and sudo
        sequence: "Ctrl+Shift+U"
        onActivated: root.clear();
    }

    echoMode: root.showPassword ? TextInput.Normal : TextInput.Password
    placeholderText: i18nd("libplasma5", "Password")
    inputMethodHints: Qt.ImhNoAutoUppercase | Qt.ImhNoPredictiveText | Qt.ImhSensitiveData
    rightActions: QQC2.Action {
        icon.name: root.showPassword ? "password-show-off" : "password-show-on"
        onTriggered: root.showPassword = !root.showPassword
    }

    Keys.onPressed: {
        if (event.matches(StandardKey.Undo)) {
            // Disable undo action for security reasons
            // See QTBUG-103934
            event.accepted = true
        }
    }
}
