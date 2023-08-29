/* SPDX-FileCopyrightText: 2023 Noah Davis <noahadvs@gmail.com>
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick 2.15
import org.kde.plasma.extras 2.0 as PlasmaExtras

Row {
    padding: 10
    spacing: 10
    PlasmaExtras.SegmentedProgressBar {
        id: bar
        anchors.verticalCenter: parent.verticalCenter
        segments: 5
        from: 0
        to: 100
        value: Number.fromLocaleString(locale, input.text)
        indeterminate: input.length < 1
    }
    TextInput {
        id: input
        anchors.verticalCenter: parent.verticalCenter
        text: "50"
        leftPadding: 10
        rightPadding: 10
        padding: 5
        selectByMouse: true
        Rectangle {
            z: -1
            anchors.top: parent.bottom
            height: 1
            width: parent.width
            color: "black"
        }
    }
}
