import QtQuick 2.0

import org.kde.plasma.core 2.0
import org.kde.plasma.components 2.0
import org.kde.plasma.extras 2.0

Item {
    width: 500
    height: 200

    Plotter {
        id: renderer
        anchors.fill: parent
        anchors.margins: 0
        stacked: stackedButton.checked
        autoRange: autoRangeButton.checked

        dataSets: [
            PlotData {
                color: "#4cb2ff"
            },
            PlotData {
                color: "#00b200"
            }
        ]

    }
    Row {
        Button {
            text: "Add value"
            onClicked: {
                renderer.addValue([Math.random() * 40, Math.random() * 40])
            }
        }
        ToolButton {
            id: stackedButton
            text: "Stacked"
            checkable: true
            checked: true
        }
        ToolButton {
            id: autoRangeButton
            text: "Auto Range"
            checkable: true
            checked: true
        }
    }
}
