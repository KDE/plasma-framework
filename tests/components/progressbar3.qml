import QtQuick 2.0

import org.kde.plasma.components 3.0 as PlasmaComponents

ComponentBase {
    id: root

    Flow {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 20

        Column {
            Text {
                text: "0%"
            }
            PlasmaComponents.ProgressBar {
                from: 0
                to: 100
                value: 0
            }
        }

        Column {
            Text {
                text: "50%"
            }
            PlasmaComponents.ProgressBar {
                from: 0
                to: 100
                value: 50
            }
        }

        Column {
            Text {
                text: "100%"
            }
            PlasmaComponents.ProgressBar {
                from: 0
                to: 100
                value: 100
            }
        }

        Column {
            Text {
                text: "Make sure the bar does\nnot leak outside"
            }
            PlasmaComponents.ProgressBar {
                from: 0
                to: 200
                value: 1
            }
        }

        Column {
            Text {
                text: "should look like 100%"
            }
            PlasmaComponents.ProgressBar {
                from: 0
                to: 100
                value: 110
            }
        }

        Column {
            Text {
                text: "QA Style 50%"
            }
            PlasmaComponents.ProgressBar {
                from: -100
                to: 100
                value: 0
            }
        }

        Column {
            Text {
                text: "should look like 0%"
            }
            PlasmaComponents.ProgressBar {
                from: 0
                to: 100
                value: -10
            }
        }

        Column {
            Text {
                text: "should be a continuous movement,<br>from one end to the other and back"
            }
            PlasmaComponents.ProgressBar {
                indeterminate: indeterminateCheckBox.checked
                value: 0.5
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

    }
}
