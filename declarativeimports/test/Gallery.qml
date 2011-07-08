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
    width: 800
    height: 400
    color: "lightgrey"

    Flickable {
        id: page

        anchors.fill: parent
        contentWidth: 2200
        contentHeight: 600
        contentX: 500

        Row {
            x: 30
            anchors {
                top: parent.top
                bottom: parent.bottom
                margins: 20
            }
            spacing: 30

            Column {
                spacing: 20

                Text {
                    font.pixelSize: 20
                    text: "Buttons"
                }

                PlasmaComponents.Button {
                    width: 140
                    height: 30
                    text: "Button"

                    onClicked: {
                        console.log("Clicked");
                    }
                }

                PlasmaComponents.Button {
                    width: 140
                    height: 30
                    text: "Checkable Button"
                    checkable: true

                    onCheckedChanged: {
                        if (checked)
                            console.log("Button Checked");
                        else
                            console.log("Button Unchecked");
                    }
                }

                PlasmaComponents.Button {
                    width: 140
                    height: 30
                    text: "Different Font"
                    font {
                        pixelSize: 20
                        family: "Helvetica"
                    }
                }

                PlasmaComponents.Button {
                    width: 140
                    height: 30
                    text: "Icon Button"
                    iconSource: "/home/dakerfp/work/comics-reader/ui/images/random.png"
                }

                PlasmaComponents.Button {
                    width: 140
                    height: 30
                    iconSource: "/home/dakerfp/work/comics-reader/ui/images/random.png"
                }
            }
            Column {
                spacing: 20

                Text {
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
                    text: "Check Box 2"
                }

                PlasmaComponents.CheckBox {
                    height: 30
                    text: ""
                }

                PlasmaComponents.CheckBox {
                    height: 30
                    text: "A loooooooooooooong text"
                }

                Text {
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
            }
            Column {
                spacing: 20

                Text {
                    font.pixelSize: 20
                    text: "BusyIndicator"
                }

                PlasmaComponents.BusyIndicator { }

                PlasmaComponents.BusyIndicator { running: true }

                PlasmaComponents.BusyIndicator {
                    id: busy
                    running: mouse.pressed
                    smoothAnimation: true
                    MouseArea {
                        id: mouse
                        anchors.fill: parent
                    }
                }
            }
            Column {
                spacing: 20

                Text {
                    font.pixelSize: 20
                    text: "Slider"
                }

                PlasmaComponents.Slider {
                    id: slider1
                    width: 140
                    height: 20
                    animated: true
                    Component.onCompleted: {
                        forceActiveFocus();
                    }
                }

                Text {
                    text: slider1.value
                }

                PlasmaComponents.Slider {
                    id: slider2
                    width: 20
                    height: 140
                    orientation: Qt.Vertical
                    minimumValue: 10
                    maximumValue: 1000
                    stepSize: 50
                    inverted: true
                    animated: true
                }

                Text {
                    text: slider2.value
                }
            }
            Column {
                spacing: 20

                Text {
                    font.pixelSize: 20
                    text: "Scroll Bar"
                }

                ListView {
                    id: scrollList

                    width: 200
                    height: 200
                    clip: true
                    model: 100
                    delegate: Text {
                        width: 200
                        height: 18
                        text: index
                    }

                    Rectangle {
                        anchors.fill: parent
                        color: "grey"
                        opacity: 0.3
                    }

                    PlasmaComponents.ScrollBar {
                        orientation: Qt.Vertical
                        flickableItem: scrollList
                        animated: true
                        stepSize: 40
                        scrollButtonInterval: 50
                        anchors {
                            top: scrollList.top
                            right: scrollList.right
                            bottom: scrollList.bottom
                        }
                    }
                }
            }

            Column {
                id: listColumn
                spacing: 20

                Text {
                    font.pixelSize: 20
                    text: "ListItemView"
                }

                PlasmaComponents.ListItemView {
                    id: listItemView

                    property bool hoverEnabled: hoverCheck.checked

                    width: 200
                    height: 200
                    clip: true
                    model: 15
                    scrollVisible: false
                    delegate: PlasmaComponents.ListItem {
                        view: listItemView
                        height: 30
                        hoverEnabled: view.hoverEnabled
                        Text {
                            id: label
                            anchors.fill: parent
                            anchors.margins: 4
                            text: index
                            font.pixelSize: 14
                        }
                        onSelected: {
                            listItemView.currentIndex = index;
                        }
                    }

                    Component.onCompleted: currentIndex = 3;
                }

                PlasmaComponents.CheckBox {
                    id: hoverCheck
                    text: "Hover Enabled"
                    checked: true
                }
            }
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
            top: parent.top
            right: parent.right
            bottom: horizontalScrollBar.top
        }
    }
}
