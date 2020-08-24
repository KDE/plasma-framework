import QtQuick 2.0
import QtQuick.Layouts 1.12
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0

ComponentBase {
    id: root
    title: "Plasma Components 2 ComboBox"
    contentItem: ColumnLayout {
        spacing: PlasmaCore.Units.largeSpacing
        ListModel {
            id: demoModel
            ListElement { text: "Banana"; color: "Yellow" }
            ListElement { text: "Apple"; color: "Green" }
            ListElement { text: "Coconut"; color: "Brown" }
        }

        ComboBox {
            model:demoModel
        }
        ComboBox {
            editable: true
            model: demoModel
        }
    }
}
