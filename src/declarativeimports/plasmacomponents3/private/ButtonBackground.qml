/*
 * SPDX-FileCopyrightText: 2020 Noah Davis <noahadvs@gmail.com>
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick 2.6
import org.kde.plasma.core 2.0 as PlasmaCore

Item {
    id: root
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

    implicitWidth: PlasmaCore.Units.gridUnit + root.leftMargin + root.rightMargin
    implicitHeight: PlasmaCore.Units.gridUnit + root.topMargin + root.bottomMargin

    FlatButtonBackground {
        id: flatButtonBackground
        anchors.fill: parent
        hovered: root.parent.hovered
        pressed: root.parent.pressed
        checked: root.parent.checked
        focused: root.parent.visualFocus
    }

    RaisedButtonBackground {
        id: raisedButtonBackground
        anchors.fill: parent
        hovered: root.parent.hovered
        pressed: root.parent.pressed
        checked: root.parent.checked
        focused: root.parent.visualFocus
    }

    state: root.parent.flat ? "flat" : "raised"

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
