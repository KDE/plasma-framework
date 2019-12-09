import QtQuick 2.0

import org.kde.plasma.components 2.0 as PlasmaComponents

import QtQuick.Controls 1.3

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
            text: root.longText
            clearButtonShown: true
        }

        PlasmaComponents.TextField {
            width: 400
            placeholderText: longText
        }

        PlasmaComponents.TextField {
            text: root.longText
            echoMode: TextInput.Password
            revealPasswordButtonShown: true
        }

        PlasmaComponents.TextField {
            text: longText
            echoMode: TextInput.Password
            revealPasswordButtonShown: true
            clearButtonShown: true
        }

        PlasmaComponents.TextField {
            text: longText
            LayoutMirroring.enabled: true
            LayoutMirroring.childrenInherit: true
            clearButtonShown: true
        }
    }
}
