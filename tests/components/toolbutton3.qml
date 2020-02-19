import QtQuick 2.0
import QtQuick.Layouts 1.4

import org.kde.plasma.core 2.0 as PlasmaCore

import org.kde.plasma.components 3.0 as PlasmaComponents

ComponentBase {
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 20
        Flow {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 20

            PlasmaComponents.ToolButton {
                icon.name: "list-remove"
                text: "test"
                flat: true
            }
            PlasmaComponents.ToolButton {
                icon.name: "list-remove"
                flat: true
            }
            PlasmaComponents.ToolButton {
                text: "test"
                flat: true
            }
            PlasmaComponents.ToolButton {
                icon.name: "list-remove"
                text: "test"
                flat: false
            }
            PlasmaComponents.ToolButton {
                icon.name: "list-remove"
                flat: false
            }
            PlasmaComponents.ToolButton {
                text: "test"
                flat: false
            }
            PlasmaComponents.ToolButton {
                icon.name: "application-menu"
                text: "Icon Only"
                display: PlasmaComponents.ToolButton.IconOnly
            }
            PlasmaComponents.ToolButton {
                icon.name: "application-menu"
                text: "Text Beside Icon"
                display: PlasmaComponents.ToolButton.TextBesideIcon
            }
            PlasmaComponents.ToolButton {
                icon.name: "application-menu"
                text: "Text Under Icon"
                display: PlasmaComponents.ToolButton.TextUnderIcon
            }
            PlasmaComponents.ToolButton {
                icon.name: "application-menu"
                text: "Text Only"
                display: PlasmaComponents.ToolButton.TextOnly
            }
        }
        RowLayout {
            Layout.fillWidth: true
            PlasmaComponents.Label {
                Layout.fillWidth: true
                text: "They should always be square:"
            }
            PlasmaComponents.ToolButton {
                icon.name: "list-remove"
            }
            PlasmaComponents.ToolButton {
                icon.name: "list-remove"
            }
            PlasmaComponents.ToolButton {
                icon.name: "list-remove"
            }
            PlasmaComponents.ToolButton {
                icon.name: "list-remove"
            }
        }
    }
}

