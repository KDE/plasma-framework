/*
    SPDX-FileCopyrightText: 2011 Daker Fernandes Pinheiro <dakerfp@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.0
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.extras 2.0 as PlasmaExtras

PlasmaComponents.Page {
    height: childrenRect.height
    property int implicitHeight: childrenRect.height

    tools: PlasmaComponents.ToolBarLayout {
        spacing: 5
        PlasmaComponents.ToolButton {
            visible: pageStack.depth > 1
            iconSource: "go-previous"
            onClicked: pageStack.pop()
        }
        PlasmaComponents.Label {
            text: "Text label:"
        }
        PlasmaComponents.TextField {
            placeholderText: "Place holder text"
        }
        PlasmaComponents.TextField {
            clearButtonShown: true
            text: "Text fields page"
        }
    }

    Flickable {
        id: flickable
        contentWidth: column.width
        contentHeight: column.height
        clip: true
        anchors.fill: parent

        Item {
            width: Math.max(flickable.width, column.width)
            height: column.height
            Column {
                id: column
                spacing: 20
                anchors.horizontalCenter: parent.horizontalCenter

                PlasmaComponents.Label {
                    text: "Text Fields"
                    font.pixelSize: 20
                }

                PlasmaExtras.Highlight {
                    width: 200
                    height: 100
                    Column {
                        spacing: 10
                        Row {
                            PlasmaComponents.Label {
                                text: "Username: "
                                anchors.verticalCenter: tf1.verticalCenter
                            }
                            PlasmaComponents.TextField {
                                id: tf1
                                placeholderText: "login"
                                Keys.onTabPressed: tf2.forceActiveFocus();
                            }
                        }

                        Row {
                            PlasmaComponents.Label {
                                text: "Password: "
                                anchors.verticalCenter: tf2.verticalCenter
                            }
                            PlasmaComponents.TextField {
                                id: tf2
                                width: 120
                                echoMode: TextInput.Password
                                Keys.onTabPressed: loginButton.forceActiveFocus();
                            }
                        }

                        PlasmaComponents.Button {
                            id: loginButton
                            text: "Login"
                            anchors {
                                right: parent.right
                                rightMargin: 0
                            }
                            width: 100
                        }
                    }
                }

                PlasmaComponents.TextField {
                    width: 120
                    placeholderText: "Disabled Text Field"
                    Keys.onTabPressed: loginButton.forceActiveFocus();
                    enabled: false
                }

                PlasmaComponents.Label {
                    text: "Text Area"
                    font.pixelSize: 20
                }

                PlasmaComponents.TextArea {
                    width: 200
                    height: 200
                    placeholderText: "Lorem ipsum et dolor"
                    wrapMode: TextEdit.WordWrap
                    contentMaxWidth: 400
                    contentMaxHeight: 400
                }

                PlasmaComponents.TextArea {
                    width: 200
                    height: 100
                    enabled: false
                    text: "Disabled Text Area"
                }
            }
        }
    }

    PlasmaComponents.ScrollBar {
        id: horizontalScrollBar

        flickableItem: flickable
        orientation: Qt.Horizontal
        anchors {
            left: parent.left
            right: verticalScrollBar.left
            bottom: parent.bottom
        }
    }

    PlasmaComponents.ScrollBar {
        id: verticalScrollBar

        orientation: Qt.Vertical
        flickableItem: flickable
        anchors {
            top: parent.top
            right: parent.right
            bottom: horizontalScrollBar.top
        }
    }
}
