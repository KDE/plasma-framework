import QtQuick 2.0

import org.kde.plasma.components 3.0

ComponentBase {
    Flow {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 20
        ListModel {
            id: demoModel
            ListElement { text: "Banana"; color: "Yellow" }
            ListElement { text: "Apple"; color: "Green" }
            ListElement { text: "Coconut"; color: "Brown" }
        }

        ComboBox {
            model:demoModel
            textRole: "text"
        }
        ComboBox {
            editable: true
            model: demoModel
            textRole: "text"
        }
    }
}
