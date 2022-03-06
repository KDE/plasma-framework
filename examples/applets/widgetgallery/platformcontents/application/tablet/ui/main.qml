/*
    SPDX-FileCopyrightText: 2011 Daker Fernandes Pinheiro <dakerfp@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.0
import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.components 2.0

Image {
    Layout.minimumWidth: 300
    Layout.minimumHeight: 400
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
                onClicked: pageStack.replace(Qt.createComponent(Plasmoid.file("ui", page)))
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
