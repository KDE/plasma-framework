import QtQuick 2.0
import QtQuick.Layouts 1.12
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 3.0 as PlasmaComponents

// Run with qmlscene to use qqc2-desktop-style

ComponentBase {
    id: root
    title: "Plasma Components 3 Slider"
    contentItem: GridLayout {
        columnSpacing: PlasmaCore.Units.largeSpacing
        rowSpacing: PlasmaCore.Units.largeSpacing
        columns: 2

        PlasmaComponents.Label {
            text: "Horizontal slider"
        }
        PlasmaComponents.Slider {
            id: horizontalSlider
            from: minSpinBox.value
            to: maxSpinBox.value
            stepSize: stepSizeSpinBox.value
        }

        PlasmaComponents.Label {
            text: "Vertical slider"
        }
        PlasmaComponents.Slider {
            id: verticalSlider
            from: minSpinBox.value
            to: maxSpinBox.value
            stepSize: stepSizeSpinBox.value
            orientation: Qt.Vertical
        }

        PlasmaComponents.Label {
            text: "from: "
        }
        PlasmaComponents.SpinBox {
            id: minSpinBox
            value: 0
            from: -999
            to: 999
            editable: true
        }

        PlasmaComponents.Label {
            text: "to: "
        }
        PlasmaComponents.SpinBox {
            id: maxSpinBox
            value: 100
            from: -999
            to: 999
            editable: true
        }

        PlasmaComponents.Label {
            text: "stepSize: "
        }
        PlasmaComponents.SpinBox {
            id: stepSizeSpinBox
            value: 1
            to: 999
            editable: true
        }
    }
}
