import QtQuick 2.0

import org.kde.plasma.components 3.0 as PlasmaComponents

ComponentBase {
    id: root
    property string longText: "This is a longer sentence"

    Flow {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 20

        PlasmaComponents.TextField {
            placeholderText: longText
        }

        PlasmaComponents.TextField {
            text: root.longText
        }

        PlasmaComponents.TextField {
            width: 400
            placeholderText: longText
        }

        PlasmaComponents.TextField {
            text: root.longText
            echoMode: TextInput.Password
        }
    }
}
