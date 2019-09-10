import QtQuick 2.0

import org.kde.plasma.components 3.0 as PlasmaComponents

Rectangle
{
    width: 500
    height: 300
    color: "white"

    Flow {
        anchors.fill: parent
        anchors.margins: 20
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
    }
}

