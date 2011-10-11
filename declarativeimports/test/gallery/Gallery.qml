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

import QtQuick 1.0
import org.kde.plasma.components 0.1 as PlasmaComponents

Rectangle {
    width: 1000
    height: 800
    color: "lightgrey"

    PlasmaComponents.ToolBar {
        id: toolBar
        z: 10
        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
        }
        tools: toolbarA
    }
    Row {
        id: toolbarA
        visible: false
        spacing: 5
        PlasmaComponents.Button {
            text: "Switch toolbar"
            onClicked: toolBar.setTools(toolbarB, "push")
        }
        PlasmaComponents.Button {
            text: "button on first toolbar"
        }
    }
    Row {
        id: toolbarB
        visible: false
        spacing: 5
        PlasmaComponents.Button {
            text: "Switch toolbar"
            onClicked: toolBar.setTools(toolbarA, "pop")
        }
        PlasmaComponents.Button {
            text: "button on second toolbar"
        }
        PlasmaComponents.TextField {}
    }
    Flickable {
        id: page

        anchors {
            top: toolBar.bottom
            left: parent.left
            right: parent.right
            bottom: parent.bottom
        }
        contentWidth: 2200
        contentHeight: 1000

        Row {
            x: 30
            anchors {
                top: parent.top
                bottom: parent.bottom
                margins: 20
            }
            spacing: 30

            Buttons{ }

            CheckableButtons { }

            Busy { }

            Sliders { }

            Scrollers { }

            Texts { }
        }
    }

    PlasmaComponents.ScrollBar {
        id: horizontalScrollBar

	    stepSize: 30

        flickableItem: page
        animated: true
        anchors {
            left: parent.left
            right: verticalScrollBar.left
            bottom: parent.bottom
        }
    }

    PlasmaComponents.ScrollBar {
        id: verticalScrollBar

	    stepSize: 30

        orientation: Qt.Vertical
        flickableItem: page
        animated: true
        anchors {
            top: toolBar.bottom
            right: parent.right
            bottom: horizontalScrollBar.top
        }
    }
}
