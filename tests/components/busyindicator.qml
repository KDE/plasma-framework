import QtQuick 2.0
import QtQuick.Layouts 1.12
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents

ComponentBase {
    id: root
    title: "Plasma Components 2 BusyIndicator"
    contentItem: ColumnLayout {
        spacing: PlasmaCore.Units.gridUnit

        PlasmaComponents.Label {
            wrapMode: Text.WordWrap
            text: "When checking and unchecking the checkbox, " +
                  "the busy indicator should resume where it has " +
                  "paused and not glitch around."
        }

        RowLayout {
            spacing: PlasmaCore.Units.gridUnit

            PlasmaComponents.BusyIndicator {
                running: runningButton.checked
            }

            PlasmaComponents.CheckBox {
                id: runningButton
                text: "Running"
                checked: true
            }
        }
    }
}
