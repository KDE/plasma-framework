/*
    SPDX-FileCopyrightText: 2011 Daker Fernandes Pinheiro <dakerfp@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.0
import QtQuick.Controls 1.2 as QtControls
import org.kde.plasma.components 2.0 as PlasmaComponents

PlasmaComponents.Page {
    height: childrenRect.height
    tools: PlasmaComponents.ToolBarLayout {
        spacing: 5
        PlasmaComponents.ToolButton {
            visible: pageStack.depth > 1
            iconSource: "go-previous"
            onClicked: pageStack.pop()
        }
        PlasmaComponents.Button {
            text: "Button"
        }
        PlasmaComponents.TextField {
            clearButtonShown: true
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
                    font.pixelSize: 20
                    text: "Buttons"
                }

                PlasmaComponents.Button {
                    id: bt1

                    text: "Button"

                    onClicked: {
                        console.log("Clicked");
                        pageStack.push(Qt.createComponent("Scrollers.qml"))
                    }

                    Keys.onTabPressed: bt2.forceActiveFocus();
                }

                PlasmaComponents.Button {
                    id: bt2

                    text: "Checkable Button"
                    checkable: true

                    onCheckedChanged: {
                        if (checked)
                            console.log("Button Checked");
                        else
                            console.log("Button Unchecked");
                    }

                    Keys.onTabPressed: bt3.forceActiveFocus();
                }

                PlasmaComponents.Button {
                    id: bt3

                    text: "Different Font"
                    font {
                        pointSize: 20
                        family: "Helvetica"
                    }

                    Keys.onTabPressed: bt4.forceActiveFocus();
                }

                PlasmaComponents.Button {
                    id: bt4

                    text: "Icon Button"
                    iconSource: "konqueror"

                    Keys.onTabPressed: bt5.forceActiveFocus();
                    menu: QtControls.Menu {
                        QtControls.MenuItem { text: "This Button" }
                        QtControls.MenuItem { text: "Happens To Have" }
                        QtControls.MenuItem { text: "A Menu Assigned" }
                    }
                }

                PlasmaComponents.Button {
                    id: bt5

                    iconSource: "plasma"

                    Keys.onTabPressed: bt1.forceActiveFocus();
                }

                PlasmaComponents.Button {

                    text: "Disabled Button"
                    enabled: false
                }

                PlasmaComponents.ToolButton {
                    text: "ToolButton"
                }

                PlasmaComponents.ToolButton {
                    text: "ToolButton not flat"
                    flat: false
                }

                PlasmaComponents.ToolButton {

                    text: "Icon ToolButton"
                    iconSource: "konqueror"
                }

                PlasmaComponents.ToolButton {
                    iconSource: "plasma"
                }
                PlasmaComponents.ToolButton {
                    iconSource: "plasma"
                    flat: false
                }

                PlasmaComponents.ToolButton {
                    text: "Disabled ToolButton"
                    enabled: false
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
