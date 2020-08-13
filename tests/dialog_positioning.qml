/*
    SPDX-FileCopyrightText: 2014 David Edmundson <davidedmundson@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
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
            height: layout.height
            anchors.centerIn: parent

            ColumnLayout {
                id: layout
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
