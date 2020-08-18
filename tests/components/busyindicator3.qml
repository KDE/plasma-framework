import QtQuick 2.0
import QtQuick.Layouts 1.12
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 3.0 as PlasmaComponents

ComponentBase {
    id: root
    title: "Plasma Components 3 BusyIndicator"
    contentItem: ColumnLayout {
        spacing: PlasmaCore.Units.gridUnit

        PlasmaComponents.Label {
            wrapMode: Text.WordWrap
            text: "The BusyIndicator should disappear when unchecked and restart when checked."
            Layout.preferredWidth: Math.max(busyIndicatorLayout.width, root.implicitHeaderWidth)
        }

        RowLayout {
            id: busyIndicatorLayout
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
