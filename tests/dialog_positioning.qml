/*
 *   Copyright 2014 David Edmundson <davidedmundson@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as
 *   published by the Free Software Foundation; either version 2,
 *   or (at your option) any later version.
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

import QtQuick.Controls 1.1 as Controls
import QtQuick.Layouts 1.1

import org.kde.plasma.core 2.0 as PlasmaCore


/*This test is for checking PlasmaDialog visualParent and related function work
*To test move the window towards various edges and press the button.
The Red rectangle should always be on screen and on the right screen

*/

PlasmaCore.Dialog {

    type: windowIsDockControl.checked ? PlasmaCore.Dialog.Dock : PlasmaCore.Dialog.Normal
    visible: true

    Rectangle {
        color: "#ffffff"
        width: 300
        height: 300

        Rectangle {
            id: innerRect
            color: "#ddffdd"
            width: 200
            height: 200
            anchors.centerIn: parent

            ColumnLayout {
                anchors.margins: 5
                anchors.top: parent.top
                anchors.left:parent.left
                anchors.right:parent.right

                Controls.Label {
                    Layout.fillWidth: true
                    text: "alt + left click and move the window to various edges to test popup position"
                    wrapMode: Text.WordWrap
                }

                Controls.ComboBox {
                    id: alignmentControl
                    //order must match Location in plasma.h
                    model: ["Left", "Right", "Top", "Bottom"]
                    currentIndex: 0
                }

                Controls.CheckBox {
                    id: windowIsDockControl
                    text: "Window is a dock"
                }

                Controls.Button {
                    text: "Show Popup"
                    onClicked: {
                        dialog.visible = !dialog.visible
                        console.log(alignmentControl.currentIndex);
                    }
                }
            }

            PlasmaCore.Dialog
            {
                id: dialog
                visualParent: innerRect
                location: {
                    switch (alignmentControl.currentIndex) {
                        case 0:
                            return PlasmaCore.Types.LeftEdge
                        case 1:
                            return PlasmaCore.Types.RightEdge
                        case 2:
                            return PlasmaCore.Types.TopEdge
                        default:
                            return PlasmaCore.Types.BottomEdge
                    }
                }


                Rectangle {
                    color: "#FF0000"
                    width: 150
                    height: 150
                }

                Component.onCompleted: {
                    console.log(alignmentControl.currentIndex);
                    console.log(dialog.location);

                }
            }
        }
    }
}
