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
        color: "#4cb2ff"
        values: [ 80, 40, 80, 50, 100, 60, 70, 40, 110, 50, 70, 20, 40, 10, 20, 10, -50, 40, 80, 70 ]
    }
    Button {
        text: "Add value"
        onClicked: {
            renderer.addValue(Math.random() * 40)
        }
    }
}
