import QtQuick 2.0

import org.kde.plasma.components 2.0 as PlasmaComponents

Rectangle {
    id: root
    color: "white"
    width: 800
    height: 300

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
            }
        }

        Column {
            Text {
                text: "these main bars should look the same width"
            }
            PlasmaComponents.ProgressBar {
                width:100
                minimumValue: 0
                maximumValue: 100
                value: 50
            }
            PlasmaComponents.Slider {
                width: 100
                minimumValue: 0
                maximumValue: 100
                value: 50
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
            }
        }


    }
}