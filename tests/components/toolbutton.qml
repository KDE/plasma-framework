import QtQuick 2.0

import org.kde.plasma.components 2.0 as PlasmaComponents

ComponentBase {
    Flow {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 20

        PlasmaComponents.ToolButton {
            iconSource: "list-remove"
            text: "test"
            flat: true
        }
        PlasmaComponents.ToolButton {
            iconSource: "list-remove"
            flat: true
        }
        PlasmaComponents.ToolButton {
            text: "test"
            flat: true
        }
        PlasmaComponents.ToolButton {
            iconSource: "list-remove"
            text: "test"
            flat: false
        }
        PlasmaComponents.ToolButton {
            iconSource: "list-remove"
            flat: false
        }
        PlasmaComponents.ToolButton {
            text: "test"
            flat: false
        }
    }
}

