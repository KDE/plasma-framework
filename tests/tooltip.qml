/*
 *   Copyright 2014 David Edmundson <davidedmundson@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as
 *   published by the Free Software Foundation; either version 2,
 *   or (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

import QtQuick 2.2

import org.kde.plasma.core 2.0 as PlasmaCore

Rectangle {
    color: "white"
    width: 600
    height: 800

    Column {
        anchors.centerIn: parent
        spacing: 4

        PlasmaCore.ToolTipArea {
            width: 300
            height: 50

            mainText: "This is some really really really really long text that should be truncated"
            subText: "subtext"

            Rectangle {
                color: "red"
                anchors.fill: parent
            }

            Text {
                anchors.fill: parent
                text: "long tooltip"
            }

        }

        PlasmaCore.ToolTipArea {
            width: 300
            height: 50

            mainText: "A"
            subText: "B"

            Rectangle {
                color: "red"
                anchors.fill: parent
            }

            Text {
                anchors.fill: parent
                text: "short tooltip"
            }
        }

        PlasmaCore.ToolTipArea {
            width: 300
            height: 50

            mainText: "A"
            subText: "Lorem ipsum dolor sit amet, consectetur adipiscing spaghetti italiano random cheesecake blah blah"

            Rectangle {
                color: "red"
                anchors.fill: parent
            }

            Text {
                anchors.fill: parent
                text: "long subtext"
            }
        }

    }
}

