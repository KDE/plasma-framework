/*
    SPDX-FileCopyrightText: 2013 Sebastian Kügler <sebas@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.0
import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.kquickcontrolsaddons 2.0 as KQuickControlsAddons
import org.kde.kirigami 2.20 as Kirigami

// PlasmoidPage

PlasmaComponents.Page {
    id: plasmoidPage
    anchors {
        fill: parent
        margins: _s
    }
    Column {
        anchors.centerIn: parent
        spacing: _s
        Kirigami.Heading {
            level: 2
            text: "I'm an applet"
        }

        PlasmaComponents.ButtonColumn {
            PlasmaComponents.RadioButton {
                text: "No background"
                onClicked: {
                    if (checked) Plasmoid.backgroundHints = 0;
                }
            }
            PlasmaComponents.RadioButton {
                text: "Default background"
                checked: true
                onClicked: {
                    if (checked) Plasmoid.backgroundHints = 1;
                }
            }
            PlasmaComponents.RadioButton {
                text: "Translucent background"
                onClicked: {
                    if (checked) Plasmoid.backgroundHints = 2;
                }
            }
        }

        PlasmaComponents.Button {
            height: Kirigami.Units.iconSizes.desktop
            text: "Busy"
            checked: Plasmoid.busy
            onClicked: {
                Plasmoid.busy = !Plasmoid.busy
            }
        }

        PlasmaComponents.Button {
            id: ctxButton
            height: Kirigami.Units.iconSizes.desktop
            text: "Context Menu"
            Loader {
                id: menuLoader
            }
            onClicked: {
                if (menuLoader.source == "") {
                   menuLoader.source = "TestMenu.qml"
                } else {
                    //menuLoader.source = ""
                }
                menuLoader.item.open(0, height);
            }
        }
    }
}

