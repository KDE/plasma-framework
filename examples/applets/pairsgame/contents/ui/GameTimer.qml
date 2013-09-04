/***************************************************************************
 *   Copyright 2010 by Marco Martin <mart@kde.org>                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

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
