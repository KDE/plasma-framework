/*
 *   Copyright 2014 Vishesh Handa <vhanda@kde.org>
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
