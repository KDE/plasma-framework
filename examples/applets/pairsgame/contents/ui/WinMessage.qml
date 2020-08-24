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
        x: 10
        y: 10
        text: "WIN!!!"
        font.pointSize: 52
    }

    Behavior on y {
        NumberAnimation {
            target: main
            property: "y"
            duration: 1000
            easing.type: "OutBounce"
            
        }
    }
}
