
import QtQuick 2.15
import QtQuick.Controls 2.15
import org.kde.plasma.core 2.0 as PlasmaCore

import org.kde.graphicaleffects 1.0

Item {

    width: 400
    height: 200
    Row {
        id: topRow
        Label {
            text: "Win ID (find via xprop)"
        }
        TextField {
            id: winIdInput
        }
    }
    Row {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: topRow.bottom
        PlasmaCore.WindowThumbnail {
            id: thumb
            width: parent.width/2
            height: 200

            winId: winIdInput.text;
            visible: true
        }
        Lanczos {
            source: thumb
            width: parent.width/2
            height: 200
        }
    }
}
