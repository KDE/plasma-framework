/*
 * SPDX-FileCopyrightText: 2020 Noah Davis <noahadvs@gmail.com>
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick 2.6
import org.kde.kirigami 2 as Kirigami

Item {
    id: root
    // These should be used as the padding for the parent control
    property real leftMargin: loader.item.leftMargin
    property real topMargin: loader.item.topMargin
    property real rightMargin: loader.item.rightMargin
    property real bottomMargin: loader.item.bottomMargin

    implicitWidth: Kirigami.Units.gridUnit + root.leftMargin + root.rightMargin
    implicitHeight: Kirigami.Units.gridUnit + root.topMargin + root.bottomMargin

    opacity: enabled ? 1 : 0.5
    layer.enabled: opacity < 1

    Loader {
        id: loader
        anchors.fill: parent
        sourceComponent: root.parent.flat ? flatButtonBackground : raisedButtonBackground
    }

    Component {
        id: flatButtonBackground
        FlatButtonBackground {
            anchors.fill: parent
            hovered: root.parent.hovered
            pressed: root.parent.down
            checked: root.parent.checked
            focused: root.parent.visualFocus
        }
    }

    Component {
        id: raisedButtonBackground
        RaisedButtonBackground {
            anchors.fill: parent
            hovered: root.parent.hovered
            pressed: root.parent.down
            checked: root.parent.checked
            focused: root.parent.visualFocus
        }
    }
}
