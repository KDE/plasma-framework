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
import org.kde.plasma.components 0.1
import org.kde.plasma.extras 0.1 as PlasmaExtras

PlasmaExtras.App {
    id: app
    width: 1000
    height: 800

    property string formFactor: "tablet"

    navigation: pageSelector
    content: page

    PlasmaExtras.ResourceInstance {
        uri: "http://api.kde.org/4.x-api/plasma-qml-apidocs/"
        title: "Plasma Components API documentation"
    }

    ListView {
        id: pageSelector
        width: 200
        anchors {
            fill: parent
            top: content.top
            bottom: parent.bottom
            left: parent.left
            right: parent.right
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
                page: "TextEditing.qml"
                title: "Text Editing"
            }
            ListElement {
                page: "Misc.qml"
                title: "Misc stuff"
            }
            ListElement {
                page: "Extras.qml"
                title: "Extras"
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
        clip: true
        width: app.contentWidth

//         anchors {
//             top: toolBar.bottom
//             left: pageSelector.right
//             right: verticalScrollBar.left
//             bottom: horizontalScrollBar.top
//             leftMargin: 5
//             rightMargin: 5
//             topMargin: 5
//             bottomMargin: 5
//         }
        anchors.fill: parent
        contentWidth: app.contentWidth
        contentHeight: pageStack.currentPage.implicitHeight

        PageStack {
            id: pageStack
            //toolBar: app.toolBar
            //width: page.width
            width: contentWidth
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
            top: content.top
            right: parent.right
            bottom: horizontalScrollBar.top
        }
    }
}
