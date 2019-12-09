import QtQuick 2.0
import org.kde.plasma.components 3.0 as PlasmaComponents

ComponentBase {
    Column {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 20

        PlasmaComponents.Label {
            width: parent.width
            wrapMode: Text.WordWrap
            text: "When checking and unchecking the checkbox, " +
                  "the busy indicator should resume where it has " +
                  "paused and not glitch around"
        }

        Row {
            spacing: 20

            PlasmaComponents.BusyIndicator {
                running: runningButton.checked
            }

            PlasmaComponents.CheckBox {
                id: runningButton
                text: "Running"
            }
        }

    }

}
