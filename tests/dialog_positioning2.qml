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
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.extras 2.0 as PlasmaExtras

Item {
    width: 400
    height: 400

    PlasmaComponents.Button {
        id: settingsButton
        iconSource: "configure"
        text: "More Settings..."
        Layout.preferredWidth: 100
        onClicked: {
            contextMenu.visible = !contextMenu.visible;
        }
    }

    PlasmaCore.Dialog {
        id: contextMenu
        visualParent: settingsButton

        //location: plasmoid.location
        type: PlasmaCore.Dialog.PopupMenu
        flags: Qt.Popup | Qt.FramelessWindowHint | Qt.WindowDoesNotAcceptFocus

        mainItem: ColumnLayout {
            id: menuColumn
            Layout.minimumWidth: menuColumn.implicitWidth
            Layout.minimumHeight: menuColumn.implicitHeight
            spacing: units.smallSpacing

            PlasmaExtras.Heading {
                level: 3
                text: "Panel Alignment"
            }

            PlasmaComponents.ButtonColumn {
                spacing: 0
                Layout.fillWidth: true
                PlasmaComponents.ToolButton {
                    anchors {
                        left: parent.left
                        right: parent.right
                    }
                    text: "Left"
                    checkable: true
                    flat: false
                }
                PlasmaComponents.ToolButton {
                    anchors {
                        left: parent.left
                        right: parent.right
                    }
                    text: "Center"
                    checkable: true
                    flat: false
                }
            }

            PlasmaExtras.Heading {
                level: 3
                text: "Visibility"
            }

            PlasmaComponents.ButtonColumn {
                spacing: 0
                Layout.fillWidth: true
                PlasmaComponents.ToolButton {
                    anchors {
                        left: parent.left
                        right: parent.right
                    }
                    text: "Always Visible"
                    checkable: true
                    flat: false
                }
                PlasmaComponents.ToolButton {
                    anchors {
                        left: parent.left
                        right: parent.right
                    }
                    text: "Auto Hide"
                    checkable: true
                    flat: false
                }
            }
            PlasmaComponents.ToolButton {
                Layout.fillWidth: true
                text: "Maximize Panel"
                iconSource: "zoom-fit-height"
            }
        }
    }
}
