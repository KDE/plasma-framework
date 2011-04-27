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

Rectangle {
    width: 600
    height: 400
    color: "lightgrey"

    Flickable {
        id: buttonPage

        anchors.fill: parent
        contentWidth: 1000

        Row {
            x: 30
            anchors {
                top: parent.top
                bottom: parent.bottom
                margins: 20
            }

            Column {
                width: 200
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
            }
            Column {
                spacing: 20

                Text {
                    font.pixelSize: 20
                    text: "BusyIndicator"
                }


                PlasmaComponents.BusyIndicator { }

                PlasmaComponents.BusyIndicator { running: true }
            }
        }
    }
}