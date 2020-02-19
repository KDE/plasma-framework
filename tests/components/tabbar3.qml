import QtQuick 2.0

import org.kde.plasma.core 2.0 as PlasmaCore

import org.kde.plasma.components 3.0 as PlasmaComponents

ComponentBase {
    PlasmaComponents.TabBar {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 20

        PlasmaComponents.TabButton {
            icon.name: "application-menu"
            text: "Icon Only"
            display: PlasmaComponents.TabButton.IconOnly
        }
        PlasmaComponents.TabButton {
            icon.name: "application-menu"
            text: "Text Beside Icon"
            display: PlasmaComponents.TabButton.TextBesideIcon
        }
        PlasmaComponents.TabButton {
            icon.name: "application-menu"
            text: "Text Under Icon"
            display: PlasmaComponents.TabButton.TextUnderIcon
        }
        PlasmaComponents.TabButton {
            icon.name: "application-menu"
            text: "Text Only"
            display: PlasmaComponents.TabButton.TextOnly
        }
    }
}

