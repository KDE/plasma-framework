import QtQuick 2.0

import org.kde.plasma.components 2.0 as PlasmaComponents
import QtQuick.Controls 1.2
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
        PlasmaComponents.CheckBox {
            text: "Some awesome checkbox"
        }

        Label {
            text: "focus"
        }
        PlasmaComponents.CheckBox {
            text: "Some awesome checkbox"
            focus: true
        }

        Label {
            text: "checked"
        }
        PlasmaComponents.CheckBox {
            text: "Some awesome checkbox"
            checkedState: Qt.Checked
        }

        Label {
            text: "tri-state"
        }
        PlasmaComponents.CheckBox {
            text: "Some awesome checkbox"
            checkedState: Qt.PartiallyChecked
        }

        Label {
            text: "disabled"
        }
        PlasmaComponents.CheckBox {
            text: "Some awesome checkbox"
            enabled: false
        }

        Label {
            text: "disabled and checked"
        }
        PlasmaComponents.CheckBox {
            text: "Some awesome checkbox"
            enabled: false
            checkedState: Qt.Checked
        }
    }
}

