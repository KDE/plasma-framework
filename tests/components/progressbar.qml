import QtQuick 2.0

import org.kde.plasma.components 2.0 as PlasmaComponents

ComponentBase {
    id: root
    property int orientation: orientationCombo.model[orientationCombo.currentIndex].value

    Flow {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 20

        Column {
            Text {
                text: "0%"
            }
            PlasmaComponents.ProgressBar {
                minimumValue: 0
                maximumValue: 100
                value: 0
                orientation: root.orientation
            }
        }

        Column {
            Text {
                text: "50%"
            }
            PlasmaComponents.ProgressBar {
                minimumValue: 0
                maximumValue: 100
                value: 50
                orientation: root.orientation
            }
        }

        Column {
            Text {
                text: "100%"
            }
            PlasmaComponents.ProgressBar {
                minimumValue: 0
                maximumValue: 100
                value: 100
                orientation: root.orientation
            }
        }

        Column {
            Text {
                text: "these main bars should look the same width"
            }
            Flow {
                PlasmaComponents.ProgressBar {
                    width: orientation === Qt.Vertical ? undefined : 100
                    height: orientation === Qt.Vertical ? 100 : undefined
                    minimumValue: 0
                    maximumValue: 100
                    value: 50
                    orientation: root.orientation
                }
                PlasmaComponents.Slider {
                    width: orientation === Qt.Vertical ? undefined : 100
                    height: orientation === Qt.Vertical ? 100 : undefined
                    minimumValue: 0
                    maximumValue: 100
                    value: 50
                    orientation: root.orientation
                }
            }
        }

        Column {
            Text {
                text: "Make sure the bar does\nnot leak outside"
            }
            PlasmaComponents.ProgressBar {
                minimumValue: 0
                maximumValue: 200
                value: 1
                orientation: root.orientation
            }
        }

        Column {
            Text {
                text: "should look like 100%"
            }
            PlasmaComponents.ProgressBar {
                minimumValue: 0
                maximumValue: 100
                value: 110
                orientation: root.orientation
            }
        }

        Column {
            Text {
                text: "QA Style 50%"
            }
            PlasmaComponents.ProgressBar {
                minimumValue: -100
                maximumValue: 100
                value: 0
                orientation: root.orientation
            }
        }

        Column {
            Text {
                text: "should look like 0%"
            }
            PlasmaComponents.ProgressBar {
                minimumValue: 0
                maximumValue: 100
                value: -10
                orientation: root.orientation
            }
        }

        Column {
            Text {
                text: "should be a continuous movement,<br>from one end to the other and back"
            }
            PlasmaComponents.ProgressBar {
                indeterminate: indeterminateCheckBox.checked
                value: 0.5
                orientation: root.orientation
            }
        }

        Column {
            Text {
                text: "Checking and unchecking should not break the layout,<br>should look like 50% if unchecked"
            }
            PlasmaComponents.CheckBox {
                id: indeterminateCheckBox
                text: "Indeterminate"
                checked: true
            }
        }

        Column {
            Text {
                text: "Slider orientation"
            }
            PlasmaComponents.ComboBox {
                id: orientationCombo
                model: [
                    {text: "Horizontal", value: Qt.Horizontal},
                    {text: "Vertical", value: Qt.Vertical}
                ]
            }
        }
    }
}
