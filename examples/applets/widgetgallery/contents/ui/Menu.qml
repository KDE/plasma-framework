/*
    SPDX-FileCopyrightText: 2011 Daker Fernandes Pinheiro <dakerfp@gmail.com>
    SPDX-FileCopyrightText: 2011 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.0
import org.kde.plasma.components 2.0

Page {
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
                page: "Typography.qml"
                title: "Typography"
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
            onClicked: pageStack.push(Qt.createComponent(page))
        }
    }
    ScrollBar {
        id: verticalScrollBar

        orientation: Qt.Vertical
        flickableItem: pageSelector
        anchors {
            top: parent.top
            right: parent.right
            bottom: parent.bottom
        }
    }
}
