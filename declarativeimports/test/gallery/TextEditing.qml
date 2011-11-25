/*
*   Copyright (C) 2011 by Daker Fernandes Pinheiro <dakerfp@gmail.com>
*
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU Library General Public License as
*   published by the Free Software Foundation; either version 2, or
*   (at your option) any later version.
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

import QtQuick 1.1
import org.kde.plasma.components 0.1 as PlasmaComponents

PlasmaComponents.Page {
    height: editThing.height
    property int implicitHeight: childrenRect.height
    //scale: 1.25

    tools: PlasmaComponents.ToolBarLayout {
        spacing: 5
        PlasmaComponents.Label {
            text: "Text label:"
        }
        PlasmaComponents.ToolButton {
            text: "ToolButton"
        }
        PlasmaComponents.TextField {
            placeholderText: "Place holder text"
        }
        PlasmaComponents.TextField {
            text: "Text fields page"
        }
    }
    MouseArea {
        anchors.fill: editThing
        onClicked: editThing.forceActiveFocus();
    }

    Column {
        id: editThing
        anchors.centerIn: parent
        width: 300
        height: 300

        spacing: 12
        Item { height: 60; width: parent.width; }
        PlasmaComponents.TextField {
            placeholderText: "Try copy & paste"
            text: "The cat bites into the socks"
            width: parent.width
            clearButtonShown: true
        }
        PlasmaComponents.TextArea {
            width: parent.width
            height: 200
            placeholderText: "Touch copy & paste not implemented yet."
        }
    }
}
