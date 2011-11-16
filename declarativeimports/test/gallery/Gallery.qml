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
import org.kde.plasma.components 0.1

Rectangle {
    width: 1000
    height: 800
    color: "lightgrey"

    ToolBar {
        id: toolBar
        z: 10
        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
        }
    }

    
    ListView {
        id: pageSelector
        width: 200
        anchors {
            top: toolBar.bottom
            bottom: parent.bottom
        }
        model:  ListModel {
            id: pagesModel
            ListElement {
                page: "Buttons.qml"
                title: "Buttons"
            }
            ListElement {
                page: "CheckableButtons.qml"
                title: "Checkable buttons"
            }
            ListElement {
                page: "Busy.qml"
                title: "Busy indicators"
            }
            ListElement {
                page: "Sliders.qml"
                title: "Sliders"
            }
            ListElement {
                page: "Scrollers.qml"
                title: "Scrollers"
            }
            ListElement {
                page: "Texts.qml"
                title: "Text elements"
            }
            ListElement {
                page: "Misc.qml"
                title: "Misc stuff"
            }
        }
        delegate: ListItem {
            enabled: true
            Column {
                Label {
                    text: title
                }
            }
            onClicked: pageStack.replace(Qt.createComponent(page))
        }
    }


    Flickable {
        id: page

        anchors {
            top: toolBar.bottom
            left: pageSelector.right
            right: parent.right
            bottom: parent.bottom
        }
        contentWidth: pageStack.currentPage.implicitWidth
        contentHeight: pageStack.currentPage.implicitHeight

        PageStack {
            id: pageStack
            toolBar: toolBar
            width: page.width
            height: currentPage.implicitHeight
            initialPage: Qt.createComponent("Buttons.qml")
        }

    }

    ScrollBar {
        id: horizontalScrollBar

	    stepSize: 30

        flickableItem: page
        orientation: Qt.Horizontal
        anchors {
            left: parent.left
            right: verticalScrollBar.left
            bottom: parent.bottom
        }
    }

    ScrollBar {
        id: verticalScrollBar

	    stepSize: 30

        orientation: Qt.Vertical
        flickableItem: page
        anchors {
            top: toolBar.bottom
            right: parent.right
            bottom: horizontalScrollBar.top
        }
    }
}
