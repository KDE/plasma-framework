/*
    SPDX-FileCopyrightText: 2014 Vishesh Handa <vhanda@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.0

import QtQuick.Controls 2.15 as Controls
import QtQuick.Layouts 1.1

import org.kde.plasma.core 2.0 as PlasmaCore

ColumnLayout {
    Controls.Label {
        Layout.maximumWidth: mainLayout.width
        wrapMode: Text.WordWrap
        text: "Click on each coloured box to make a dialog popup. It should popup in the correct position. The popup should also move from one rectangle to the other on hovering"
    }

    RowLayout {
        id: mainLayout
        Rectangle {
            width: 300
            height: 100
            color: "red"

            MouseArea {
                anchors.fill: parent
                hoverEnabled: true
                onClicked: {
                    dialog.visualParent = parent;
                    dialog.visible = !dialog.visible;
                }
                onEntered: {
                    dialog.visualParent = parent;
                }
            }
        }

        Rectangle {
            width: 300
            height: 100
            color: "blue"

            MouseArea {
                anchors.fill: parent
                hoverEnabled: true
                onClicked: {
                    dialog.visualParent = parent;
                    dialog.visible = !dialog.visible;
                }
                onEntered: {
                    dialog.visualParent = parent;
                }
            }
        }

        Rectangle {
            width: 300
            height: 100
            color: "green"

            MouseArea {
                anchors.fill: parent
                hoverEnabled: true
                onClicked: {
                    dialog.visualParent = parent;
                    dialog.visible = !dialog.visible;
                }
                onEntered: {
                    dialog.visualParent = parent;
                }
            }
        }

        Rectangle {
            width: 300
            height: 100
            color: "yellow"

            MouseArea {
                anchors.fill: parent
                hoverEnabled: true
                onClicked: {
                    dialog.visualParent = parent;
                    dialog.visible = !dialog.visible;
                }
                onEntered: {
                    dialog.visualParent = parent;
                }
            }
        }

        PlasmaCore.Dialog {
            id: dialog
            location: PlasmaCore.Types.BottomEdge
            visible: false

            Rectangle {
                color: "black"
                width: 150
                height: 150
            }
        }
    }
}
