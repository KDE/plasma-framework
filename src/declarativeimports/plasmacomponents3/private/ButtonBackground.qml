/*
 * SPDX-FileCopyrightText: 2020 Noah Davis <noahadvs@gmail.com>
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick 2.6
import QtQuick.Templates @QQC2_VERSION@ as T
import org.kde.plasma.core 2.0 as PlasmaCore

Item {
    id: root
    // TODO: mark this required when our minimum Qt version is new enough
    property T.AbstractButton parentControl

    // These should be used as the padding for the parent control
    property real leftMargin
    property real topMargin
    property real rightMargin
    property real bottomMargin
    /* FIXME: This is null when passed to the parent control's contentItem.
     * This is meant to be used to pass information about the current
     * appearance of the button to the contentItem of the Button or
     * ToolButton so that the icon and label can use the right color.
     */
    property string usedPrefix

    readonly property bool keyboardFocus: parentControl.activeFocus &&
        (parentControl.focusReason == Qt.TabFocusReason || parentControl.focusReason == Qt.BacktabFocusReason)

    FlatButtonBackground {
        id: flatButtonBackground
        anchors.fill: parent
        hovered: parentControl.hovered
        pressed: parentControl.pressed
        checked: parentControl.checked
        focused: root.keyboardFocus
    }

    RaisedButtonBackground {
        id: raisedButtonBackground
        anchors.fill: parent
        hovered: parentControl.hovered
        pressed: parentControl.pressed
        checked: parentControl.checked
        focused: root.keyboardFocus
    }

    state: parentControl.flat ? "flat" : "raised"

    states: [
        State {
            name: "flat"
            PropertyChanges {
                target: root
                leftMargin: flatButtonBackground.leftMargin
                topMargin: flatButtonBackground.topMargin
                rightMargin: flatButtonBackground.rightMargin
                bottomMargin: flatButtonBackground.bottomMargin
                usedPrefix: flatButtonBackground.usedPrefix
            }
            PropertyChanges {
                target: flatButtonBackground
                visible: true
            }
            PropertyChanges {
                target: raisedButtonBackground
                visible: false
            }
        },
        State {
            name: "raised"
            PropertyChanges {
                target: root
                leftMargin: raisedButtonBackground.leftMargin
                topMargin: raisedButtonBackground.topMargin
                rightMargin: raisedButtonBackground.rightMargin
                bottomMargin: raisedButtonBackground.bottomMargin
                usedPrefix: raisedButtonBackground.usedPrefix
            }
            PropertyChanges {
                target: flatButtonBackground
                visible: false
            }
            PropertyChanges {
                target: raisedButtonBackground
                visible: true
            }
        }
    ]
}
