import QtQuick 2.0
import QtQuick.Layouts 1.12
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents

ComponentBase {
    id: root
    title: "Plasma Components 2 ToolButton"

    contentItem: Flow {
        spacing: PlasmaCore.Units.gridUnit

        PlasmaComponents.ToolButton {
            iconSource: "start-here-kde-plasma"
            text: "test"
            flat: true
        }
        PlasmaComponents.ToolButton {
            iconSource: "start-here-kde-plasma"
            flat: true
        }
        PlasmaComponents.ToolButton {
            text: "test"
            flat: true
        }
        PlasmaComponents.ToolButton {
            iconSource: "start-here-kde-plasma"
            text: "test"
            flat: false
        }
        PlasmaComponents.ToolButton {
            iconSource: "start-here-kde-plasma"
            flat: false
        }
        PlasmaComponents.ToolButton {
            text: "test"
            flat: false
        }
    }
}

