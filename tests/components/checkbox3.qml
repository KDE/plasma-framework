import QtQuick 2.0

import org.kde.plasma.components 3.0 as PlasmaComponents
import QtQuick.Layouts 1.2

ComponentBase {
    Grid {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 4
        columns: 2

        PlasmaComponents.Label {
            text: "text"
        }
        PlasmaComponents.CheckBox {
            text: "Some awesome checkbox"
        }

        PlasmaComponents.Label {
            text: "icon"
        }
        PlasmaComponents.CheckBox {
            icon.name: "start-here-kde-plasma"
        }

        PlasmaComponents.Label {
            text: "text plus icon"
        }
        PlasmaComponents.CheckBox {
            text: "text"
            icon.name: "start-here-kde-plasma"
        }

        PlasmaComponents.Label {
            text: "focus"
        }
        PlasmaComponents.CheckBox {
            text: "Some awesome checkbox"
            focus: true
        }

        PlasmaComponents.Label {
            text: "checked"
        }
        PlasmaComponents.CheckBox {
            text: "Some awesome checkbox"
            checkState: Qt.Checked
        }

        PlasmaComponents.Label {
            text: "tri-state"
        }
        PlasmaComponents.CheckBox {
            text: "Some awesome checkbox"
            checkState: Qt.PartiallyChecked
        }

        PlasmaComponents.Label {
            text: "disabled"
        }
        PlasmaComponents.CheckBox {
            text: "Some awesome checkbox"
            enabled: false
        }

        PlasmaComponents.Label {
            text: "disabled and checked"
        }
        PlasmaComponents.CheckBox {
            text: "Some awesome checkbox"
            enabled: false
            checkState: Qt.Checked
        }
    }
}

