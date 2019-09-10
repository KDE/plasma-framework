import QtQuick 2.0

import org.kde.plasma.components 2.0 as PlasmaComponents
import QtQuick.Layouts 1.2

Rectangle
{
    width: 500
    height: 500
    color: "white"

    Grid {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 4
        columns: 2

        Label {
            text: "text"
        }
        PlasmaComponents.RadioButton {
            text: "Some awesome radiobutton"
        }

        Label {
            text: "focus"
        }
        PlasmaComponents.RadioButton {
            text: "Some awesome radiobutton"
            focus: true
        }

        Label {
            text: "checked"
        }
        PlasmaComponents.RadioButton {
            text: "Some awesome radiobutton"
            checked: true
        }

        Label {
            text: "disabled"
        }
        PlasmaComponents.RadioButton {
            text: "Some awesome radiobutton"
            enabled: false
        }

        Label {
            text: "disabled and checked"
        }
        PlasmaComponents.RadioButton {
            text: "Some awesome radiobutton"
            enabled: false
            checked: true
        }
    }
}

