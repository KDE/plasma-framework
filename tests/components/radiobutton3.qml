import QtQuick 2.0

import org.kde.plasma.components 2.0 as PlasmaComponents
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
        PlasmaComponents.RadioButton {
            text: "Some awesome radiobutton"
        }

        PlasmaComponents.Label {
            text: "focus"
        }
        PlasmaComponents.RadioButton {
            text: "Some awesome radiobutton"
            focus: true
        }

        PlasmaComponents.Label {
            text: "checked"
        }
        PlasmaComponents.RadioButton {
            text: "Some awesome radiobutton"
            checked: true
        }

        PlasmaComponents.Label {
            text: "disabled"
        }
        PlasmaComponents.RadioButton {
            text: "Some awesome radiobutton"
            enabled: false
        }

        PlasmaComponents.Label {
            text: "disabled and checked"
        }
        PlasmaComponents.RadioButton {
            text: "Some awesome radiobutton"
            enabled: false
            checked: true
        }
    }
}

