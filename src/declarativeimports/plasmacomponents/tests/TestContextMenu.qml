/*
    SPDX-FileCopyrightText: 2014 Sebastian Kügler <sebas@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.1
// import QtQuick.Layouts 1.1
//
// import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents

Item {
    id: root
    width: 300
    height: 400

    PlasmaComponents.ContextMenu {
        id: contextMenu

        PlasmaComponents.MenuItem {
            id: actionsSeparator

            separator: true
        }
        PlasmaComponents.MenuItem {
            id: enableDisable

            text: enabled ? "It's on!" : "Shut Down"
            icon: "view-sort-ascending"

            onClicked: {
                print("Clicked " + text);
            }
        }
        PlasmaComponents.MenuItem {
            id: sortFavoritesDescending

            text: "Just a random menu item"
            icon: "view-sort-descending"

            onClicked: {
                print("Clicked " + text);
            }
        }

        PlasmaComponents.MenuItem {
            id: blinker

            text: "Blinker: " + visible
            icon: "device-notifier"

            onVisibleChanged: {
                print(" vis: " + visible);
            }

            Timer {
                interval: 2000
                repeat: true
                running: true
                onTriggered: {
                    blinker.visible = !blinker.visible;
                    enableDisable.enabled = !enableDisable.enabled;

                }
            }

            onClicked: {
                print("Clicked " + text);
            }
        }
    }

    Rectangle {
        anchors.fill: parent

        color: "orange"

        MouseArea {
            anchors.fill: parent
            onClicked: {
                contextMenu.open(mouse.x, mouse.y);
            }
        }
    }


    Component.onCompleted: contextMenu.open();

}
