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
    height: childrenRect.height
    property int implicitHeight: childrenRect.height

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
    Column {
        spacing: 30
        Text {
            text: "Text Fields"
            font.pixelSize: 20
        }

        PlasmaComponents.Highlight {
            width: 200
            height: 100
            Column {
                spacing: 10
                Row {
                    Text {
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
                    Text {
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

        Text {
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
