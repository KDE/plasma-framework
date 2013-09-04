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
import org.kde.plasma.components 2.0 as PlasmaComponents


PlasmaComponents.Page {

    tools: PlasmaComponents.ToolBarLayout {
        spacing: 5
        PlasmaComponents.ToolButton {
            visible: pageStack.depth > 1
            iconSource: "go-previous"
            onClicked: pageStack.pop()
        }
        PlasmaComponents.CheckBox {
            text: "Checkbox in the toolbar"
        }
        PlasmaComponents.TextField {
            clearButtonShown: true
            text: "hello"
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
                    text: "Check Box"
                }

                PlasmaComponents.CheckBox {
                    width: 140
                    height: 30
                    text: "Check Box 1"

                    onCheckedChanged: {
                        if (checked)
                            console.log("CheckBox checked");
                        else
                            console.log("CheckBox unchecked");
                    }
                    onClicked: {
                        console.log("CheckBox clicked");
                    }
                }

                PlasmaComponents.CheckBox {
                    height: 30
                    text: "Disabled"
                    enabled: false
                }

                PlasmaComponents.CheckBox {
                    height: 30
                    text: ""
                }

                PlasmaComponents.CheckBox {
                    height: 30
                    text: "A loooooooooooooong text"
                }

                PlasmaComponents.Label {
                    font.pixelSize: 20
                    text: "Radio Button"
                }

                PlasmaComponents.RadioButton {
                    width: 140
                    height: 30
                    text: "RadioButton"

                    onCheckedChanged: {
                        if (checked)
                            console.log("RadioButton Checked");
                        else
                            console.log("RadioButton Unchecked");
                    }
                }

                PlasmaComponents.Switch { }

                PlasmaComponents.Label {
                    font.pixelSize: 20
                    text: "Button Row"
                }

                PlasmaComponents.ButtonRow {
                    spacing: 20
                    PlasmaComponents.RadioButton { text: "A" }
                    PlasmaComponents.RadioButton { text: "B" }
                    PlasmaComponents.RadioButton { text: "C" }
                }

                PlasmaComponents.Label {
                    font.pixelSize: 20
                    text: "Button Column"
                }

                PlasmaComponents.ButtonColumn {
                    spacing: 20
                    PlasmaComponents.RadioButton { text: "Alice" }
                    PlasmaComponents.RadioButton { text: "Bob" }
                    PlasmaComponents.RadioButton { text: "Charles" }
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
