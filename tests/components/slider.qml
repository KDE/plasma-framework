import QtQuick 2.0
import QtQuick.Layouts 1.12
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents

// Run with qmlscene to use qqc2-desktop-style

ComponentBase {
    id: root
    title: "Plasma Components 2 Slider"
    contentItem: GridLayout {
        columnSpacing: PlasmaCore.Units.largeSpacing
        rowSpacing: PlasmaCore.Units.largeSpacing
        columns: 2

        PlasmaComponents.Label {
            text: "Horizontal slider"
        }
        PlasmaComponents.Slider {
            id: horizontalSlider
            minimumValue: minTextField.text
            maximumValue: maxTextField.text
            stepSize: stepSizeTextField.text
        }

        PlasmaComponents.Label {
            text: "Vertical slider"
        }
        PlasmaComponents.Slider {
            id: verticalSlider
            minimumValue: minTextField.text
            maximumValue: maxTextField.text
            stepSize: stepSizeTextField.text
            orientation: Qt.Vertical
        }

        PlasmaComponents.Label {
            text: "from: "
        }
        PlasmaComponents.TextField {
            id: minTextField
            text: "0"
        }

        PlasmaComponents.Label {
            text: "to: "
        }
        PlasmaComponents.TextField {
            id: maxTextField
            text: "100"
        }

        PlasmaComponents.Label {
            text: "stepSize: "
        }
        PlasmaComponents.TextField {
            id: stepSizeTextField
            text: "1"
        }
    }
}
