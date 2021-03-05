/*
    SPDX-FileCopyrightText: 2013 Sebastian Kügler <sebas@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.extras 2.0 as PlasmaExtras
import org.kde.kquickcontrolsaddons 2.0 as KQuickControlsAddons

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
        PlasmaExtras.Heading {
            level: 2
            text: "I'm an applet"
        }

        PlasmaComponents.ButtonColumn {
            PlasmaComponents.RadioButton {
                text: "No background"
                onClicked: {
                    if (checked) plasmoid.backgroundHints = 0;
                }
            }
            PlasmaComponents.RadioButton {
                text: "Default background"
                checked: true
                onClicked: {
                    if (checked) plasmoid.backgroundHints = 1;
                }
            }
            PlasmaComponents.RadioButton {
                text: "Translucent background"
                onClicked: {
                    if (checked) plasmoid.backgroundHints = 2;
                }
            }
        }

        PlasmaComponents.Button {
            height: PlasmaCore.Units.iconSizes.desktop
            text: "Busy"
            checked: plasmoid.busy
            onClicked: {
                plasmoid.busy = !plasmoid.busy
            }
        }

        PlasmaComponents.Button {
            id: ctxButton
            height: PlasmaCore.Units.iconSizes.desktop
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

