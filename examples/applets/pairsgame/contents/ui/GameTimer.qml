/*
    SPDX-FileCopyrightText: 2010 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 1.0

Rectangle {
    id:main
    property int time: 0
    property bool running: true

    color: Qt.rgba(1,1,1,0.7)
    radius: 10
    smooth: true

    width: childrenRect.width + 20
    height: childrenRect.height + 20
    Text {
        anchors.horizontalCenter: parent.horizontalCenter
        x: 10
        y: 10
        text: ""+Math.floor((time/60)/10)+Math.floor((time/60))+":"+Math.floor((time%60)/10)+(time%60)%10
        font.pointSize: 22
    }

    Timer {
        interval: 1000
        running: main.running
        repeat: true
        onTriggered: {
            time++
        }
    }
}
