/*
    SPDX-FileCopyrightText: 2014 Sebastian Kügler <sebas@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 
import QtQuick.Controls 

Text {

    font.pointSize: 22

    font.family: fontCheck.text
    font.weight: lightCheck.checked ? Font.Light : Font.Normal
    text: "Lesley 40:83 - (" + font.family + ")"
    height: paintedHeightCheck.checked ? paintedHeight : 22
    verticalAlignment: Text.AlignTop

    Rectangle {
        color: "yellow"
        visible: boxesCheck.checked

        height: 1
        width: paintedWidth

        anchors {
            verticalCenter: parent.verticalCenter
            left: parent.left
        }
    }
    Rectangle {
        color: "transparent"
        border.width: 1
        border.color: "green"
        visible: boxesCheck.checked

        height: parent.paintedHeight
        width: paintedWidth

        anchors {
            top: parent.top
            left: parent.left
        }
    }
}
