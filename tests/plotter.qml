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

        dataSets: [
            PlotData {
                color: "#4cb2ff"
                values: [ 20, 180, 20, 80, 70, 12, 180, 60]
            },
            PlotData {
                color: "#00b200"
                values: [ 30, 60, 20, 80, 12, 180, 20, 40]
            }
        ]

    }
    Button {
        text: "Add value"
        onClicked: {
            renderer.addValue([Math.random() * 40, Math.random() * 40])
        }
    }
}
