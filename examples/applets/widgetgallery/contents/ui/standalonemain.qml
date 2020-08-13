/*
    SPDX-FileCopyrightText: 2011 Daker Fernandes Pinheiro <dakerfp@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.0
import QtQuick.Layouts 1.1
import org.kde.plasma.core 2.0
import org.kde.plasma.components 2.0

FrameSvgItem {
    width: 300
    height: 400
    imagePath: "widgets/background"


    ToolBar {
        id: toolBar
        z: 10

        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
            margins: 10
        }
    }

    PageStack {
        id: pageStack
        toolBar: toolBar
        clip: true
        anchors {
            top: toolBar.bottom
            left: parent.left
            right: parent.right
            bottom: parent.bottom
            margins: 10
        }
        initialPage: Qt.createComponent("Menu.qml")
    }
}
