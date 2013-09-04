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

import QtQuick 2.0
import org.kde.plasma.components 2.0

Image {
    property int minimumWidth: 300
    property int minimumHeight: 400
    source: "image://appbackgrounds/standard"
    fillMode: Image.Tile
    asynchronous: true

    ToolBar {
        id: toolBar
        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
        }
    }


    Image {
        id: sidebar
        source: "image://appbackgrounds/contextarea"
        fillMode: Image.Tile
        asynchronous: true
        width: 200
        z: 10
        anchors {
            top: toolBar.bottom
            bottom: parent.bottom
        }
        Image {
            source: "image://appbackgrounds/shadow-right"
            fillMode: Image.Tile
            anchors {
                left: parent.right
                top: parent.top
                bottom: parent.bottom
                leftMargin: -1
            }
        }

        ListView {
            id: pageSelector
            clip: true
            anchors.fill: parent

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
                onClicked: pageStack.replace(Qt.createComponent(plasmoid.file("ui", page)))
            }
        }
    }


    PageStack {
        id: pageStack
        toolBar: toolBar
        anchors {
            top: toolBar.bottom
            left: sidebar.right
            right: parent.right
            bottom: parent.bottom
        }
        initialPage: Qt.createComponent("Buttons.qml")
    }

}
